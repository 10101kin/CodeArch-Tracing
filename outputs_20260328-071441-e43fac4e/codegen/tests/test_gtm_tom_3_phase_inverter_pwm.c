#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQUENCY_HZ             (20000U)
#define UT_NUM_CHANNELS                 (6U)
#define UT_INIT_DUTY_U                  (0.25f)
#define UT_INIT_DUTY_V                  (0.50f)
#define UT_INIT_DUTY_W                  (0.75f)
#define UT_DEAD_TIME_US                 (0.5f)
#define UT_MIN_PULSE_US                 (1.0f)
#define UT_DEVICE_EXPECTED_SYS_FREQ_HZ  (300000000U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initGtmTomPwm: initialization / enable guard */
void test_TC_G1_001_init_calls_config_and_init_once_when_gtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must NOT be called when GTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when GTM already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must NOT be called when GTM already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

void test_TC_G1_002_init_enables_module_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_DEVICE_EXPECTED_SYS_FREQ_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must be called when GTM is disabled");
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Cmu_getModuleFrequency_getCallCount() >= 1), "CMU getModuleFrequency must be called at least once when enabling clocks");
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Cmu_enableClocks_getCallCount() >= 1), "CMU enableClocks must be called at least once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

/* GROUP 2 - initGtmTomPwm: configuration values */
void test_TC_G2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned)mock_IfxGtm_Pwm_init_lastNumChannels, "Configured number of channels must be 6 (3 complementary pairs)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (unsigned)mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned)mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must reflect 6 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (unsigned)mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig must reflect 20 kHz frequency");
}

void test_TC_G2_002_updateFrequencyImmediate_requested_with_pwm_frequency_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount() >= 1), "updateFrequencyImmediate should be requested at least once after init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (unsigned)mock_IfxGtm_Pwm_updateFrequencyImmediate_lastRequestFrequency, "Requested frequency must be 20 kHz");
}

/* GROUP 3 - initGtmTomPwm: runtime update logic (frequency apply behavior) */
void test_TC_G3_001_init_does_not_push_duty_updates_during_initialization(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty updates should be pushed during init (shadow-update handled at runtime)");
}

void test_TC_G3_002_init_requests_frequency_apply_via_updateFrequencyImmediate(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount() >= 1), "updateFrequencyImmediate must be called to apply initial frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (unsigned)mock_IfxGtm_Pwm_updateFrequencyImmediate_lastRequestFrequency, "Applied frequency must be 20 kHz");
}

/* GROUP 4 - updateGtmTomPwmDutyCycles: initialization / enable guard */
void test_TC_G4_001_update_calls_bulk_duty_update_once_per_call(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* pre-enabled path */
    initGtmTomPwm();
    uint32_t baselineUpdateCalls = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    uint32_t baselineEnableCalls = mock_IfxGtm_enable_getCallCount();
    uint32_t baselineInitCalls   = mock_IfxGtm_Pwm_init_getCallCount();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineUpdateCalls + 1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty bulk update must be called exactly once per update invocation");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineEnableCalls, mock_IfxGtm_enable_getCallCount(), "No additional GTM enable calls should occur during runtime updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineInitCalls, mock_IfxGtm_Pwm_init_getCallCount(), "Driver init must not be re-invoked by updates");
}

void test_TC_G4_002_update_after_init_when_gtm_was_disabled_only_affects_duty_update_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* force enable/CMU path during init */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_DEVICE_EXPECTED_SYS_FREQ_HZ;
    initGtmTomPwm();
    uint32_t baselineUpdateCalls = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    uint32_t baselineEnableCalls = mock_IfxGtm_enable_getCallCount();
    uint32_t baselineInitCalls   = mock_IfxGtm_Pwm_init_getCallCount();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineUpdateCalls + 1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one duty update call per update() even after enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineEnableCalls, mock_IfxGtm_enable_getCallCount(), "GTM must not be re-enabled during updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baselineInitCalls, mock_IfxGtm_Pwm_init_getCallCount(), "Driver init must not be called by updates");
}

/* GROUP 5 - updateGtmTomPwmDutyCycles: configuration values */
void test_TC_G5_001_after_update_channel_count_and_frequency_remain_configured(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned)mock_IfxGtm_Pwm_init_lastNumChannels, "Channel count must remain 6 after updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (unsigned)mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after updates");
}

void test_TC_G5_002_after_update_all_six_duty_entries_within_0_to_1(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    for (int i = 0; i < 6; i++)
    {
        float d = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i];
        TEST_ASSERT_TRUE_MESSAGE((d >= 0.0f), "Duty entry must be >= 0.0");
        TEST_ASSERT_TRUE_MESSAGE((d <= 1.0f), "Duty entry must be <= 1.0");
    }
}

/* GROUP 6 - updateGtmTomPwmDutyCycles: runtime update logic */
void test_TC_G6_001_multiple_updates_call_hal_once_each_and_keep_duty_bounds(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();
    uint32_t baseline = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    const int N = 5;
    for (int i = 0; i < N; i++)
    {
        updateGtmTomPwmDutyCycles();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseline + N, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once per update() call");
    for (int i = 0; i < 6; i++)
    {
        float d = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i];
        TEST_ASSERT_TRUE_MESSAGE((d >= 0.0f), "Duty entry must be >= 0.0 after multiple updates");
        TEST_ASSERT_TRUE_MESSAGE((d <= 1.0f), "Duty entry must be <= 1.0 after multiple updates");
    }
}

void test_TC_G6_002_many_updates_still_within_bounds_and_counts_accumulate(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTomPwm();
    uint32_t baseline = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    const int N = 100;
    for (int i = 0; i < N; i++)
    {
        updateGtmTomPwmDutyCycles();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseline + N, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update call count must accumulate one per call across many updates");
    for (int i = 0; i < 6; i++)
    {
        float d = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i];
        TEST_ASSERT_TRUE_MESSAGE((d >= 0.0f), "Duty entry must be >= 0.0 after many updates");
        TEST_ASSERT_TRUE_MESSAGE((d <= 1.0f), "Duty entry must be <= 1.0 after many updates");
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_config_and_init_once_when_gtm_already_enabled);
    RUN_TEST(test_TC_G1_002_init_enables_module_and_configures_cmu_when_disabled);

    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_updateFrequencyImmediate_requested_with_pwm_frequency_after_init);

    RUN_TEST(test_TC_G3_001_init_does_not_push_duty_updates_during_initialization);
    RUN_TEST(test_TC_G3_002_init_requests_frequency_apply_via_updateFrequencyImmediate);

    RUN_TEST(test_TC_G4_001_update_calls_bulk_duty_update_once_per_call);
    RUN_TEST(test_TC_G4_002_update_after_init_when_gtm_was_disabled_only_affects_duty_update_count);

    RUN_TEST(test_TC_G5_001_after_update_channel_count_and_frequency_remain_configured);
    RUN_TEST(test_TC_G5_002_after_update_all_six_duty_entries_within_0_to_1);

    RUN_TEST(test_TC_G6_001_multiple_updates_call_hal_once_each_and_keep_duty_bounds);
    RUN_TEST(test_TC_G6_002_many_updates_still_within_bounds_and_counts_accumulate);

    return UNITY_END();
}
