#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_MIN_DUTY_PERCENT              (10.0f)
#define UT_MAX_DUTY_PERCENT              (90.0f)
#define UT_DEAD_TIME_S                   (1e-06f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
Group 1 - initGtmTom3phInv: initialization / enable guard
*/
void test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE; /* simulate GTM disabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "init() must query GTM enabled state once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "init() must enable GTM when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "init() must read GTM CMU module frequency when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "init() must enable CMU FX clocks for TOM when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "init() must call IfxGtm_Pwm_initConfig exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init() must call IfxGtm_Pwm_init exactly once");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Configured number of channels must be 3");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE; /* simulate GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "init() must query GTM enabled state once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "init() must not enable GTM when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "init() should not read CMU module frequency when GTM is already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "init() should not re-enable CMU FX clocks when GTM is already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "init() must call IfxGtm_Pwm_initConfig once regardless of GTM state");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init() must call IfxGtm_Pwm_init once regardless of GTM state");
}

/*
Group 2 - initGtmTom3phInv: configuration values
*/
void test_TC_G2_001_init_sets_pwm_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init() must program PWM frequency to 20000.0 Hz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init() must configure exactly 3 logical PWM channels (U,V,W)");
}

void test_TC_G2_002_init_does_not_call_immediate_duty_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "init() must rely on sync start and not call immediate duty update");
}

/*
Group 3 - initGtmTom3phInv: runtime update logic (validated via first update after init)
*/
void test_TC_G3_001_first_update_increments_duties_from_initial_values(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One update must call the HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must increment by +10%% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must increment by +10%% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must increment by +10%% on first update");
}

void test_TC_G3_002_second_update_wraps_W_phase_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 25,50,75 -> 35,60,85 */
    updateGtmTom3phInvDuty(); /* 35,60,85 -> 45,70,10 (wrap W) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must call the HAL twice");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after second update must be 45%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after second update must be 70%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_MIN_DUTY_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must wrap to 10%% when reaching >=90%%");
}

/*
Group 4 - initGtmTom3phInv: ISR / interrupt behavior (none used)
*/
void test_TC_G4_001_init_does_not_install_isr(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxCpu_Irq_installInterruptHandler_getCallCount(), "init() must not install any ISR handlers");
}

void test_TC_G4_002_init_does_not_start_or_stop_channel_outputs(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_startSyncedChannels_getCallCount(), "init() must not call startSyncedChannels()");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_stopSyncedChannels_getCallCount(), "init() must not call stopSyncedChannels()");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_startChannelOutputs_getCallCount(), "init() must not call startChannelOutputs()");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_stopChannelOutputs_getCallCount(), "init() must not call stopChannelOutputs()");
}

/*
Group 5 - updateGtmTom3phInvDuty: initialization / enable guard (call behavior)
*/
void test_TC_G5_001_update_calls_driver_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update() must call HAL exactly once per invocation");
}

void test_TC_G5_002_two_updates_call_driver_twice(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must call HAL twice");
}

/*
Group 6 - updateGtmTom3phInvDuty: configuration values
*/
void test_TC_G6_001_update_uses_percent_units_not_fraction(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* Expected: 35, 60, 85 percent */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 35.0 (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 60.0 (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 85.0 (percent units)");
}

void test_TC_G6_002_update_values_within_allowed_range(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_MESSAGE((mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= UT_MIN_DUTY_PERCENT) && (mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= UT_MAX_DUTY_PERCENT), "U duty must remain within [10%, 90%]");
    TEST_ASSERT_MESSAGE((mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= UT_MIN_DUTY_PERCENT) && (mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= UT_MAX_DUTY_PERCENT), "V duty must remain within [10%, 90%]");
    TEST_ASSERT_MESSAGE((mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= UT_MIN_DUTY_PERCENT) && (mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= UT_MAX_DUTY_PERCENT), "W duty must remain within [10%, 90%]");
}

/*
Group 7 - updateGtmTom3phInvDuty: runtime update logic
*/
void test_TC_G7_001_wrap_occurs_independently_per_channel(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    /* Apply 4 updates:
       U: 25->35->45->55->65
       V: 50->60->70->80->(wrap at 4th)10
       W: 75->85->(wrap)10->20 */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Four updates must call HAL four times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 4 updates must be 65%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must wrap to 10%% on reaching 90%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 4 updates must be 20%% (wrapped at 2nd update)");
}

void test_TC_G7_002_multiple_updates_result_in_expected_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 10 updates */
    for (int i = 0; i < 10; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Expected after 10 updates: U=40, V=70, W=10 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Ten updates must call HAL ten times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 10 updates must be 40%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 10 updates must be 70%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 10 updates must be 10%%");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_num_channels);
    RUN_TEST(test_TC_G2_002_init_does_not_call_immediate_duty_update);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_duties_from_initial_values);
    RUN_TEST(test_TC_G3_002_second_update_wraps_W_phase_only);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_init_does_not_install_isr);
    RUN_TEST(test_TC_G4_002_init_does_not_start_or_stop_channel_outputs);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_calls_driver_once_per_invocation);
    RUN_TEST(test_TC_G5_002_two_updates_call_driver_twice);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_update_uses_percent_units_not_fraction);
    RUN_TEST(test_TC_G6_002_update_values_within_allowed_range);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_wrap_occurs_independently_per_channel);
    RUN_TEST(test_TC_G7_002_multiple_updates_result_in_expected_duties);

    return UNITY_END();
}
