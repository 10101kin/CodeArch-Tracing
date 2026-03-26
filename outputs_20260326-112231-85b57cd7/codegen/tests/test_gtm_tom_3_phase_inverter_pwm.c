#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern callback symbol from production */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQ_HZ                  (20000.0f)
#define UT_NUM_CHANNELS                 (3)
#define UT_DUTY_INIT_U                  (25.0f)
#define UT_DUTY_INIT_V                  (50.0f)
#define UT_DUTY_INIT_W                  (75.0f)
#define UT_DUTY_STEP                    (10.0f)
#define UT_DEADTIME_NS                  (1000.0f)
#define UT_GTM_ASSUMED_FXCLK_HZ        (100000000.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ===================================================================================== */
/* GROUP 1 - initGtmTom3phInv: initialization / enable guard                           */
/* ===================================================================================== */
void test_TC_G1_001_init_when_GTM_enabled_calls_minimal(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once when GTM already enabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled should be checked once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output exactly once after PWM init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No runtime duty update expected during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(), "No runtime dead-time update expected during init");
}

void test_TC_G1_002_init_when_GTM_disabled_enables_and_configures_clocks(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FXCLK_HZ; /* Provide a valid FXCLK */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once when GTM disabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called after enabling GTM");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must be called once after reading module frequency");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called to enable FXCLK/CLK0");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output after PWM init");
}

/* ===================================================================================== */
/* GROUP 2 - initGtmTom3phInv: configuration values                                     */
/* ===================================================================================== */
void test_TC_G2_001_init_config_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert: validate frequency and channel count via init and initConfig spies */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init: PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init: number of channels must be 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig: PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig: number of channels must be 3");
}

void test_TC_G2_002_init_does_not_call_runtime_update_functions(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update should occur during initialization");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(), "No dead-time update should occur during initialization");
}

/* ===================================================================================== */
/* GROUP 3 - initGtmTom3phInv: runtime update logic                                      */
/* ===================================================================================== */
void test_TC_G3_001_update_once_increments_duty_by_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: {25,50,75} -> {35,60,85} */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update should call IfxGtm_Pwm_updateChannelsDutyImmediate once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should increase by 10% to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should increase by 10% to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should increase by 10% to 85%");
}

void test_TC_G3_002_update_three_times_wraps_W_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: perform three updates */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert: after 3 updates, expected {55,80,10} with W wrapping */
    TEST_ASSERT_EQUAL_INT_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Should have 3 duty update calls after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10% after 3 updates");
}

/* ===================================================================================== */
/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior                                  */
/* ===================================================================================== */
void test_TC_G4_001_after_init_no_led_toggle_has_occurred(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "No LED toggle should occur during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_togglePin_callCount, "LED toggle call count spy must remain 0 after init");
}

void test_TC_G4_002_period_callback_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: invoke the PWM period callback twice (it should be empty) */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_togglePin_callCount, "Period callback must not change LED toggle count");
}

/* ===================================================================================== */
/* GROUP 5 - updateGtmTom3phInvDuty: initialization / enable guard                       */
/* ===================================================================================== */
void test_TC_G5_001_update_after_init_enabled_path_calls_once(void)
{
    /* Arrange: GTM already enabled path */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates should result in two IfxGtm_Pwm_updateChannelsDutyImmediate calls");
}

void test_TC_G5_002_update_after_init_disabled_path_clock_guard_executed(void)
{
    /* Arrange: GTM disabled path */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FXCLK_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: ensure clock guard executed during init and update invoked once */
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should have been enabled during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read should have occurred during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU GCLK set should have occurred during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should have been enabled during init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One update should result in one duty immediate call");
}

/* ===================================================================================== */
/* GROUP 6 - updateGtmTom3phInvDuty: configuration values                                */
/* ===================================================================================== */
void test_TC_G6_001_update_applies_expected_duties_array(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: {25,50,75} -> {35,60,85} */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After first update, U should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After first update, V should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After first update, W should be 85%");
}

void test_TC_G6_002_update_does_not_change_configured_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert: frequency spies remain at configured 20 kHz */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init frequency must remain 20 kHz after updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig frequency must remain 20 kHz after updates");
}

/* ===================================================================================== */
/* GROUP 7 - updateGtmTom3phInvDuty: runtime update logic                                */
/* ===================================================================================== */
void test_TC_G7_001_update_five_steps_wraps_V_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 5 updates */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert: expected {75,10,30} after 5 updates */
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates should result in five immediate duty calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 75% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should wrap to 10% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be 30% after 5 updates");
}

void test_TC_G7_002_update_eight_steps_wraps_U_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 8 updates */
    for (int i = 0; i < 8; ++i) {
        updateGtmTom3phInvDuty();
    }

    /* Assert: expected {10,40,60} after 8 updates */
    TEST_ASSERT_EQUAL_INT_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates should result in eight immediate duty calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should wrap to 10% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be 60% after 8 updates");
}

/* ===================================================================================== */
/* GROUP 8 - IfxGtm_periodEventFunction: initialization / enable guard                   */
/* ===================================================================================== */
void test_TC_G8_001_period_callback_without_init_makes_no_driver_calls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Callback without init must not trigger duty update");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Callback without init must not toggle LED");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_togglePin_callCount, "LED toggle count must remain zero without init");
}

void test_TC_G8_002_period_callback_after_init_makes_no_additional_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "After init, period callback must not trigger duty update");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "After init, period callback must not toggle LED");
}

/* ===================================================================================== */
/* GROUP 9 - IfxGtm_periodEventFunction: configuration values                            */
/* ===================================================================================== */
void test_TC_G9_001_period_callback_does_not_change_last_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* establish a known lastDuties[] state */
    float beforeU = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float beforeV = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float beforeW = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeU, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Callback must not alter stored duty for U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeV, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Callback must not alter stored duty for V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeW, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Callback must not alter stored duty for W");
}

void test_TC_G9_002_period_callback_does_not_change_config_frequency_spies(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init frequency must remain 20 kHz after callback");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig frequency must remain 20 kHz after callback");
}

/* ===================================================================================== */
/* GROUP 10 - IfxGtm_periodEventFunction: ISR / interrupt behavior                       */
/* ===================================================================================== */
void test_TC_G10_001_period_callback_single_call_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Single callback call must not toggle LED");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_togglePin_callCount, "Single callback call must not increment LED toggle spy");
}

void test_TC_G10_002_period_callback_multiple_calls_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Multiple callback calls must not toggle LED");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, mock_togglePin_callCount, "Multiple callback calls must not increment LED toggle spy");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_when_GTM_enabled_calls_minimal);
    RUN_TEST(test_TC_G1_002_init_when_GTM_disabled_enables_and_configures_clocks);

    RUN_TEST(test_TC_G2_001_init_config_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_does_not_call_runtime_update_functions);

    RUN_TEST(test_TC_G3_001_update_once_increments_duty_by_step);
    RUN_TEST(test_TC_G3_002_update_three_times_wraps_W_only);

    RUN_TEST(test_TC_G4_001_after_init_no_led_toggle_has_occurred);
    RUN_TEST(test_TC_G4_002_period_callback_does_not_toggle_led);

    RUN_TEST(test_TC_G5_001_update_after_init_enabled_path_calls_once);
    RUN_TEST(test_TC_G5_002_update_after_init_disabled_path_clock_guard_executed);

    RUN_TEST(test_TC_G6_001_update_applies_expected_duties_array);
    RUN_TEST(test_TC_G6_002_update_does_not_change_configured_frequency);

    RUN_TEST(test_TC_G7_001_update_five_steps_wraps_V_only);
    RUN_TEST(test_TC_G7_002_update_eight_steps_wraps_U_only);

    RUN_TEST(test_TC_G8_001_period_callback_without_init_makes_no_driver_calls);
    RUN_TEST(test_TC_G8_002_period_callback_after_init_makes_no_additional_calls);

    RUN_TEST(test_TC_G9_001_period_callback_does_not_change_last_duties);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_change_config_frequency_spies);

    RUN_TEST(test_TC_G10_001_period_callback_single_call_no_toggle);
    RUN_TEST(test_TC_G10_002_period_callback_multiple_calls_no_toggle);

    return UNITY_END();
}
