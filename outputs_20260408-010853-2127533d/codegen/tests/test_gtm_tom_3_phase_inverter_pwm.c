#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for functions not exposed in the production header */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEAD_TIME_S                   (0.5e-6f)
#define UT_MIN_PULSE_S                   (1.0e-6f)
#define UT_WRAP_THRESHOLD_PERCENT        (90.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* Group 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_init_skips_enable_when_gtm_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency should not be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should not be enabled when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should not be set when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency should not be set when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as push-pull output once");
}

void test_TC_G1_002_init_enables_gtm_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000.0f; /* 100 MHz */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency should be read once when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK clocks should be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should be set when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency should be set when GTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
}

/* Group 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_pwm_init_uses_expected_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3");
}

void test_TC_G2_002_led_pin_configured_output_once_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED output mode should be set once after PWM init");
}

/* Group 3 - initGtmTom3phInv: runtime update logic (post-init behavior on first updates) */
void test_TC_G3_001_first_update_increments_duty_by_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be issued once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should increase by step to 35.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should increase by step to 60.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should increase by step to 85.0%");
}

void test_TC_G3_002_second_update_triggers_wrap_for_W_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1st: U=35, V=60, W=85 */
    updateGtmTom3phInvDuty(); /* 2nd: U=45, V=70, W wraps to 10 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 45.0% after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 70.0% after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10.0% when next >= 90.0%");
}

/* Group 4 - initGtmTom3phInv: ISR / interrupt behavior */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED exactly once");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggles should accumulate");
}

/* Group 5 - updateGtmTom3phInvDuty: initialization / enable guard */
void test_TC_G5_001_update_calls_hal_once_per_call_when_gtm_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "First update should call HAL exactly once");

    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Second update should call HAL exactly once more");
}

void test_TC_G5_002_update_after_init_with_gtm_disabled_path_still_calls_hal_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000.0f;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should have been enabled during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update should call HAL exactly once");
}

/* Group 6 - updateGtmTom3phInvDuty: configuration values */
void test_TC_G6_001_duty_units_are_percent_not_fraction(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: expect 35.0, 60.0, 85.0 (percent), not 0.35, etc. */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Duty should be reported in percent for phase U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Duty should be reported in percent for phase V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Duty should be reported in percent for phase W");
}

void test_TC_G6_002_init_config_num_channels_persists_as_three(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Logical channel count must be 3 across the driver lifetime");
}

/* Group 7 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G7_001_multiple_updates_produce_independent_wraps(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 4 updates */
    updateGtmTom3phInvDuty(); /* [35,60,85] */
    updateGtmTom3phInvDuty(); /* [45,70,10] */
    updateGtmTom3phInvDuty(); /* [55,80,20] */
    updateGtmTom3phInvDuty(); /* [65,10,30] */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 65.0% after 4 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should wrap to 10.0% on 4th update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be 30.0% after independent wraps");
}

void test_TC_G7_002_wrap_occurs_when_next_ge_threshold_for_phase_U(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 7 updates to trigger U wrap (threshold 90) */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5 */
    updateGtmTom3phInvDuty(); /* 6 -> U=85 */
    updateGtmTom3phInvDuty(); /* 7 -> U wraps to 10 */

    /* Assert: After 7th update: [10,40,60] */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should wrap to 10.0% when next >= 90.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 40.0% after 7 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be 60.0% after 7 updates");
}

/* Group 8 - interruptGtmAtom: initialization / enable guard */
void test_TC_G8_001_isr_toggle_without_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED even without PWM init");
}

void test_TC_G8_002_isr_toggle_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED exactly once after init");
}

/* Group 9 - interruptGtmAtom: ISR / interrupt behavior */
void test_TC_G9_001_multiple_isr_calls_accumulate_toggles(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

void test_TC_G9_002_isr_does_not_invoke_pwm_update(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update");
}

/* Group 10 - IfxGtm_periodEventFunction: initialization / enable guard */
void test_TC_G10_001_period_callback_no_side_effects_without_init(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duty");
}

void test_TC_G10_002_period_callback_no_side_effects_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED even after init");
}

/* Group 11 - IfxGtm_periodEventFunction: configuration values */
void test_TC_G11_001_period_callback_does_not_change_channel_count_or_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();
    int prevNumCh = mock_IfxGtm_Pwm_init_lastNumChannels;
    float prevFreq = mock_IfxGtm_Pwm_init_lastFrequency;

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(prevNumCh, mock_IfxGtm_Pwm_init_lastNumChannels, "Period callback must not alter logical channel count");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, prevFreq, mock_IfxGtm_Pwm_init_lastFrequency, "Period callback must not alter PWM frequency");
}

void test_TC_G11_002_period_callback_does_not_trigger_pwm_init_or_update_again(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Sanity: init called once before callback");

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Period callback must not call PWM init again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM update");
}

/* Group 12 - IfxGtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_G12_001_period_callback_can_be_called_multiple_times_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
}

void test_TC_G12_002_period_callback_multiple_calls_no_pwm_update(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not update PWM duties");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_skips_enable_when_gtm_enabled);
    RUN_TEST(test_TC_G1_002_init_enables_gtm_and_clocks_when_disabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_pwm_init_uses_expected_frequency_and_num_channels);
    RUN_TEST(test_TC_G2_002_led_pin_configured_output_once_after_init);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_duty_by_step);
    RUN_TEST(test_TC_G3_002_second_update_triggers_wrap_for_W_only);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_call_when_gtm_enabled);
    RUN_TEST(test_TC_G5_002_update_after_init_with_gtm_disabled_path_still_calls_hal_once);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_duty_units_are_percent_not_fraction);
    RUN_TEST(test_TC_G6_002_init_config_num_channels_persists_as_three);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_multiple_updates_produce_independent_wraps);
    RUN_TEST(test_TC_G7_002_wrap_occurs_when_next_ge_threshold_for_phase_U);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_isr_toggle_without_init);
    RUN_TEST(test_TC_G8_002_isr_toggle_after_init);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_multiple_isr_calls_accumulate_toggles);
    RUN_TEST(test_TC_G9_002_isr_does_not_invoke_pwm_update);

    /* Group 10 */
    RUN_TEST(test_TC_G10_001_period_callback_no_side_effects_without_init);
    RUN_TEST(test_TC_G10_002_period_callback_no_side_effects_after_init);

    /* Group 11 */
    RUN_TEST(test_TC_G11_001_period_callback_does_not_change_channel_count_or_frequency);
    RUN_TEST(test_TC_G11_002_period_callback_does_not_trigger_pwm_init_or_update_again);

    /* Group 12 */
    RUN_TEST(test_TC_G12_001_period_callback_can_be_called_multiple_times_no_toggle);
    RUN_TEST(test_TC_G12_002_period_callback_multiple_calls_no_pwm_update);

    return UNITY_END();
}
