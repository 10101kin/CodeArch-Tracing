#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback (defined in production .c) */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)

#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQ_HZ                   (20000.0f)

#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)

#define UT_DUTY_STEP_PERCENT             (10.0f)
#define UT_DUTY_MIN_PERCENT              (10.0f)
#define UT_DUTY_MAX_PERCENT              (90.0f)

#define UT_DEAD_TIME_S                   (1e-06f)
#define UT_MIN_PULSE_S                   (1e-06f)

/* Expected GTM CMU module frequency used during enable path */
#define UT_EXPECTED_GTM_MODULE_FREQ_HZ   (100000000.0f)

void setUp(void)
{
    mock_gtm_tom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/*
 * GROUP 1 - GTM_TOM_3_Phase_Inverter_PWM_init: initialization / enable guard
 */
void test_TC_G1_001_init_calls_initConfig_and_init_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled must be checked once");

    /* When already enabled, no CMU or GTM enable operations occur */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU_getModuleFrequency must NOT be called when GTM is enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU_setGclkFrequency must NOT be called when GTM is enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU_setClkFrequency must NOT be called when GTM is enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU_enableClocks must NOT be called when GTM is enabled");
}

void test_TC_G1_002_init_enables_cmu_when_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = (uint32)UT_EXPECTED_GTM_MODULE_FREQ_HZ;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM_enable must be called once when GTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU_getModuleFrequency must be called once on enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU_setGclkFrequency must be called once on enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU_setClkFrequency (CLK0) must be called once on enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU_enableClocks (FXCLK domain) must be called once on enable path");

    /* initConfig/init are still called once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must be called exactly once");
}

/*
 * GROUP 2 - GTM_TOM_3_Phase_Inverter_PWM_init: configuration values
 */
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 (U, V, W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED/debug GPIO must be configured exactly once");
}

void test_TC_G2_002_init_calls_initConfig_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once to populate defaults");
}

/*
 * GROUP 3 - GTM_TOM_3_Phase_Inverter_PWM_init: runtime update logic (post-init effects)
 */
void test_TC_G3_001_init_does_not_call_update_duty(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No immediate duty update must occur during init");
}

void test_TC_G3_002_init_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Init must not toggle LED (ISR only)");
}

/*
 * GROUP 4 - GTM_TOM_3_Phase_Inverter_PWM_init: ISR / interrupt behavior
 */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_isr_toggles_led_accumulatively(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/*
 * GROUP 5 - GTM_TOM_3_Phase_Inverter_PWM_updateDuties: configuration values
 */
void test_TC_G5_001_update_first_step_increments_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_DUTY_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must increment by step on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_DUTY_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must increment by step on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_DUTY_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must increment by step on first update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
}

void test_TC_G5_002_update_call_count_is_one_per_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Act */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update invocation");
}

/*
 * GROUP 6 - GTM_TOM_3_Phase_Inverter_PWM_updateDuties: runtime update logic
 */
void test_TC_G6_001_second_update_wraps_W_to_min(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Act */
    /* 1st update: 35, 60, 85 */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    /* 2nd update: 45, 70, 95 -> W wraps to 10 */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 2nd update, Phase U must be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 2nd update, Phase V must be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_MIN_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 2nd update, Phase W must wrap to min (10%)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called twice after two updates");
}

void test_TC_G6_002_fourth_update_wraps_V_to_min_independently(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    GTM_TOM_3_Phase_Inverter_PWM_init();

    /* Act */
    /* Updates to reach V wrap: */
    /* 1: 35,60,85 */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    /* 2: 45,70,10 (W wrapped) */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    /* 3: 55,80,20 */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();
    /* 4: 65,10,30 (V wraps) */
    GTM_TOM_3_Phase_Inverter_PWM_updateDuties();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 4th update, Phase U must be 65%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_MIN_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 4th update, Phase V must wrap to min (10%) independently of U/W");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 4th update, Phase W must be 30%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called four times after four updates");
}

/*
 * GROUP 7 - interruptGtmAtom: initialization / enable guard (independence from GTM state)
 */
void test_TC_G7_001_isr_toggle_when_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even if GTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not invoke PWM duty update");
}

void test_TC_G7_002_isr_toggle_when_gtm_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must toggle LED when GTM is enabled as well");
}

/*
 * GROUP 8 - interruptGtmAtom: ISR / interrupt behavior
 */
void test_TC_G8_001_isr_multiple_calls_accumulate(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        interruptGtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "ISR toggle count must equal number of ISR invocations");
}

void test_TC_G8_002_isr_does_not_invoke_pwm_update(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update function");
}

/*
 * GROUP 9 - IfxGtm_periodEventFunction: configuration values (no side effects)
 */
void test_TC_G9_001_period_callback_has_no_side_effects_null(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

void test_TC_G9_002_period_callback_has_no_side_effects_nonnull(void)
{
    /* Arrange */
    void *userData = (void*)0x1234;

    /* Act */
    IfxGtm_periodEventFunction(userData);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED even with non-NULL data");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties even with non-NULL data");
}

/*
 * GROUP 10 - IfxGtm_periodEventFunction: ISR / interrupt behavior (ensuring no toggles)
 */
void test_TC_G10_001_period_callback_multiple_calls_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must never toggle LED across multiple calls");
}

void test_TC_G10_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    for (int i = 0; i < 4; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must never call PWM update");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_initConfig_and_init_once);
    RUN_TEST(test_TC_G1_002_init_enables_cmu_when_gtm_disabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_calls_initConfig_once);

    RUN_TEST(test_TC_G3_001_init_does_not_call_update_duty);
    RUN_TEST(test_TC_G3_002_init_does_not_toggle_led);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_accumulatively);

    RUN_TEST(test_TC_G5_001_update_first_step_increments_duties);
    RUN_TEST(test_TC_G5_002_update_call_count_is_one_per_update);

    RUN_TEST(test_TC_G6_001_second_update_wraps_W_to_min);
    RUN_TEST(test_TC_G6_002_fourth_update_wraps_V_to_min_independently);

    RUN_TEST(test_TC_G7_001_isr_toggle_when_gtm_disabled);
    RUN_TEST(test_TC_G7_002_isr_toggle_when_gtm_enabled);

    RUN_TEST(test_TC_G8_001_isr_multiple_calls_accumulate);
    RUN_TEST(test_TC_G8_002_isr_does_not_invoke_pwm_update);

    RUN_TEST(test_TC_G9_001_period_callback_has_no_side_effects_null);
    RUN_TEST(test_TC_G9_002_period_callback_has_no_side_effects_nonnull);

    RUN_TEST(test_TC_G10_001_period_callback_multiple_calls_no_toggle);
    RUN_TEST(test_TC_G10_002_period_callback_does_not_call_pwm_update);

    return UNITY_END();
}
