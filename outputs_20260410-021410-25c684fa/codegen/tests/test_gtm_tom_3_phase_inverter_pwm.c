#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Externs for ISR/callback defined in production .c */
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
#define UT_MODULE_FREQ_HZ                (100000000u)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_init_guard_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    mock_IfxGtm_Cmu_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO configured once");
}

void test_TC_G1_002_init_guard_enables_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_MODULE_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enable must be called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once when GTM is enabled in guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm_init must be called once");
}

/* GROUP 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_init_sets_numChannels_and_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Num channels must be 3 (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_configures_led_gpio_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED/debug GPIO configured once during init");
}

/* GROUP 3 - initGtmTom3phInv: runtime update logic (post-init update behavior) */
void test_TC_G3_001_update_after_init_increments_by_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update called once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step to 85%");
}

void test_TC_G3_002_update_three_steps_wraps_phase_W_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 35,60,85 */
    updateGtmTom3phInvDuty(); /* 45,70,95 */
    updateGtmTom3phInvDuty(); /* 55,80,10 (W wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update called once per update, total 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 3 steps must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 3 steps must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to step (10%)");
}

/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior (LED toggle) */
void test_TC_G4_001_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once");
}

void test_TC_G4_002_isr_multiple_calls_accumulate_toggle_count(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/* GROUP 5 - updateGtmTom3phInvDuty: configuration values (call counts) */
void test_TC_G5_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL update per update invocation");
}

void test_TC_G5_002_update_called_twice_increments_call_count_to_two(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must yield two HAL calls");
}

/* GROUP 6 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G6_001_eight_updates_result_in_all_channels_wrapped_to_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates must yield eight HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U wraps to step (10%) after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V wraps to step (10%) after 6th update, remains 10% at 8th");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W wraps to step (10%) by 3rd update, remains 10% at 8th");
}

void test_TC_G6_002_update_uses_percent_units_not_fraction(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Duties must be percent values (35.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Duties must be percent values (60.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Duties must be percent values (85.0)");
}

/* GROUP 7 - interruptGtmAtom: initialization / enable guard (independent of GTM state) */
void test_TC_G7_001_isr_toggles_when_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even if GTM disabled");
}

void test_TC_G7_002_isr_toggles_when_gtm_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED when GTM enabled");
}

/* GROUP 8 - interruptGtmAtom: ISR / interrupt behavior (no PWM HAL calls) */
void test_TC_G8_001_isr_does_not_call_pwm_update(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update HAL");
}

void test_TC_G8_002_isr_does_not_call_pwm_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR must not call PWM init HAL");
}

/* GROUP 9 - IfxGtm_periodEventFunction: configuration values (no side effects) */
void test_TC_G9_001_period_callback_is_empty_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM duty update HAL");
}

/* GROUP 10 - IfxGtm_periodEventFunction: ISR / interrupt behavior (still empty) */
void test_TC_G10_001_multiple_period_callback_calls_have_no_side_effects(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not call PWM duty update HAL");
}

void test_TC_G10_002_period_callback_accepts_null_and_leaves_state_unchanged(void)
{
    /* Arrange */
    /* No state to arrange; ensure counts are zero */

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback with NULL data must leave LED toggle count unchanged");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "Period callback must not trigger PWM init");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_guard_already_enabled);
    RUN_TEST(test_TC_G1_002_init_guard_enables_and_clocks_when_disabled);

    RUN_TEST(test_TC_G2_001_init_sets_numChannels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_configures_led_gpio_once);

    RUN_TEST(test_TC_G3_001_update_after_init_increments_by_step);
    RUN_TEST(test_TC_G3_002_update_three_steps_wraps_phase_W_only);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_isr_multiple_calls_accumulate_toggle_count);

    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_called_twice_increments_call_count_to_two);

    RUN_TEST(test_TC_G6_001_eight_updates_result_in_all_channels_wrapped_to_step);
    RUN_TEST(test_TC_G6_002_update_uses_percent_units_not_fraction);

    RUN_TEST(test_TC_G7_001_isr_toggles_when_gtm_disabled);
    RUN_TEST(test_TC_G7_002_isr_toggles_when_gtm_enabled);

    RUN_TEST(test_TC_G8_001_isr_does_not_call_pwm_update);
    RUN_TEST(test_TC_G8_002_isr_does_not_call_pwm_init);

    RUN_TEST(test_TC_G9_001_period_callback_is_empty_no_toggle);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_call_pwm_update);

    RUN_TEST(test_TC_G10_001_multiple_period_callback_calls_have_no_side_effects);
    RUN_TEST(test_TC_G10_002_period_callback_accepts_null_and_leaves_state_unchanged);

    return UNITY_END();
}
