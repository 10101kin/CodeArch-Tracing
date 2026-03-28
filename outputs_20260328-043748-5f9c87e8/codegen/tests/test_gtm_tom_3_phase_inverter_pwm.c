#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for functions possibly not in header */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_DUTY_INIT_U                   (25.0f)
#define UT_DUTY_INIT_V                   (50.0f)
#define UT_DUTY_INIT_W                   (75.0f)
#define UT_DUTY_STEP                     (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_FXCLK0_HZ                     (100000000U)
#define UT_LED_ISR_PRIORITY              (20U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initGtmTomPwm: initialization / enable guard */
void test_TC_G1_001_init_enablesGtmAndConfiguresClocks_whenDisabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled should be called once in init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable should be called when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency should be read when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK domain should be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once during init");
}

void test_TC_G1_002_init_skipsEnable_whenAlreadyEnabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled should be called once in init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must not be called when GTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read should be skipped if GTM already enabled (guarded block)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK enable should be skipped if GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured once as output");
}

/* GROUP 2 - initGtmTomPwm: configuration values */
void test_TC_G2_001_init_setsFrequencyAndChannels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Skip GTM enable path */

    /* Act */
    initGtmTomPwm();

    /* Assert - frequency and number of channels */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, (unsigned int)mock_IfxGtm_Pwm_init_lastFrequency, "PWM init should use required switching frequency (20 kHz)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxGtm_Pwm_init_lastNumChannels, "PWM init should configure exactly 3 channels (U,V,W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, (unsigned int)mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig should carry 20 kHz frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig should carry 3 channels");
}

void test_TC_G2_002_init_doesNotApplyDutyUpdatesDuringInit(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - no duty update during init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update calls should be made during initialization");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin output configuration should be performed exactly once");
}

/* GROUP 3 - initGtmTomPwm: runtime update logic (first updates after init) */
void test_TC_G3_001_firstUpdateStepsFromInitialDuties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert - duties stepped from 25/50/75 to 35/60/85 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one duty update call per updateGtmTomPwmDutyCycles()");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should step to 35% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should step to 60% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should step to 85% on first update");
}

void test_TC_G3_002_wrapOccursOnWAtThirdUpdate(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles(); /* -> 35,60,85 */
    updateGtmTomPwmDutyCycles(); /* -> 45,70,95 */
    updateGtmTomPwmDutyCycles(); /* -> 55,80,10 (W wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three update calls should result in three HAL updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U at third update should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V at third update should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10% on third update");
}

/* GROUP 4 - initGtmTomPwm: ISR / interrupt behavior */
void test_TC_G4_001_isr_toggleIncrementsOnce(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR should toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggleAccumulatesAcrossCalls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxPort_togglePin_getCallCount(), "Toggle count should accumulate across ISR calls");
}

/* GROUP 5 - updateGtmTomPwmDutyCycles: initialization / enable guard */
void test_TC_G5_001_update_afterInit_doesNotReinitOrReenable_whenGtmWasEnabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig should have been called exactly once in init only");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init should have been called exactly once in init only");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Clock enable must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one duty update call on first update");
}

void test_TC_G5_002_update_afterInit_doesNotReenable_whenGtmWasDisabledButInitialized(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* Force enable path during init */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_HZ;
    initGtmTomPwm();

    /* Sanity: enable path occurred */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enable should have happened during init when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK enable should have happened during init when disabled");

    /* Act - updates after init */
    updateGtmTomPwmDutyCycles();

    /* Assert - no re-enable on update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "No additional GTM enable calls expected during update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "No additional clock enable calls expected during update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One duty update should be performed");
}

/* GROUP 6 - updateGtmTomPwmDutyCycles: configuration values */
void test_TC_G6_001_update_usesThreeChannelDutyArray_andStepsCorrectly(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert - verify stepped duties match expectation */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should be 35% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should be 60% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should be 85% after first update");
}

void test_TC_G6_002_twoUpdates_keepDutiesWithinPercentRange(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles(); /* 35,60,85 */
    updateGtmTomPwmDutyCycles(); /* 45,70,95 */

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 100.0f, "U duty must remain within 0..100 percent");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 100.0f, "V duty must remain within 0..100 percent");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 100.0f, "W duty must remain within 0..100 percent");
}

/* GROUP 7 - updateGtmTomPwmDutyCycles: runtime update logic */
void test_TC_G7_001_secondUpdate_showsNoWrapForUandV_but95ForW(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles(); /* 35,60,85 */
    updateGtmTomPwmDutyCycles(); /* 45,70,95 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should be 45% after two updates (no wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should be 70% after two updates (no wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should be 95% after two updates (no wrap yet)");
}

void test_TC_G7_002_afterFiveUpdates_VWrappedTo10_UNotWrapped_WAdvancedTo30(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act: five updates */
    updateGtmTomPwmDutyCycles(); /* 35,60,85 */
    updateGtmTomPwmDutyCycles(); /* 45,70,95 */
    updateGtmTomPwmDutyCycles(); /* 55,80,10 */
    updateGtmTomPwmDutyCycles(); /* 65,90,20 */
    updateGtmTomPwmDutyCycles(); /* 75,10,30 (V wraps) */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 75% after five updates (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should wrap to 10% on fifth update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should advance to 30% after fifth update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five logical updates should make five HAL duty updates");
}

/* GROUP 8 - interruptGtmAtom: initialization / enable guard */
void test_TC_G8_001_isr_withoutInit_doesNotTouchPwmOrCmu(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR must not trigger PWM init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not trigger PWM duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_isEnabled_getCallCount(), "ISR must not touch GTM enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR should toggle LED once");
}

void test_TC_G8_002_isr_afterInit_stillOnlyTogglesLed(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR should toggle LED exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "No additional PWM init calls from ISR");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not update duties");
}

/* GROUP 9 - interruptGtmAtom: configuration values */
void test_TC_G9_001_isr_doesNotChangeDutiesOrFrequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not perform duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, (unsigned int)mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency remains configured as 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxGtm_Pwm_init_lastNumChannels, "PWM remains configured for 3 channels");
}

void test_TC_G9_002_isr_callDoesNotTriggerCmuOperations(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "ISR should not query CMU frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "ISR should not enable CMU clocks");
}

/* GROUP 10 - interruptGtmAtom: ISR / interrupt behavior */
void test_TC_G10_001_multipleIsrCallsAccumulate(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i) {
        interruptGtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxPort_togglePin_getCallCount(), "Five ISR calls should toggle LED five times");
}

void test_TC_G10_002_isrNoOtherDriverCalls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR should not call PWM init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not call PWM duty update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_isEnabled_getCallCount(), "ISR should not check GTM enable state");
}

/* GROUP 11 - IfxGtm_periodEventFunction: configuration values */
void test_TC_G11_001_periodCallback_isNoOp_noDriverCalls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback should not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback should not update duties");
}

void test_TC_G11_002_periodCallback_afterInit_doesNotAlterPwmConfig(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();
    unsigned int freqBefore = (unsigned int)mock_IfxGtm_Pwm_init_lastFrequency;

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(freqBefore, (unsigned int)mock_IfxGtm_Pwm_init_lastFrequency, "Period callback should not modify PWM frequency configuration");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback should not cause duty updates");
}

/* GROUP 12 - IfxGtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_G12_001_periodCallback_noToggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Callback must not toggle LED");
}

void test_TC_G12_002_periodCallback_safeOnMultipleInvocations(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Multiple callback calls must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple callback calls must not update duties");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enablesGtmAndConfiguresClocks_whenDisabled);
    RUN_TEST(test_TC_G1_002_init_skipsEnable_whenAlreadyEnabled);

    RUN_TEST(test_TC_G2_001_init_setsFrequencyAndChannels);
    RUN_TEST(test_TC_G2_002_init_doesNotApplyDutyUpdatesDuringInit);

    RUN_TEST(test_TC_G3_001_firstUpdateStepsFromInitialDuties);
    RUN_TEST(test_TC_G3_002_wrapOccursOnWAtThirdUpdate);

    RUN_TEST(test_TC_G4_001_isr_toggleIncrementsOnce);
    RUN_TEST(test_TC_G4_002_isr_toggleAccumulatesAcrossCalls);

    RUN_TEST(test_TC_G5_001_update_afterInit_doesNotReinitOrReenable_whenGtmWasEnabled);
    RUN_TEST(test_TC_G5_002_update_afterInit_doesNotReenable_whenGtmWasDisabledButInitialized);

    RUN_TEST(test_TC_G6_001_update_usesThreeChannelDutyArray_andStepsCorrectly);
    RUN_TEST(test_TC_G6_002_twoUpdates_keepDutiesWithinPercentRange);

    RUN_TEST(test_TC_G7_001_secondUpdate_showsNoWrapForUandV_but95ForW);
    RUN_TEST(test_TC_G7_002_afterFiveUpdates_VWrappedTo10_UNotWrapped_WAdvancedTo30);

    RUN_TEST(test_TC_G8_001_isr_withoutInit_doesNotTouchPwmOrCmu);
    RUN_TEST(test_TC_G8_002_isr_afterInit_stillOnlyTogglesLed);

    RUN_TEST(test_TC_G9_001_isr_doesNotChangeDutiesOrFrequency);
    RUN_TEST(test_TC_G9_002_isr_callDoesNotTriggerCmuOperations);

    RUN_TEST(test_TC_G10_001_multipleIsrCallsAccumulate);
    RUN_TEST(test_TC_G10_002_isrNoOtherDriverCalls);

    RUN_TEST(test_TC_G11_001_periodCallback_isNoOp_noDriverCalls);
    RUN_TEST(test_TC_G11_002_periodCallback_afterInit_doesNotAlterPwmConfig);

    RUN_TEST(test_TC_G12_001_periodCallback_noToggle);
    RUN_TEST(test_TC_G12_002_periodCallback_safeOnMultipleInvocations);

    return UNITY_END();
}
