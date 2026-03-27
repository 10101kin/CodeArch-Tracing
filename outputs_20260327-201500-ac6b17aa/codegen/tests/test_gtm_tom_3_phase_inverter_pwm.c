#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern production callback (empty by design) */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_NUM_CHANNELS                  (3U)
#define UT_DUTY_INIT_U                   (25)
#define UT_DUTY_INIT_V                   (50)
#define UT_DUTY_INIT_W                   (75)
#define UT_DUTY_STEP                     (10)
#define UT_DEADTIME_US_RISING            (1.0f)
#define UT_DEADTIME_US_FALLING           (1.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_TEST_MODULE_FREQ_HZ           (100000000U) /* 100 MHz for CMU mock */

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ---------------------------- GROUP 1: init - initialization / enable guard ---------------------------- */
void test_TC_G1_001_init_when_already_enabled_does_not_reconfigure_cmu(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable state must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must not be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must not be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must not be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK must not be reconfigured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK0 must not be reconfigured when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR/debug GPIO must be configured once");
}

void test_TC_G1_002_init_when_disabled_configures_cmu_and_pwm(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_TEST_MODULE_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable state must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK mask must be enabled once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK must be set equal to module frequency when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK0 must be derived when enabling");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR/debug GPIO must be configured once");
}

/* ---------------------------- GROUP 2: init - configuration values ---------------------------- */
void test_TC_G2_001_init_config_sets_frequency_and_channel_count_in_config(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must set 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig must set PWM frequency to 20 kHz");
}

void test_TC_G2_002_init_driver_frequency_and_channel_count_match_requirements(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver init must use 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Driver init must use PWM frequency 20 kHz");
}

/* ---------------------------- GROUP 3: init - runtime update logic (post-init update checks) ---------------------------- */
void test_TC_G3_001_first_update_after_init_increments_duty_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called exactly once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must step to 85%");
}

void test_TC_G3_002_three_updates_after_init_wraps_independently(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per API call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must wrap to 10% after 3 updates (75->85->95->wrap to 10)");
}

/* ---------------------------- GROUP 4: init - ISR / interrupt behavior ---------------------------- */
void test_TC_G4_001_init_configures_isr_gpio_pin_mode_output(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR/debug GPIO must be configured exactly once during init");
}

void test_TC_G4_002_no_isr_invocation_results_in_no_toggle_and_no_handler_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_interruptHandler_getCallCount(), "Interrupt handler must not be called without ISR trigger");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Debug GPIO must not toggle without ISR trigger");
}

/* ---------------------------- GROUP 5: update - configuration values (sanity with update) ---------------------------- */
void test_TC_G5_001_update_once_confirms_channel_count_and_frequency_remain_correct(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver must remain configured for 3 channels after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Driver PWM frequency remains 20 kHz after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one duty update call expected");
}

void test_TC_G5_002_update_duty_values_within_0_to_100_range(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    int d0 = (int)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    int d1 = (int)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    int d2 = (int)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
    TEST_ASSERT_TRUE_MESSAGE((d0 >= 0) && (d0 < 100), "U duty must be within [0,100)");
    TEST_ASSERT_TRUE_MESSAGE((d1 >= 0) && (d1 < 100), "V duty must be within [0,100)");
    TEST_ASSERT_TRUE_MESSAGE((d2 >= 0) && (d2 < 100), "W duty must be within [0,100)");
}

/* ---------------------------- GROUP 6: update - runtime update logic ---------------------------- */
void test_TC_G6_001_ten_updates_return_to_initial_duties_and_callcount_matches(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 10; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per request (10x)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_DUTY_INIT_U, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must return to initial after 10 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_DUTY_INIT_V, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must return to initial after 10 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_DUTY_INIT_W, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must return to initial after 10 updates");
}

void test_TC_G6_002_eight_updates_wrap_points_are_independent(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called 8 times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must wrap to 10% after 8 updates (25->...->95->10)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 60% after 8 updates");
}

/* ---------------------------- GROUP 7: IfxGtm_periodEventFunction - configuration values (no-ops) ---------------------------- */
void test_TC_G7_001_period_callback_is_empty_no_hal_calls_or_toggles(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_interruptHandler_getCallCount(), "Period callback must not call interrupt handler");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle debug pin");
}

void test_TC_G7_002_period_callback_after_init_does_not_modify_last_duties_or_counts(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* establish baseline duties */
    uint32_t beforeCalls = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    float beforeU = (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float beforeV = (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float beforeW = (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(beforeCalls, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not change duty update call count");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeU, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must remain unchanged by period callback");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeV, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must remain unchanged by period callback");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, beforeW, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must remain unchanged by period callback");
}

/* ---------------------------- GROUP 8: IfxGtm_periodEventFunction - runtime update logic (no-ops) ---------------------------- */
void test_TC_G8_001_multiple_period_callbacks_have_no_side_effects(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Callback must not trigger duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Callback must not toggle debug pin");
}

void test_TC_G8_002_period_callback_does_not_invoke_pwm_interrupt_handler(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_interruptHandler_getCallCount(), "Callback must not call PWM interrupt handler");
}

/* ---------------------------- GROUP 9: IfxGtm_periodEventFunction - ISR / interrupt behavior (no-ops) ---------------------------- */
void test_TC_G9_001_period_callback_does_not_toggle_pin_even_when_called_multiple_times(void)
{
    /* Act */
    for (int i = 0; i < 4; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Debug pin must not toggle from period callback invocations");
}

void test_TC_G9_002_period_callback_does_not_issue_duty_updates(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update calls expected from period callback");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_when_already_enabled_does_not_reconfigure_cmu);
    RUN_TEST(test_TC_G1_002_init_when_disabled_configures_cmu_and_pwm);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_config_sets_frequency_and_channel_count_in_config);
    RUN_TEST(test_TC_G2_002_init_driver_frequency_and_channel_count_match_requirements);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_after_init_increments_duty_below_boundary);
    RUN_TEST(test_TC_G3_002_three_updates_after_init_wraps_independently);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_init_configures_isr_gpio_pin_mode_output);
    RUN_TEST(test_TC_G4_002_no_isr_invocation_results_in_no_toggle_and_no_handler_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_once_confirms_channel_count_and_frequency_remain_correct);
    RUN_TEST(test_TC_G5_002_update_duty_values_within_0_to_100_range);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_ten_updates_return_to_initial_duties_and_callcount_matches);
    RUN_TEST(test_TC_G6_002_eight_updates_wrap_points_are_independent);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_period_callback_is_empty_no_hal_calls_or_toggles);
    RUN_TEST(test_TC_G7_002_period_callback_after_init_does_not_modify_last_duties_or_counts);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_multiple_period_callbacks_have_no_side_effects);
    RUN_TEST(test_TC_G8_002_period_callback_does_not_invoke_pwm_interrupt_handler);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_period_callback_does_not_toggle_pin_even_when_called_multiple_times);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_issue_duty_updates);

    return UNITY_END();
}
