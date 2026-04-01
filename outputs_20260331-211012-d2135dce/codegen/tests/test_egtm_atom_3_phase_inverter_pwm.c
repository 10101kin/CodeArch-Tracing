#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_NUM_CHANNELS                 (3)
#define UT_PWM_FREQUENCY_HZ             (20000.0f)
#define UT_INIT_DUTY_U_PERCENT          (25.0f)
#define UT_INIT_DUTY_V_PERCENT          (50.0f)
#define UT_INIT_DUTY_W_PERCENT          (75.0f)
#define UT_STEP_PERCENT                 (10.0f)
#define UT_DEADTIME_RISE_US             (1.0f)
#define UT_DEADTIME_FALL_US             (1.0f)
#define UT_ISR_PRIORITY                 (20)
#define UT_CMU_TARGET_FREQ_HZ           (100000000U)

void setUp(void) { 
    mock_egtm_atom_3_phase_inverter_pwm_reset(); 
}

void tearDown(void) {}

/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
void test_TC_01_001_Init_PeripheralDisabled_ConfiguresAndEnablesClocks(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* eGTM disabled path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled should be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "enable should be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "getModuleFrequency should be called once in enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks should be enabled in enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "Pwm_init should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should be configured once in enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 frequency should be configured once in enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO should be configured as output once");
}

void test_TC_01_002_Init_PeripheralAlreadyEnabled_SkipsEnableAndClockSetup(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "enable should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "getModuleFrequency should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "enableClocks should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "setGclkFrequency should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "setClkFrequency should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "Pwm_init should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO should be configured as output once");
}

/* GROUP 2 - initEgtmAtom3phInv: configuration values */
void test_TC_02_001_Init_SetsFrequencyAndChannelCount(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* focus on config spies */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency should be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels should be 3 for 3-phase");
}

void test_TC_02_002_Init_CallsInitConfigAndInitExactlyOnce(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "init should be called exactly once");
}

/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (post-init behavior) */
void test_TC_03_001_AfterInit_SingleUpdate_IncrementsAllDutiesByStep(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called once per update cycle");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should increment by step to 35% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should increment by step to 60% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should increment by step to 85% on first update");
}

void test_TC_03_002_AfterInit_ThreeUpdates_WrapsWOnly(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1: U35 V60 W85 */
    updateEgtmAtom3phInvDuty(); /* 2: U45 V70 W95 */
    updateEgtmAtom3phInvDuty(); /* 3: U55 V80 W10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10% after 3 updates");
}

/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior */
void test_TC_04_001_ISR_TogglesLedOnce(void) {
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1, mock_togglePin_callCount, "ISR should toggle LED exactly once per invocation");
}

void test_TC_04_002_ISR_Toggle_AccumulatesAcrossMultipleCalls(void) {
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 2, mock_togglePin_callCount, "LED toggle should accumulate across multiple ISR calls");
}

/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
void test_TC_05_001_Update_FirstCall_UsesPercentUnits(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Duties passed to HAL should be in percent (35.0f), not normalized");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Single immediate update call is expected");
}

void test_TC_05_002_Update_DoesNotAffectInitFrequencyOrChannelCount(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Init frequency spy should remain 20 kHz after updates");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Init numChannels spy should remain 3 after updates");
}

/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
void test_TC_06_001_Update_WrapBehavior_AllChannelsIndependent(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 8 updates */
    for (int i = 0; i < 8; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 updates: U=10, V=40, W=60 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL call per update; should be 8 after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U wrap result should be 10% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V expected 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W expected 60% after 8 updates");
}

void test_TC_06_002_Update_CallCountAccumulates(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "updateChannelsDutyImmediate should be invoked once per call; total 5");
}

/* GROUP 7 - interruptEgtmAtom: initialization / enable guard (no PWM or CMU interactions) */
void test_TC_07_001_ISR_DoesNotInvokePwmUpdate(void) {
    /* Arrange */
    uint32_t beforeUpdates = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeUpdates, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not invoke PWM duty update");
}

void test_TC_07_002_ISR_ToggleOnce_WhenCalledOnce(void) {
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1, mock_togglePin_callCount, "ISR should toggle LED exactly once");
}

/* GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior */
void test_TC_08_001_ISR_MultipleInvocations_AccumulateToggleCount(void) {
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3, mock_togglePin_callCount, "Three ISR calls should produce three toggles");
}

void test_TC_08_002_ISR_DoesNotAffectCmuClockEnableCalls(void) {
    /* Arrange */
    uint32_t beforeCmuEnable = mock_IfxEgtm_Cmu_enableClocks_getCallCount();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeCmuEnable, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "ISR should not touch CMU clock enable");
}

/* GROUP 9 - IfxEgtm_periodEventFunction: configuration values (no side effects) */
void test_TC_09_001_PeriodCallback_DoesNothing_NoToggle(void) {
    /* Arrange */
    uint32_t beforeToggle = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeToggle, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_09_002_PeriodCallback_DoesNothing_NoPwmUpdate(void) {
    /* Arrange */
    uint32_t beforeUpdates = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeUpdates, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

/* GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (no-op) */
void test_TC_10_001_PeriodCallback_MultipleInvocations_NoSideEffects(void) {
    /* Arrange */
    uint32_t beforeToggle = mock_togglePin_callCount;
    uint32_t beforeUpdates = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeToggle, mock_togglePin_callCount, "Multiple period callbacks should not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeUpdates, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks should not update PWM duties");
}

void test_TC_10_002_PeriodCallback_NullDataAccepted_NoAction(void) {
    /* Arrange */
    uint32_t beforeToggle = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeToggle, mock_togglePin_callCount, "Callback should accept NULL data and perform no action");
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_TC_01_001_Init_PeripheralDisabled_ConfiguresAndEnablesClocks);
    RUN_TEST(test_TC_01_002_Init_PeripheralAlreadyEnabled_SkipsEnableAndClockSetup);

    RUN_TEST(test_TC_02_001_Init_SetsFrequencyAndChannelCount);
    RUN_TEST(test_TC_02_002_Init_CallsInitConfigAndInitExactlyOnce);

    RUN_TEST(test_TC_03_001_AfterInit_SingleUpdate_IncrementsAllDutiesByStep);
    RUN_TEST(test_TC_03_002_AfterInit_ThreeUpdates_WrapsWOnly);

    RUN_TEST(test_TC_04_001_ISR_TogglesLedOnce);
    RUN_TEST(test_TC_04_002_ISR_Toggle_AccumulatesAcrossMultipleCalls);

    RUN_TEST(test_TC_05_001_Update_FirstCall_UsesPercentUnits);
    RUN_TEST(test_TC_05_002_Update_DoesNotAffectInitFrequencyOrChannelCount);

    RUN_TEST(test_TC_06_001_Update_WrapBehavior_AllChannelsIndependent);
    RUN_TEST(test_TC_06_002_Update_CallCountAccumulates);

    RUN_TEST(test_TC_07_001_ISR_DoesNotInvokePwmUpdate);
    RUN_TEST(test_TC_07_002_ISR_ToggleOnce_WhenCalledOnce);

    RUN_TEST(test_TC_08_001_ISR_MultipleInvocations_AccumulateToggleCount);
    RUN_TEST(test_TC_08_002_ISR_DoesNotAffectCmuClockEnableCalls);

    RUN_TEST(test_TC_09_001_PeriodCallback_DoesNothing_NoToggle);
    RUN_TEST(test_TC_09_002_PeriodCallback_DoesNothing_NoPwmUpdate);

    RUN_TEST(test_TC_10_001_PeriodCallback_MultipleInvocations_NoSideEffects);
    RUN_TEST(test_TC_10_002_PeriodCallback_NullDataAccepted_NoAction);

    return UNITY_END();
}
