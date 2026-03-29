#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                           (1e-4f)
#define UT_NUM_CHANNELS                            (3)
#define UT_PWM_FREQUENCY_HZ                        (20000)
#define UT_INIT_DUTY_U_PERCENT                     (25.0f)
#define UT_INIT_DUTY_V_PERCENT                     (50.0f)
#define UT_INIT_DUTY_W_PERCENT                     (75.0f)
#define UT_DEAD_TIME_TICKS                         (50)      /* 0.5 us @ FXCLK0=100 MHz */
#define UT_MIN_PULSE_TICKS                         (100)     /* 1.0 us @ FXCLK0=100 MHz */
#define UT_GTM_MODULE_FREQ_HZ                      (300000000u) /* 300 MHz */
#define UT_FXCLK0_TICKS_PER_US                     (100)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
GROUP 1 - initGtmTomPwm: initialization / enable guard
*/
void test_TC_G1_001_enable_guard_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must NOT be enabled when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must NOT be used by this module");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

void test_TC_G1_002_enable_guard_needs_enable_and_clock_setup(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_MODULE_FREQ_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must be called once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must NOT be used by this module");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
}

void test_TC_G1_003_init_sequence_once_each(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* path independent of enable for this check */

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once before init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must be called exactly once");
}

/*
GROUP 2 - initGtmTomPwm: configuration values
*/
void test_TC_G2_001_config_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - number of logical channels (complementary pairs counted as 1) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxGtm_Pwm_init_lastNumChannels, "Number of PWM logical channels must match 3-phase (3)");

    /* Assert - PWM switching frequency set to 20 kHz on init() config */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency on init() must be 20 kHz");

    /* Also check the immediate frequency update uses the same 20 kHz */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_updateFrequencyImmediate_lastFrequency, "Immediate frequency update must be 20 kHz");
}

void test_TC_G2_002_config_initial_duty_and_deadtime_values(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - initial duties (percent) U=25, V=50, W=75 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Initial U duty must be 25% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Initial V duty must be 50% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Initial W duty must be 75% (percent units)");

    /* Assert - dead-time ticks applied equally to rising and falling edges for all 3 channels */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[0], "Dead-time rising[0] must be 0.5us in ticks");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[1], "Dead-time rising[1] must be 0.5us in ticks");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[2], "Dead-time rising[2] must be 0.5us in ticks");

    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[0], "Dead-time falling[0] must be 0.5us in ticks");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[1], "Dead-time falling[1] must be 0.5us in ticks");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_DEAD_TIME_TICKS, (int)mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[2], "Dead-time falling[2] must be 0.5us in ticks");
}

/*
GROUP 3 - initGtmTomPwm: runtime update logic (calls issued during init)
*/
void test_TC_G3_001_runtime_update_calls_once_each(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - driver updates are invoked exactly once (not per channel) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount(), "Frequency immediate update must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty immediate update must be called once for all channels (atomic)");
}

void test_TC_G3_002_runtime_update_frequency_value_is_pwm_frequency_not_module_clock(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_MODULE_FREQ_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert - ensure PWM update uses switching frequency (20 kHz), not module clock */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (float)UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_updateFrequencyImmediate_lastFrequency, "Immediate frequency update must use PWM frequency (20 kHz), not module frequency");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_enable_guard_already_enabled);
    RUN_TEST(test_TC_G1_002_enable_guard_needs_enable_and_clock_setup);
    RUN_TEST(test_TC_G1_003_init_sequence_once_each);

    RUN_TEST(test_TC_G2_001_config_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_config_initial_duty_and_deadtime_values);

    RUN_TEST(test_TC_G3_001_runtime_update_calls_once_each);
    RUN_TEST(test_TC_G3_002_runtime_update_frequency_value_is_pwm_frequency_not_module_clock);

    return UNITY_END();
}
