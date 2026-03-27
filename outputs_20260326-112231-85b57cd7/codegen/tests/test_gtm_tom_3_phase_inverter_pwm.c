#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Period callback symbol is provided by production */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_DUTY_STEP_PERCENT             (10.0f)
#define UT_DUTY_INIT_U                   (25.0f)
#define UT_DUTY_INIT_V                   (50.0f)
#define UT_DUTY_INIT_W                   (75.0f)
#define UT_DEADTIME_RISING_NS            (1000U)
#define UT_DEADTIME_FALLING_NS           (1000U)
#define UT_GTM_MODULE_CLOCK_HZ           (100000000U)
#define UT_ISR_PRIORITY                  (20U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ------------------------------------------------------------ */
/* GROUP 1 - initGtmTom3phInv: initialization / enable guard   */
/* ------------------------------------------------------------ */
void test_TC_G1_001_init_when_GTM_already_enabled_does_not_enable_or_configure_clocks(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM should NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency should NOT be called when GTM already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks should NOT be called when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once during init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once during init");
}

void test_TC_G1_002_init_when_GTM_disabled_enables_and_configures_clocks_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled -> enable + CMU config */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_MODULE_CLOCK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled exactly once when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency should be called once when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks should be called once when enabling GTM");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once during init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once during init");
}

/* ------------------------------------------------------------ */
/* GROUP 2 - initGtmTom3phInv: configuration values             */
/* ------------------------------------------------------------ */
void test_TC_G2_001_init_config_sets_frequency_and_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert: verify PWM output frequency and number of channels captured by spies */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init: unexpected number of channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init: PWM frequency should be 20 kHz");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig: unexpected number of channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig: PWM frequency should be 20 kHz");
}

void test_TC_G2_002_init_applies_initial_duty_and_deadtime(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert: initial duties (percent) applied to three channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "init: U phase initial duty should be 25%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_V, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "init: V phase initial duty should be 50%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_W, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "init: W phase initial duty should be 75%%");

    /* Assert: hardware deadtime (ns) configured for rising and falling on first three channels */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_RISING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[0], "init: U phase rising deadtime should be 1000 ns");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_RISING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[1], "init: V phase rising deadtime should be 1000 ns");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_RISING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[2], "init: W phase rising deadtime should be 1000 ns");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_FALLING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[0], "init: U phase falling deadtime should be 1000 ns");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_FALLING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[1], "init: V phase falling deadtime should be 1000 ns");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_FALLING_NS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[2], "init: W phase falling deadtime should be 1000 ns");
}

/* ------------------------------------------------------------ */
/* GROUP 3 - initGtmTom3phInv: runtime update logic             */
/* ------------------------------------------------------------ */
void test_TC_G3_001_update_after_init_increments_duty_by_step_and_updates_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t prevUpdates = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(prevUpdates + 1U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: HAL should be called exactly once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_U + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "update: U should increase by +10%% from 25%% to 35%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_V + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "update: V should increase by +10%% from 50%% to 60%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_INIT_W + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "update: W should increase by +10%% from 75%% to 85%%");
}

void test_TC_G3_002_update_wraps_only_channels_reaching_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: perform three updates */
    updateGtmTom3phInvDuty(); /* 25->35, 50->60, 75->85 */
    updateGtmTom3phInvDuty(); /* 35->45, 60->70, 85->95 */
    updateGtmTom3phInvDuty(); /* 45->55, 70->80, 95->wrap->10 */

    /* Assert: independent wrap on W only */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: HAL should be called once per invocation (3 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "update: U should be 55%% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "update: V should be 80%% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "update: W should wrap to 10%% after 3rd step");
}

/* ------------------------------------------------------------ */
/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior         */
/* ------------------------------------------------------------ */
void test_TC_G4_001_init_configures_led_gpio_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured exactly once during init");
}

void test_TC_G4_002_no_led_toggle_during_init_or_without_isr(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert: no toggles should occur unless ISR runs */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxPort_togglePin_getCallCount(), "LED must not toggle during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "LED toggle call counter must remain zero without ISR");
}

/* ------------------------------------------------------------ */
/* GROUP 5 - updateGtmTom3phInvDuty: initialization / guards    */
/* ------------------------------------------------------------ */
void test_TC_G5_001_update_does_not_reinitialize_or_touch_clocks_when_gtm_pre_enabled(void)
{
    /* Arrange: GTM already enabled path */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Sanity: after init with GTM enabled, no CMU clock enable should have occurred */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Precondition: CMU enableClocks should be 0 when GTM was already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_init_getCallCount(), "Precondition: Pwm_init should be 1 after init");

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: only duty update is issued */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: duty update should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_init_getCallCount(), "update: must NOT re-call Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "update: must NOT touch CMU clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_enable_getCallCount(), "update: must NOT re-enable GTM");
}

void test_TC_G5_002_update_does_not_reenable_or_reconfigure_clocks_after_init_guarded_path(void)
{
    /* Arrange: GTM disabled -> init performs enable + CMU config */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_MODULE_CLOCK_HZ;
    initGtmTom3phInv();

    /* Sanity after init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_enable_getCallCount(), "Precondition: GTM should have been enabled during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Precondition: CMU clocks should have been enabled during init");

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: update should not modify enable/clock state */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: duty update should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_enable_getCallCount(), "update: must NOT re-enable GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "update: must NOT re-configure CMU clocks");
}

/* ------------------------------------------------------------ */
/* GROUP 6 - updateGtmTom3phInvDuty: configuration values       */
/* ------------------------------------------------------------ */
void test_TC_G6_001_update_maintains_config_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: configuration spies remain consistent */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "config: channel count must remain 3 after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "config: PWM frequency must remain 20 kHz after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "config: initConfig channel count must remain 3 after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "config: initConfig PWM frequency must remain 20 kHz after update");
}

void test_TC_G6_002_update_once_sets_expected_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: duties after one step */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "update: U duty should be 35%% after one step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "update: V duty should be 60%% after one step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "update: W duty should be 85%% after one step");
}

/* ------------------------------------------------------------ */
/* GROUP 7 - updateGtmTom3phInvDuty: runtime update logic       */
/* ------------------------------------------------------------ */
void test_TC_G7_001_multiple_updates_eventually_wrap_all_channels_to_step_value(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: perform 8 updates; with +10% steps all three wrap to 10% */
    for (int i = 0; i < 8; ++i) {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: HAL should be called once per update (8 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "update: U should be 10%% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "update: V should be 10%% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "update: W should be 10%% after 8 steps");
}

void test_TC_G7_002_additional_update_continues_from_wrapped_value(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    for (int i = 0; i < 8; ++i) {
        updateGtmTom3phInvDuty();
    }

    /* Act: one more update from 10% -> 20% on all channels */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(9U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update: HAL should be called once per update (9 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "update: U should be 20%% after 9th step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "update: V should be 20%% after 9th step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "update: W should be 20%% after 9th step");
}

/* ------------------------------------------------------------ */
/* GROUP 8 - IfxGtm_periodEventFunction: configuration values   */
/* ------------------------------------------------------------ */
void test_TC_G8_001_period_callback_does_not_change_config_or_call_hal(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t updates_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(updates_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "period callback: must not issue duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "period callback: must not alter configured PWM frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "period callback: must not alter configured channel count");
}

void test_TC_G8_002_period_callback_does_not_modify_last_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* establish a known lastDuties snapshot */

    float u_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float v_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float w_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
    uint32_t updates_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(updates_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "period callback: must not invoke duty update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, u_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "period callback: U duty must remain unchanged");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, v_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "period callback: V duty must remain unchanged");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, w_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "period callback: W duty must remain unchanged");
}

/* ------------------------------------------------------------ */
/* GROUP 9 - IfxGtm_periodEventFunction: ISR / behavior         */
/* ------------------------------------------------------------ */
void test_TC_G9_001_period_callback_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t toggles_before = mock_IfxPort_togglePin_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggles_before, mock_IfxPort_togglePin_getCallCount(), "period callback: must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "period callback: raw toggle count must remain zero");
}

void test_TC_G9_002_multiple_period_callbacks_do_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i) {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxPort_togglePin_getCallCount(), "period callback: repeated calls must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "period callback: repeated calls must leave raw toggle count at zero");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_when_GTM_already_enabled_does_not_enable_or_configure_clocks);
    RUN_TEST(test_TC_G1_002_init_when_GTM_disabled_enables_and_configures_clocks_once);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_config_sets_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_init_applies_initial_duty_and_deadtime);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_after_init_increments_duty_by_step_and_updates_once);
    RUN_TEST(test_TC_G3_002_update_wraps_only_channels_reaching_boundary);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_init_configures_led_gpio_once);
    RUN_TEST(test_TC_G4_002_no_led_toggle_during_init_or_without_isr);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_does_not_reinitialize_or_touch_clocks_when_gtm_pre_enabled);
    RUN_TEST(test_TC_G5_002_update_does_not_reenable_or_reconfigure_clocks_after_init_guarded_path);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_update_maintains_config_frequency_and_channel_count);
    RUN_TEST(test_TC_G6_002_update_once_sets_expected_duties);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_multiple_updates_eventually_wrap_all_channels_to_step_value);
    RUN_TEST(test_TC_G7_002_additional_update_continues_from_wrapped_value);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_period_callback_does_not_change_config_or_call_hal);
    RUN_TEST(test_TC_G8_002_period_callback_does_not_modify_last_duties);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_period_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G9_002_multiple_period_callbacks_do_not_toggle_led);

    return UNITY_END();
}
