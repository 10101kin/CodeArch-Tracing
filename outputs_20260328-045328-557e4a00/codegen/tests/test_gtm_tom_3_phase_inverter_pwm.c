#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Period event callback provided by production code */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQ_HZ                  (20000)
#define UT_NUM_CHANNELS                 (3)
#define UT_DEADTIME_US                  (1.0f)
#define UT_DUTY_INIT_U_PERCENT          (25.0f)
#define UT_DUTY_INIT_V_PERCENT          (50.0f)
#define UT_DUTY_INIT_W_PERCENT          (75.0f)
#define UT_DUTY_STEP_PERCENT            (10.0f)
#define UT_GTM_CMU_FXCLK_HZ             (100000000u)
#define UT_ISR_PRIORITY                 (20)

static void perform_updates(unsigned count)
{
    for (unsigned i = 0; i < count; ++i)
    {
        updateGtmTom3phInvDuty();
    }
}

void setUp(void)
{
    mock_gtm_tom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/**********************
 * GROUP 1 - init: initialization / enable guard
 **********************/
void test_TC_G1_001_init_when_gtm_already_enabled_does_not_touch_cmu(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must not be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

void test_TC_G1_002_init_enables_gtm_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM not enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_CMU_FXCLK_HZ; /* FXCLK source for guard path */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must be called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must be called when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must be called when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called when enabling GTM");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_sets_pwm_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig must use PWM frequency = 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init must use PWM frequency = 20 kHz");

    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must configure 3 logical channels");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init must initialize 3 logical channels");
}

void test_TC_G2_002_init_configures_debug_led_output_pin_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug LED GPIO must be configured once as push-pull output");
}

/**********************
 * GROUP 3 - init: runtime update logic (first-step behavior derived from initial config)
 **********************/
void test_TC_G3_001_first_update_increments_duty_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.35f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 35% (0.35) after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 60% (0.60) after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.85f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 85% (0.85) after first update");
}

void test_TC_G3_002_third_update_wraps_only_phase_W_to_10_percent(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    perform_updates(3); /* Expect U=55%, V=80%, W=10% */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three updates must yield three HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.55f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.80f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 10% after 3 updates");
}

/**********************
 * GROUP 4 - init: ISR / interrupt behavior
 **********************/
void test_TC_G4_001_period_event_function_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "Period event must toggle LED exactly once");
}

void test_TC_G4_002_multiple_period_events_accumulate_toggle_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxPort_togglePin_getCallCount(), "Five ISR invocations must produce five toggles");
}

/**********************
 * GROUP 5 - update: initialization / enable guard (call sequencing and no re-init)
 **********************/
void test_TC_G5_001_update_does_not_reinit_pwm(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Sanity check after init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called once after init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must be called once after init");

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "update must not re-call initConfig");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "update must not re-call init");
}

void test_TC_G5_002_update_multiple_times_does_not_call_cmu_or_enable(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* avoid CMU path in init */
    initGtmTom3phInv();

    /* Act */
    perform_updates(3);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three updates must yield three HAL calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "update must not (re)enable GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "update must not touch CMU getModuleFrequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "update must not touch CMU setGclkFrequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "update must not touch CMU setClkFrequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "update must not touch CMU enableClocks");
}

/**********************
 * GROUP 6 - update: configuration values (duty arrays and stable PWM config)
 **********************/
void test_TC_G6_001_update_four_steps_expected_duty_array(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    perform_updates(4); /* Expect U=65%, V=90%, W=20% */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.65f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 4 updates, Phase U must be 65% (0.65)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.90f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 4 updates, Phase V must be 90% (0.90)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.20f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 4 updates, Phase W must be 20% (0.20)");
}

void test_TC_G6_002_pwm_frequency_and_channel_count_stable_after_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    perform_updates(2);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after updates");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels must remain 3 after updates");
}

/**********************
 * GROUP 7 - update: runtime update logic (wrap behavior and independence)
 **********************/
void test_TC_G7_001_independent_wraps_V_after_five_steps_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    perform_updates(5); /* Expect U=75%, V=10% (wrap), W=30% */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates must yield five HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.75f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 5 updates, Phase U must be 75% (0.75)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 5 updates, Phase V must wrap to 10% (0.10)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.30f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 5 updates, Phase W must be 30% (0.30)");
}

void test_TC_G7_002_eight_updates_expected_duties_across_all_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    perform_updates(8); /* Expect U=10%, V=40%, W=60% */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates must yield eight HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 8 updates, Phase U must be 10% (0.10)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.40f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 8 updates, Phase V must be 40% (0.40)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 8 updates, Phase W must be 60% (0.60)");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_when_gtm_already_enabled_does_not_touch_cmu);
    RUN_TEST(test_TC_G1_002_init_enables_gtm_and_configures_cmu_when_disabled);

    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_num_channels);
    RUN_TEST(test_TC_G2_002_init_configures_debug_led_output_pin_once);

    RUN_TEST(test_TC_G3_001_first_update_increments_duty_below_boundary);
    RUN_TEST(test_TC_G3_002_third_update_wraps_only_phase_W_to_10_percent);

    RUN_TEST(test_TC_G4_001_period_event_function_toggles_led_once);
    RUN_TEST(test_TC_G4_002_multiple_period_events_accumulate_toggle_calls);

    RUN_TEST(test_TC_G5_001_update_does_not_reinit_pwm);
    RUN_TEST(test_TC_G5_002_update_multiple_times_does_not_call_cmu_or_enable);

    RUN_TEST(test_TC_G6_001_update_four_steps_expected_duty_array);
    RUN_TEST(test_TC_G6_002_pwm_frequency_and_channel_count_stable_after_updates);

    RUN_TEST(test_TC_G7_001_independent_wraps_V_after_five_steps_only);
    RUN_TEST(test_TC_G7_002_eight_updates_expected_duties_across_all_channels);

    return UNITY_END();
}
