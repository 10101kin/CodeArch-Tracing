#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_NUM_CHANNELS                  (3U)
#define UT_INIT_DUTY_U                   (0.25f)
#define UT_INIT_DUTY_V                   (0.50f)
#define UT_INIT_DUTY_W                   (0.75f)
#define UT_DUTY_STEP                     (0.10f)
#define UT_TRUE_ZERO                     (0.0f)
#define UT_TRUE_FULL                     (1.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**************
 * GROUP 1 - initGtmTomPwm: initialization / enable guard
 **************/
void test_TC_G1_001_init_enables_gtm_clocks_and_inits_pwm_once(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1; /* allow timer init to succeed */

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enable should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "GTM CMU enableClocks should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "PWM initConfig should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "PWM init should be called exactly once during init");
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_Timer_addToChannelMask_getCallCount() >= 1U), "Timer addToChannelMask should be called at least once to include all three channels in update mask");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer run should be called exactly once to start the timebase");
}

void test_TC_G1_002_init_handles_timer_init_failure_gracefully(void)
{
    /* Arrange: force timer init to fail */
    mock_IfxGtm_Tom_Timer_init_returnValue = 0;

    /* Act */
    initGtmTomPwm();

    /* Assert: GTM enable path executed, but PWM init and run must not occur on failure */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enable should be attempted even if timer init fails");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should be enabled before timer init attempt");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig should be called despite later failure");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init should be called and fail");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_initConfig_getCallCount(), "PWM initConfig should NOT be called when timer init fails");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "PWM init should NOT be called when timer init fails");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer should NOT be started when timer init fails");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "disableUpdate should NOT be called when init fails early");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "No channel duty update should be staged when init fails");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "No shadow transfer should be applied when init fails");
}

/**************
 * GROUP 2 - initGtmTomPwm: configuration values
 **************/
void test_TC_G2_001_init_config_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM should be initialized with exactly 3 channels (U/V/W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM requested frequency should be 20 kHz");
}

void test_TC_G2_002_init_stages_initial_duties_and_applies_shadow_transfer(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - one-time shadowed commit of initial duties */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "disableUpdate should be called once to stage initial duty update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "updateChannelsDuty should be called once to stage all three initial duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "applyUpdate should be called once to commit staged duties synchronously");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[0], "Initial duty for U should be 25% (0.25)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[1], "Initial duty for V should be 50% (0.50)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[2], "Initial duty for W should be 75% (0.75)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(), "Dead-time should not be configured for single-ended PWM (call count must be 0)");
}

/**************
 * GROUP 3 - initGtmTomPwm: runtime update logic (init-time sequencing)
 **************/
void test_TC_G3_001_init_performs_disable_update_apply_sequence_once(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert - exact one-time sequence counts */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "At init, disableUpdate should be called once before staging");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "At init, updateChannelsDuty should be called once for all channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "At init, applyUpdate should be called once to commit shadowed duties");
}

void test_TC_G3_002_init_updates_input_frequency_once(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_updateInputFrequency_getCallCount(), "Timer input frequency should be updated exactly once during init");
}

/**************
 * GROUP 4 - updateGtmTomPwmDutyCycles: configuration values
 **************/
void test_TC_G4_001_after_first_update_frequency_and_channel_count_remain_configured(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels should remain 3 after an update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency should remain 20 kHz after an update");
}

void test_TC_G4_002_no_deadtime_calls_during_update_sequence(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(), "Dead-time API must not be used during runtime updates for single-ended PWM");
}

/**************
 * GROUP 5 - updateGtmTomPwmDutyCycles: runtime update logic
 **************/
void test_TC_G5_001_single_increment_below_boundary_updates_all_channels_once(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    initGtmTomPwm(); /* baseline: one disable/update/apply already performed */

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert: duties incremented by +10% and single HAL call used */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[0], "U duty should increment from 25% to 35% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[1], "V duty should increment from 50% to 60% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[2], "W duty should increment from 75% to 85% on first update");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "disableUpdate should be called once at init and once at first runtime update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "updateChannelsDuty should be called exactly once per update cycle (init + 1 update)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "applyUpdate should be called once at init and once at first runtime update");
}

void test_TC_G5_002_wrap_around_and_true_full_scale_behavior(void)
{
    /* Arrange */
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    initGtmTomPwm(); /* baseline initial duties: 0.25, 0.50, 0.75 */

    /* Act: perform 5 updates (500 ms step in app, but unit tests call directly) */
    for (int i = 0; i < 5; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    /* After 5 updates: U=0.75, V=1.00 (true full), W wraps at 3rd update -> 0.20 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.75f, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[0], "After 5 updates, U duty should be 75% (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_TRUE_FULL, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[1], "After 5 updates, V duty should reach true 100% and hold");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.20f, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[2], "After 5 updates, W duty should have wrapped to 20%");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(6, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "disableUpdate should be called once at init plus once per each of 5 updates (total 6)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(6, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "updateChannelsDuty should be called once at init plus once per each of 5 updates (total 6)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(6, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "applyUpdate should be called once at init plus once per each of 5 updates (total 6)");

    /* Next update: V should wrap from 100% to 0% */
    updateGtmTomPwmDutyCycles();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.85f, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[0], "After 6 updates, U duty should be 85%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_TRUE_ZERO, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[1], "After 6 updates, V duty should wrap from 100% to 0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.30f, mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[2], "After 6 updates, W duty should be 30%");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "disableUpdate should accumulate to init + 6 updates (total 7)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(), "updateChannelsDuty should accumulate to init + 6 updates (total 7)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "applyUpdate should accumulate to init + 6 updates (total 7)");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_gtm_clocks_and_inits_pwm_once);
    RUN_TEST(test_TC_G1_002_init_handles_timer_init_failure_gracefully);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_config_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_stages_initial_duties_and_applies_shadow_transfer);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_init_performs_disable_update_apply_sequence_once);
    RUN_TEST(test_TC_G3_002_init_updates_input_frequency_once);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_after_first_update_frequency_and_channel_count_remain_configured);
    RUN_TEST(test_TC_G4_002_no_deadtime_calls_during_update_sequence);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_single_increment_below_boundary_updates_all_channels_once);
    RUN_TEST(test_TC_G5_002_wrap_around_and_true_full_scale_behavior);

    return UNITY_END();
}
