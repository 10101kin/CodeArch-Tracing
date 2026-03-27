#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Period-event callback provided by production module */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                        (1e-4f)
#define UT_NUM_CHANNELS                         (3U)
#define UT_PWM_FREQUENCY_HZ                     (20000U)
#define UT_DUTY_U_INIT                          (0.25f)
#define UT_DUTY_V_INIT                          (0.50f)
#define UT_DUTY_W_INIT                          (0.75f)
#define UT_UPDATE_STEP                          (0.10f)
#define UT_DT_RISE_S                            (1.0e-6f)
#define UT_DT_FALL_S                            (1.0e-6f)

/* LED pin info (documentary, not asserted for arguments) */
#define UT_LED_PIN                              "P13.0"
#define UT_LED_INIT_LEVEL_LOW                   (0)

void setUp(void)
{
    mock_gtm_tom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/**********************
 * GROUP 1 - init: enable guard and driver calls
 **********************/
void test_TC_G1_001_init_enables_and_configures_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;        /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz */

    /* Act */
    initGtmTom3phInv();

    /* Assert - peripheral enable guard and CMU setup */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be queried once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must be set when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_selectClkInput_getCallCount(), "FXCLK0 must be sourced from GCLK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "TOM CLK0 frequency must be configured");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks must be enabled");

    /* Core PWM driver init path */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
}

void test_TC_G1_002_init_skips_enable_and_clock_config_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;        /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert - no CMU reconfiguration inside guard */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must not be enabled again when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "No CMU frequency read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "No GCLK reconfiguration when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_selectClkInput_getCallCount(), "No FXCLK source selection when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "No TOM CLK0 frequency set when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "No FXCLK enable when already enabled");

    /* Core PWM driver init path still required */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");

    /* LED configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinState_getCallCount(), "LED pin initial state must be set");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_sets_pwm_frequency_20khz_and_3_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert - PWM switching frequency and channel count */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM driver init frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS,     mock_IfxGtm_Pwm_init_lastNumChannels, "PWM driver must initialize exactly 3 channels");

    /* Also ensure initConfig mirrors the same requested values */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS,     mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must be for 3 channels");
}

void test_TC_G2_002_led_pin_configured_output_and_low_on_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert - LED GPIO configuration */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be set to push-pull output mode once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinState_getCallCount(),      "LED pin initial output level must be driven once");
}

/**********************
 * GROUP 3 - init: runtime update linkage (first update after init)
 **********************/
void test_TC_G3_001_after_init_single_update_increments_all_by_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be issued once per call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_U_INIT + UT_UPDATE_STEP), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by 10% from initial");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_V_INIT + UT_UPDATE_STEP), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by 10% from initial");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_W_INIT + UT_UPDATE_STEP), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by 10% from initial");
}

void test_TC_G3_002_three_updates_wrap_W_only_and_one_hal_call_per_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* -> U:0.35 V:0.6 W:0.85 */
    updateGtmTom3phInvDuty(); /* -> U:0.45 V:0.7 W:0.95 */
    updateGtmTom3phInvDuty(); /* -> U:0.55 V:0.8 W:0.10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL duty update per call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.55f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty after 3 updates must be 0.55");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.80f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty after 3 updates must be 0.80");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 0.10 at/above 100%");
}

/**********************
 * GROUP 4 - init: ISR / interrupt behavior (ATOM toggle)
 **********************/
void test_TC_G4_001_atom_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_atom_isr_accumulates_toggle_counts_across_multiple_calls(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        interruptGtmAtom0Ch0();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxPort_togglePin_getCallCount(), "ISR toggles must accumulate across multiple calls");
}

/**********************
 * GROUP 5 - updateGtmTom3phInvDuty: configuration values
 **********************/
void test_TC_G5_001_update_preserves_init_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency from init must remain 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS,     mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels must remain 3");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL update must be triggered by update() call");
}

void test_TC_G5_002_update_first_step_expected_duties_from_initial_config(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - exact expected first-step duties */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.35f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after first update must be 0.35");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after first update must be 0.60");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.85f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after first update must be 0.85");
}

/**********************
 * GROUP 6 - updateGtmTom3phInvDuty: runtime update logic
 **********************/
void test_TC_G6_001_update_increments_when_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* -> 0.35, 0.6, 0.85 */
    updateGtmTom3phInvDuty(); /* -> 0.45, 0.7, 0.95 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two update() calls -> two HAL duty updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.45f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 2 updates must be 0.45");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.70f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 2 updates must be 0.70");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.95f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 2 updates must be 0.95");
}

void test_TC_G6_002_update_wrap_rule_applies_at_or_above_full_scale(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act - 5 updates total to trigger V wrap and advance U/W */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3: W -> 0.10 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5: V -> 0.10 */

    /* Expected after 5 updates: U=0.75, V=0.10 (wrap), W=0.30 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five update() calls -> five HAL duty updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.75f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 5 updates must be 0.75");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must wrap to 0.10 at/above 100%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.30f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 5 updates must be 0.30");
}

/**********************
 * GROUP 7 - IfxGtm_periodEventFunction: configuration values
 **********************/
void test_TC_G7_001_period_event_callback_does_not_affect_pwm_frequency_or_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t freq_before = mock_IfxGtm_Pwm_init_lastFrequency;

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(freq_before, mock_IfxGtm_Pwm_init_lastFrequency, "Callback must not change PWM frequency configuration");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Callback must not trigger duty updates");
}

void test_TC_G7_002_period_event_callback_accepts_nonnull_data_and_has_no_side_effects(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    int dummy = 1234;

    /* Act */
    IfxGtm_periodEventFunction((void *)&dummy);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Callback with non-NULL data must not request duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Callback must not toggle any port");
}

/**********************
 * GROUP 8 - IfxGtm_periodEventFunction: ISR behavior (no-op)
 **********************/
void test_TC_G8_001_period_event_callback_does_not_toggle_led(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period-event callback must not toggle LED");
}

void test_TC_G8_002_period_event_callback_does_not_issue_pwm_update(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period-event callback must not call PWM update API");
}

/**********************
 * GROUP 9 - interruptGtmAtom0Ch0: init/guard (callable anytime)
 **********************/
void test_TC_G9_001_isr_toggles_led_even_before_init(void)
{
    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED even if init not called");
}

void test_TC_G9_002_isr_multiple_calls_before_init_accumulate(void)
{
    /* Act */
    interruptGtmAtom0Ch0();
    interruptGtmAtom0Ch0();
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxPort_togglePin_getCallCount(), "ISR toggles must accumulate across multiple pre-init calls");
}

/**********************
 * GROUP 10 - interruptGtmAtom0Ch0: ISR behavior
 **********************/
void test_TC_G10_001_isr_toggles_led_once_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED once after init");
}

void test_TC_G10_002_isr_toggle_accumulates_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom0Ch0();
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxPort_togglePin_getCallCount(), "ISR toggles must accumulate across multiple post-init calls");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_clock_config_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_20khz_and_3_channels);
    RUN_TEST(test_TC_G2_002_led_pin_configured_output_and_low_on_init);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_after_init_single_update_increments_all_by_step);
    RUN_TEST(test_TC_G3_002_three_updates_wrap_W_only_and_one_hal_call_per_update);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_atom_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_atom_isr_accumulates_toggle_counts_across_multiple_calls);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_preserves_init_frequency_and_channel_count);
    RUN_TEST(test_TC_G5_002_update_first_step_expected_duties_from_initial_config);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_update_increments_when_below_boundary);
    RUN_TEST(test_TC_G6_002_update_wrap_rule_applies_at_or_above_full_scale);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_period_event_callback_does_not_affect_pwm_frequency_or_calls);
    RUN_TEST(test_TC_G7_002_period_event_callback_accepts_nonnull_data_and_has_no_side_effects);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_period_event_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G8_002_period_event_callback_does_not_issue_pwm_update);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_isr_toggles_led_even_before_init);
    RUN_TEST(test_TC_G9_002_isr_multiple_calls_before_init_accumulate);

    /* GROUP 10 */
    RUN_TEST(test_TC_G10_001_isr_toggles_led_once_after_init);
    RUN_TEST(test_TC_G10_002_isr_toggle_accumulates_after_init);

    return UNITY_END();
}
