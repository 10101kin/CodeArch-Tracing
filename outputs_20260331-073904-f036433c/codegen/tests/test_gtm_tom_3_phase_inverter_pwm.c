#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback (defined in production .c) */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3u)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DUTY_MIN_PERCENT              (10.0f)
#define UT_DUTY_MAX_PERCENT              (90.0f)
#define UT_DEAD_TIME_S                   (5e-07f)
#define UT_MIN_PULSE_S                   (1e-06f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ========================= GROUP 1 - initGtmTom3phInv: initialization / enable guard ========================= */
void test_TC_G1_001_init_enables_gtm_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;  /* Force enable path */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* arbitrary module freq */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard should query isEnabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_enable_getCallCount(),     "GTM should be enabled when disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Should read GTM CMU module frequency once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(),   "Should configure GCLK once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(),    "Should configure CLK0 once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Cmu_enableClocks_getCallCount(),       "Should enable FXCLK clocks once when enabling");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_init_getCallCount(),       "IfxGtm_Pwm_init should be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as push-pull output once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;   /* Skip enable path */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard should query isEnabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_enable_getCallCount(),     "GTM enable should be skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK configuration should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 configuration should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Cmu_enableClocks_getCallCount(),       "FXCLK enable should be skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_init_getCallCount(),       "IfxGtm_Pwm_init should still be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as push-pull output once");
}

/* ========================= GROUP 2 - initGtmTom3phInv: configuration values ========================= */
void test_TC_G2_001_init_sets_frequency_and_numChannels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert (assert on init() spy, not initConfig) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init() must configure 3 logical channels (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, (float)mock_IfxGtm_Pwm_init_lastFrequency, "init() must configure PWM switching frequency to 20 kHz");
}

void test_TC_G2_002_init_calls_initConfig_and_init_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_init_getCallCount(),       "IfxGtm_Pwm_init must be called exactly once");
}

/* ========================= GROUP 3 - initGtmTom3phInv: runtime update logic ========================= */
void test_TC_G3_001_update_once_increments_duties_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Single update should call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should increment by step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should increment by step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should increment by step to 85%");
}

void test_TC_G3_002_update_twice_wraps_w_channel_to_min(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 25->35, 50->60, 75->85 */
    updateGtmTom3phInvDuty(); /* 35->45, 60->70, 85->10 (wrap at >=90) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates should call HAL twice");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 2 updates: 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 2 updates: 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_MIN_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to min (10%) after exceeding max");
}

/* ========================= GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior ========================= */
void test_TC_G4_001_isr_single_call_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR should toggle LED exactly once per call");
}

void test_TC_G4_002_isr_multiple_calls_accumulate_toggles(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "ISR toggles must accumulate across multiple calls");
}

/* ========================= GROUP 5 - updateGtmTom3phInvDuty: configuration values ========================= */
void test_TC_G5_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update() must invoke HAL exactly once per call");
}

void test_TC_G5_002_update_uses_percent_units(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert (values in percent, not fractional) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Duty must be provided in percent for Phase U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Duty must be provided in percent for Phase V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Duty must be provided in percent for Phase W");
}

/* ========================= GROUP 6 - updateGtmTom3phInvDuty: runtime update logic ========================= */
void test_TC_G6_001_four_updates_wrap_v_only_and_not_u(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 4 updates */
    updateGtmTom3phInvDuty(); /* U:35 V:60 W:85 */
    updateGtmTom3phInvDuty(); /* U:45 V:70 W:10 */
    updateGtmTom3phInvDuty(); /* U:55 V:80 W:20 */
    updateGtmTom3phInvDuty(); /* U:65 V:10 W:30 (V wraps here) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Four updates should call HAL four times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 4 updates: 65% (no wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should wrap to 10% at 90%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W continues stepping after earlier wrap");
}

void test_TC_G6_002_multiple_updates_call_hal_each_time(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL should be called once per update invocation");
}

/* ========================= GROUP 7 - interruptGtmAtom: configuration values ========================= */
void test_TC_G7_001_isr_does_not_call_pwm_update(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Interrupt must not call PWM duty update");
}

void test_TC_G7_002_isr_does_not_invoke_pwm_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_init_getCallCount(),       "ISR must not call IfxGtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_initConfig_getCallCount(), "ISR must not call IfxGtm_Pwm_initConfig");
}

/* ========================= GROUP 8 - interruptGtmAtom: ISR / interrupt behavior ========================= */
void test_TC_G8_001_isr_three_toggles_count_is_three(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "LED toggle calls should equal ISR invocations");
}

void test_TC_G8_002_isr_does_not_affect_pwm_calls(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call duty update HAL");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_init_getCallCount(),                       "ISR must not call PWM init");
}

/* ========================= GROUP 9 - IfxGtm_periodEventFunction: configuration values ========================= */
void test_TC_G9_001_period_callback_has_no_side_effect_on_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not trigger PWM duty update");
}

/* ========================= GROUP 10 - IfxGtm_periodEventFunction: ISR / interrupt behavior ========================= */
void test_TC_G10_001_multiple_period_callbacks_still_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void *)0);
    IfxGtm_periodEventFunction((void *)0);
    IfxGtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
}

void test_TC_G10_002_multiple_period_callbacks_no_pwm_activity(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void *)0);
    IfxGtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not call PWM update HAL");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_gtm_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_numChannels);
    RUN_TEST(test_TC_G2_002_init_calls_initConfig_and_init_once);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_once_increments_duties_below_boundary);
    RUN_TEST(test_TC_G3_002_update_twice_wraps_w_channel_to_min);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_single_call_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_multiple_calls_accumulate_toggles);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_uses_percent_units);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_four_updates_wrap_v_only_and_not_u);
    RUN_TEST(test_TC_G6_002_multiple_updates_call_hal_each_time);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_isr_does_not_call_pwm_update);
    RUN_TEST(test_TC_G7_002_isr_does_not_invoke_pwm_init);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_isr_three_toggles_count_is_three);
    RUN_TEST(test_TC_G8_002_isr_does_not_affect_pwm_calls);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_period_callback_has_no_side_effect_on_toggle);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_call_pwm_update);

    /* Group 10 */
    RUN_TEST(test_TC_G10_001_multiple_period_callbacks_still_no_toggle);
    RUN_TEST(test_TC_G10_002_multiple_period_callbacks_no_pwm_activity);

    return UNITY_END();
}
