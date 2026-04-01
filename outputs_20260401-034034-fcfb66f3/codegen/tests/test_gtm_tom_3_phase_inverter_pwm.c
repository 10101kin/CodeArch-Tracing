#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and callback defined in production .c */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                     (1e-4f)
#define UT_NUM_CHANNELS                      (3)
#define UT_PWM_FREQ_HZ                       (20000.0f)
#define UT_INIT_DUTY_U_PERCENT               (25.0f)
#define UT_INIT_DUTY_V_PERCENT               (50.0f)
#define UT_INIT_DUTY_W_PERCENT               (75.0f)
#define UT_STEP_PERCENT                      (10.0f)
#define UT_DUTY_MIN_PERCENT                  (10.0f)
#define UT_DUTY_MAX_PERCENT                  (90.0f)
#define UT_DEAD_TIME_S                       (5e-07f)   /* 0.5 us */
#define UT_MIN_PULSE_S                       (1e-06f)

/* Descriptive, not asserted directly without spies */
#define UT_PWM_MODULE_NAME                   "GTM.TOM1"
#define UT_PWM_CLUSTER_INDEX                 (1)
#define UT_PWM_ALIGNMENT_CENTER              (1)
#define UT_PWM_SYNC_START                    (1)
#define UT_PWM_SYNC_UPDATES                  (1)
#define UT_TOM_TIMER_CHANNEL                 (7)
#define UT_TOM_TIMER_CLOCK                   "CMU_CLK0"
#define UT_LED_PIN_NAME                      "P13.0"

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* Group 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_Init_WhenGtmAlreadyEnabled_MinimalClockSetup(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must not be enabled when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm_init should be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer_init should be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO configured once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should not be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should not be re-enabled when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should not be set when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU CLK frequency should not be set when GTM already enabled");
}

void test_TC_G1_002_Init_WhenGtmDisabled_EnablesAndConfigClocks(void)
{
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz */

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");

    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_getModuleFrequency_getCallCount() > 0, "Module frequency should be read when enabling GTM");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_enableClocks_getCallCount() > 0, "CMU clocks must be enabled when GTM was disabled");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_setGclkFrequency_getCallCount() > 0, "GCLK frequency should be configured when enabling GTM");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_setClkFrequency_getCallCount() > 0, "CMU CLK frequency should be configured when enabling GTM");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm_init should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer_init should be called once");
}

/* Group 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_Init_SetsExpectedPwmFrequencyAndChannelCount(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of PWM logical channels must be 3");
}

void test_TC_G2_002_Init_TimerInitCalledExactlyOnce(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer_init called once");
}

/* Group 3 - initGtmTom3phInv: runtime update logic (post-init checks) */
void test_TC_G3_001_FirstUpdate_IncrementsAllDutiesByStep(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be issued once per call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "Update gating disable called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "Update gating apply called once");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 35.0% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 60.0% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should be 85.0% after first update");
}

void test_TC_G3_002_ThirdUpdate_WrapsOnlyPhaseW_ToStepAndClamps(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called exactly three times");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate called once per update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate called once per update");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 55.0% after third update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 80.0% after third update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should wrap to 10.0% after third update");
}

/* Group 4 - initGtmTom3phInv: ISR / interrupt behavior */
void test_TC_G4_001_ISR_ToggleOnceAfterInit(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 before = mock_togglePin_callCount;
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1, mock_togglePin_callCount, "ISR should toggle LED once");
}

void test_TC_G4_002_ISR_ToggleTwiceAccumulates(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 before = mock_togglePin_callCount;
    interruptGtmAtom();
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 2, mock_togglePin_callCount, "ISR toggle should accumulate across calls");
}

/* Group 5 - updateGtmTom3phInvDuty: configuration values */
void test_TC_G5_001_AfterInit_UpdateUsesThreeChannelArray(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM logical channel count must be 3");

    updateGtmTom3phInvDuty();
    /* Validate three entries are written with expected first-step values */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U first-step duty valid");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V first-step duty valid");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W first-step duty valid");
}

void test_TC_G5_002_UpdateDoesNotChangeConfiguredFrequency(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();
    float configuredFreq = mock_IfxGtm_Pwm_init_lastFrequency;

    updateGtmTom3phInvDuty();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, configuredFreq, "Configured PWM frequency must be 20 kHz");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency spy remains 20 kHz after updates");
}

/* Group 6 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G6_001_TwoUpdates_ClampWToMax90Percent(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates should invoke duty update twice");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate per update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate per update");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 2 updates = 45.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 2 updates = 70.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_MAX_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should clamp to 90.0% at 2nd update");
}

void test_TC_G6_002_FiveUpdates_VWrapsToStep_UNotWrappedYet(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5 */

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates should invoke duty update five times");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 5 updates = 75.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should wrap to 10.0% after 5th update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after wrap progression = 30.0%");
}

/* Group 7 - interruptGtmAtom: initialization / enable guard */
void test_TC_G7_001_ISR_Toggle_DoesNotAffectPwmUpdateCalls(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 beforeUpdates = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeUpdates, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not invoke PWM duty updates");
}

void test_TC_G7_002_ISR_MultipleCalls_NoTimerUpdateCalls(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    interruptGtmAtom();
    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "ISR should not call disableUpdate");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ISR should not call applyUpdate");
}

/* Group 8 - interruptGtmAtom: configuration values */
void test_TC_G8_001_Init_ConfiguresLedGpioOnce(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured once during init");
}

void test_TC_G8_002_PeriodCallbackDoesNotToggleLed(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    initGtmTom3phInv();

    uint32 before = mock_togglePin_callCount;
    IfxGtm_periodEventFunction(NULL);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_callCount, "Period callback must not toggle LED");
}

/* Group 9 - interruptGtmAtom: ISR / interrupt behavior */
void test_TC_G9_001_ISR_Toggle_Single(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 before = mock_togglePin_callCount;
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1, mock_togglePin_callCount, "Single ISR call toggles once");
}

void test_TC_G9_002_ISR_Toggle_AccumulatesAcrossCalls(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 before = mock_togglePin_callCount;
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3, mock_togglePin_callCount, "Three ISR calls toggle three times");
}

/* Group 10 - IfxGtm_periodEventFunction: configuration values */
void test_TC_G10_001_PeriodCallback_DoesNothingToPwmDuties(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();
    float du = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float dv = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float dw = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, du, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Period callback should not change Phase U duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, dv, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Period callback should not change Phase V duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, dw, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Period callback should not change Phase W duty");
}

void test_TC_G10_002_PeriodCallback_DoesNotAffectFrequencyOrCounts(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    float freqBefore = mock_IfxGtm_Pwm_init_lastFrequency;
    uint32 toggleBefore = mock_togglePin_callCount;

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, freqBefore, "Configured PWM frequency remains 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggleBefore, mock_togglePin_callCount, "Period callback must not toggle LED");
}

/* Group 11 - IfxGtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_G11_001_PeriodCallback_DoesNotInvokeTimerUpdate(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "Period callback should not disable updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "Period callback should not apply updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback should not update PWM duties");
}

void test_TC_G11_002_PeriodCallback_MultipleCalls_NoSideEffects(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    initGtmTom3phInv();

    uint32 toggleBefore = mock_togglePin_callCount;

    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggleBefore, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not update duties");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_Init_WhenGtmAlreadyEnabled_MinimalClockSetup);
    RUN_TEST(test_TC_G1_002_Init_WhenGtmDisabled_EnablesAndConfigClocks);

    RUN_TEST(test_TC_G2_001_Init_SetsExpectedPwmFrequencyAndChannelCount);
    RUN_TEST(test_TC_G2_002_Init_TimerInitCalledExactlyOnce);

    RUN_TEST(test_TC_G3_001_FirstUpdate_IncrementsAllDutiesByStep);
    RUN_TEST(test_TC_G3_002_ThirdUpdate_WrapsOnlyPhaseW_ToStepAndClamps);

    RUN_TEST(test_TC_G4_001_ISR_ToggleOnceAfterInit);
    RUN_TEST(test_TC_G4_002_ISR_ToggleTwiceAccumulates);

    RUN_TEST(test_TC_G5_001_AfterInit_UpdateUsesThreeChannelArray);
    RUN_TEST(test_TC_G5_002_UpdateDoesNotChangeConfiguredFrequency);

    RUN_TEST(test_TC_G6_001_TwoUpdates_ClampWToMax90Percent);
    RUN_TEST(test_TC_G6_002_FiveUpdates_VWrapsToStep_UNotWrappedYet);

    RUN_TEST(test_TC_G7_001_ISR_Toggle_DoesNotAffectPwmUpdateCalls);
    RUN_TEST(test_TC_G7_002_ISR_MultipleCalls_NoTimerUpdateCalls);

    RUN_TEST(test_TC_G8_001_Init_ConfiguresLedGpioOnce);
    RUN_TEST(test_TC_G8_002_PeriodCallbackDoesNotToggleLed);

    RUN_TEST(test_TC_G9_001_ISR_Toggle_Single);
    RUN_TEST(test_TC_G9_002_ISR_Toggle_AccumulatesAcrossCalls);

    RUN_TEST(test_TC_G10_001_PeriodCallback_DoesNothingToPwmDuties);
    RUN_TEST(test_TC_G10_002_PeriodCallback_DoesNotAffectFrequencyOrCounts);

    RUN_TEST(test_TC_G11_001_PeriodCallback_DoesNotInvokeTimerUpdate);
    RUN_TEST(test_TC_G11_002_PeriodCallback_MultipleCalls_NoSideEffects);

    return UNITY_END();
}
