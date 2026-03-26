#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern production callback symbol (empty callback) */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQ_HZ                  (20000U)
#define UT_NUM_CHANNELS                 (3U)
#define UT_DEADTIME_NS                  (1000U)
#define UT_DUTY_STEP                    (10.0f)
#define UT_DUTY_INIT_U                  (25.0f)
#define UT_DUTY_INIT_V                  (50.0f)
#define UT_DUTY_INIT_W                  (75.0f)
#define UT_DUTY_IDX_U                   (0)
#define UT_DUTY_IDX_V                   (1)
#define UT_DUTY_IDX_W                   (2)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ============================
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 * ============================ */
void test_TC_G1_001_init_when_gtm_enabled_should_not_enable_or_clock(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once when GTM is enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must NOT be enabled when GTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinLow_getCallCount(), "LED pin must be driven low once during init");
}

void test_TC_G1_002_init_when_gtm_disabled_should_enable_and_clock(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once when GTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when previously disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

/* ============================
 * GROUP 2 - initGtmTom3phInv: configuration values
 * ============================ */
void test_TC_G2_001_init_config_sets_frequency_and_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must be 20 kHz in IfxGtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3 in IfxGtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "Number of logical PWM channels must be 3 in IfxGtm_Pwm_initConfig");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "Configured PWM frequency must be 20 kHz in IfxGtm_Pwm_initConfig");
}

void test_TC_G2_002_led_gpio_configured_low_on_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as push-pull output exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinLow_getCallCount(), "LED GPIO must be driven low exactly once on init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "LED must NOT be toggled during initialization");
}

/* ============================
 * GROUP 3 - initGtmTom3phInv: runtime update logic
 * ============================ */
void test_TC_G3_001_first_update_after_init_increments_duty_by_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update HAL must be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_U], "Phase U duty must be 25% + 10% = 35% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_V], "Phase V duty must be 50% + 10% = 60% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_W], "Phase W duty must be 75% + 10% = 85% after first update");
}

void test_TC_G3_002_wrap_behavior_progressive_per_channel_after_multiple_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1: 35/60/85 */
    updateGtmTom3phInvDuty(); /* 2: 45/70/95 */
    updateGtmTom3phInvDuty(); /* 3: 55/80/10 (W wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per loop update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_U], "Phase U duty must be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_V], "Phase V duty must be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_W], "Phase W duty must wrap to 10% after exceeding 100%");
}

/* ============================
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
 * ============================ */
void test_TC_G4_001_init_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "LED must not toggle during initialization");
}

void test_TC_G4_002_multiple_initializations_do_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "LED must not toggle even if init is called multiple times");
}

/* ============================
 * GROUP 5 - updateGtmTom3phInvDuty: initialization / enable guard
 * ============================ */
void test_TC_G5_001_update_after_init_calls_hal_once_and_does_not_reinit(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t initCfgCount_before = mock_IfxGtm_Pwm_initConfig_getCallCount();
    uint32_t initCount_before    = mock_IfxGtm_Pwm_init_getCallCount();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once by a single update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(initCfgCount_before, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Update must NOT call IfxGtm_Pwm_initConfig again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(initCount_before, mock_IfxGtm_Pwm_init_getCallCount(), "Update must NOT call IfxGtm_Pwm_init again");
}

void test_TC_G5_002_two_updates_increment_hal_count_without_reinitialization(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    uint32_t initCfgCount_before = mock_IfxGtm_Pwm_initConfig_getCallCount();
    uint32_t initCount_before    = mock_IfxGtm_Pwm_init_getCallCount();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must call HAL twice (once per update)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(initCfgCount_before, mock_IfxGtm_Pwm_initConfig_getCallCount(), "No reconfiguration via IfxGtm_Pwm_initConfig on updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(initCount_before, mock_IfxGtm_Pwm_init_getCallCount(), "No re-initialization via IfxGtm_Pwm_init on updates");
}

/* ============================
 * GROUP 6 - updateGtmTom3phInvDuty: configuration values
 * ============================ */
void test_TC_G6_001_two_updates_set_expected_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 35/60/85 */
    updateGtmTom3phInvDuty(); /* 45/70/95 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_U], "Phase U duty must be 45% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_V], "Phase V duty must be 70% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_W], "Phase W duty must be 95% after two updates");
}

void test_TC_G6_002_channel_count_and_frequency_stable_across_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Logical channel count must remain 3 after updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured frequency must remain 20 kHz after updates");
}

/* ============================
 * GROUP 7 - updateGtmTom3phInvDuty: runtime update logic
 * ============================ */
void test_TC_G7_001_wrap_for_V_after_five_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1: 35/60/85 */
    updateGtmTom3phInvDuty(); /* 2: 45/70/95 */
    updateGtmTom3phInvDuty(); /* 3: 55/80/10 */
    updateGtmTom3phInvDuty(); /* 4: 65/90/20 */
    updateGtmTom3phInvDuty(); /* 5: 75/10/30 (V wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates must call HAL five times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_U], "Phase U duty must be 75% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_V], "Phase V duty must wrap to 10% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_W], "Phase W duty must be 30% after five updates");
}

void test_TC_G7_002_wrap_for_U_after_eight_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1: 35/60/85 */
    updateGtmTom3phInvDuty(); /* 2: 45/70/95 */
    updateGtmTom3phInvDuty(); /* 3: 55/80/10 */
    updateGtmTom3phInvDuty(); /* 4: 65/90/20 */
    updateGtmTom3phInvDuty(); /* 5: 75/10/30 */
    updateGtmTom3phInvDuty(); /* 6: 85/20/40 */
    updateGtmTom3phInvDuty(); /* 7: 95/30/50 */
    updateGtmTom3phInvDuty(); /* 8: 10/40/60 (U wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates must call HAL eight times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_U], "Phase U duty must wrap to 10% after eight updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_V], "Phase V duty must be 40% after eight updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_DUTY_IDX_W], "Phase W duty must be 60% after eight updates");
}

/* ============================
 * GROUP 8 - IfxGtm_periodEventFunction: configuration values
 * ============================ */
void test_TC_G8_001_period_callback_does_not_change_duty_or_toggle(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* Establish known duty array */
    uint32_t updateCount_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    uint32_t toggleCount_before = mock_IfxPort_togglePin_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(updateCount_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not trigger PWM duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggleCount_before, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED");
}

void test_TC_G8_002_period_callback_accepts_null_and_has_no_side_effects(void)
{
    /* Arrange */
    /* No init or update */

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Empty period callback must not update PWM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Empty period callback must not toggle LED");
}

/* ============================
 * GROUP 9 - IfxGtm_periodEventFunction: ISR / interrupt behavior
 * ============================ */
void test_TC_G9_001_multiple_period_callbacks_do_not_toggle_led(void)
{
    /* Arrange */
    /* No init required to verify no toggles happen */

    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "LED must not toggle on empty period callback invocations");
}

void test_TC_G9_002_multiple_period_callbacks_do_not_update_pwm(void)
{
    /* Arrange */

    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "PWM duties must not be updated by the empty period callback");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_when_gtm_enabled_should_not_enable_or_clock);
    RUN_TEST(test_TC_G1_002_init_when_gtm_disabled_should_enable_and_clock);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_config_sets_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_led_gpio_configured_low_on_init);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_after_init_increments_duty_by_step);
    RUN_TEST(test_TC_G3_002_wrap_behavior_progressive_per_channel_after_multiple_updates);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_init_does_not_toggle_led);
    RUN_TEST(test_TC_G4_002_multiple_initializations_do_not_toggle_led);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_after_init_calls_hal_once_and_does_not_reinit);
    RUN_TEST(test_TC_G5_002_two_updates_increment_hal_count_without_reinitialization);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_two_updates_set_expected_duties);
    RUN_TEST(test_TC_G6_002_channel_count_and_frequency_stable_across_updates);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_wrap_for_V_after_five_updates);
    RUN_TEST(test_TC_G7_002_wrap_for_U_after_eight_updates);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_period_callback_does_not_change_duty_or_toggle);
    RUN_TEST(test_TC_G8_002_period_callback_accepts_null_and_has_no_side_effects);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_multiple_period_callbacks_do_not_toggle_led);
    RUN_TEST(test_TC_G9_002_multiple_period_callbacks_do_not_update_pwm);

    return UNITY_END();
}
