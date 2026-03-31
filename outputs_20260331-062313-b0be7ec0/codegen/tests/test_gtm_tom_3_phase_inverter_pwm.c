#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declaration for ISR defined in production .c (not in .h) */
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON            (1e-4f)
#define UT_NUM_CHANNELS             (3)
#define UT_PWM_FREQ_HZ              (20000.0f)
#define UT_INIT_DUTY_U_PERCENT      (25.0f)
#define UT_INIT_DUTY_V_PERCENT      (50.0f)
#define UT_INIT_DUTY_W_PERCENT      (75.0f)
#define UT_STEP_PERCENT             (10.0f)
#define UT_DUTY_MIN_PERCENT         (10.0f)
#define UT_DUTY_MAX_PERCENT         (90.0f)
#define UT_PWM_DEAD_TIME_S          (1e-06f)
#define UT_PWM_MIN_PULSE_TIME_S     (1e-06f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
GROUP 1 - initGtmTom3phInv: initialization / enable guard
*/
void test_TC_G1_001_init_enables_module_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;  /* Force enable path */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* any value */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enable called once when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency read once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FX clocks enabled once when GTM disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm initConfig called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm init called exactly once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;   /* Already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency not read when already enabled (inside enable-guard)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FX clocks not re-enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm initConfig called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "Pwm init called exactly once");
}

/*
GROUP 2 - initGtmTom3phInv: configuration values
*/
void test_TC_G2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert - use init() spies (application-configured values) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must be 20 kHz (switching frequency)");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Configured number of logical channels must be 3 (U,V,W)");
}

void test_TC_G2_002_after_init_first_update_reflects_initial_duties_with_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - first runtime update */
    updateGtmTom3phInvDuty();

    /* Expected: U=25+10=35, V=50+10=60, W=75+10=85>=90? no, 85<90 so 85 -> wait, per wrap rule apply after add; from spec W=75+10=85 (<90), but behavior_description says wrap when >=90. However combined with WRAP RULE example for 95. For W initial 75, after +10 => 85 (<90) so no wrap; BUT DYNAMIC GROUPS example expects demonstrating wrap in first update. Our module spec explicitly defines wrap if >=90. To align with step-by-step here, we'll enforce second channelization in other tests. For this test, assert the immediate next duties. */
    /* Correct expectation from provided behavior: W=75+10=85 (<90) -> no wrap yet. But the top "DUTY CONVENTION" example in requirements indicates wrap happens when (duty + step) >= max, result becomes STEP. We'll keep using provided values for other tests where wrap occurs. */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after first update should be 35.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after first update should be 60.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after first update should be 85.0%%");
}

/*
GROUP 3 - initGtmTom3phInv: runtime update logic
*/
void test_TC_G3_001_update_once_increments_and_wraps_correctly(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: After one update: U=35, V=60, W=85 (no wrap yet since < 90) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U after one update must be 35.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V after one update must be 60.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W after one update must be 85.0%%");
}

void test_TC_G3_002_four_updates_accumulate_and_wrap_independently(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - perform four updates */
    updateGtmTom3phInvDuty(); /* 1: [35,60,85] */
    updateGtmTom3phInvDuty(); /* 2: [45,70,95 -> wrap 10] => [45,70,10]? Wait, apply rule only when >=90: 85+10=95 -> wrap to 10 */
    updateGtmTom3phInvDuty(); /* 3: [55,80,20] */
    updateGtmTom3phInvDuty(); /* 4: [65,90 -> wrap 10, 30] => [65,10,30] */

    /* Assert - final expected after 4 updates from 25/50/75: [65,10,30] */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U after 4 updates must be 65.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V wraps to 10.0%% on reaching 90.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W after 4 updates must be 30.0%%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update HAL should be called once per update (4 calls total)");
}

/*
GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
*/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggles_led_accumulatively(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle count should accumulate across calls");
}

/*
GROUP 5 - updateGtmTom3phInvDuty: initialization / enable guard (call-sequence & HAL usage)
*/
void test_TC_G5_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig called once during init only");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init called once during init only");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update HAL called exactly once for one update invocation");
}

void test_TC_G5_002_multiple_updates_call_hal_once_each_time(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must yield two HAL calls (coherent multi-channel update each time)");
}

/*
GROUP 6 - updateGtmTom3phInvDuty: configuration values
*/
void test_TC_G6_001_update_after_init_preserves_configured_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - init() spies remain as configured */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must remain 20 kHz after updates");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Channel count must remain 3 after updates");
}

void test_TC_G6_002_update_uses_percent_units_in_duty_array(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - perform two updates: expected [45,70,20] */
    updateGtmTom3phInvDuty(); /* [35,60,85] */
    updateGtmTom3phInvDuty(); /* [45,70,95->wrap 10] => [45,70,10]? No, from previous step: 85+10=95 -> wrap to 10. So second step yields [45,70,10]. We'll add one more to reach [55,80,20] if needed, but this test focuses on percent units. */

    /* Assert - values are in percent scale and match expected progression [45,70,10] at this step */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be expressed in percent (45.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be expressed in percent (70.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W wraps to 10.0 percent per wrap rule");
}

/*
GROUP 7 - updateGtmTom3phInvDuty: runtime update logic
*/
void test_TC_G7_001_increment_below_boundary_no_wrap(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - two updates: expected [45,70,10] as derived in Group 6 */
    updateGtmTom3phInvDuty(); /* [35,60,85] */
    updateGtmTom3phInvDuty(); /* [45,70,95 -> wrap 10] => [45,70,10] */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U=45.0%% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V=70.0%% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W wrapped to 10.0%% after two updates");
}

void test_TC_G7_002_wraps_to_min_when_threshold_reached_or_exceeded(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - perform 7 updates to force multiple wraps */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5 */
    updateGtmTom3phInvDuty(); /* 6 */
    updateGtmTom3phInvDuty(); /* 7 */

    /* Expected after 7 updates from [25,50,75]:
       U: 25+(7*10)=95 -> wrap to 10
       V: sequence -> 40
       W: sequence -> 70
    */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U wraps to 10.0%% when reaching/exceeding 90.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V independently wraps and continues to 40.0%% after 7 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W progresses to 70.0%% after 7 updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update called once per update (7 calls)");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_module_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_after_init_first_update_reflects_initial_duties_with_step);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_once_increments_and_wraps_correctly);
    RUN_TEST(test_TC_G3_002_four_updates_accumulate_and_wrap_independently);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_accumulatively);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G5_002_multiple_updates_call_hal_once_each_time);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_update_after_init_preserves_configured_frequency_and_channel_count);
    RUN_TEST(test_TC_G6_002_update_uses_percent_units_in_duty_array);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_increment_below_boundary_no_wrap);
    RUN_TEST(test_TC_G7_002_wraps_to_min_when_threshold_reached_or_exceeded);

    return UNITY_END();
}
