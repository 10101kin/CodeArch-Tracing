#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for non-header production symbols */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQUENCY                 (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_MAX_PERCENT                   (100.0f)
#define UT_DEADTIME_RISE_US              (1.0f)
#define UT_DEADTIME_FALL_US              (1.0f)
#define UT_CMU_MODULE_FREQ_HZ            (100000000U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* =========================
 * GROUP 1 - init: enable guard and core calls
 * ========================= */
void test_TC_G1_001_init_when_disabled_enables_gtm_and_clocks(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_CMU_MODULE_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard check called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled when initially disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks enabled when GTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxPort_setPinMode_getCallCount(), "Generic setPinMode should not be used for LED output");
}

void test_TC_G1_002_init_when_already_enabled_skips_enable(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;  /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_CMU_MODULE_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard check called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_enable_getCallCount(), "GTM enable should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks enable should be skipped when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig should still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_init_getCallCount(), "init should still be called once");
}

/* =========================
 * GROUP 2 - init: configuration values
 * ========================= */
void test_TC_G2_001_init_sets_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
}

void test_TC_G2_002_init_does_not_apply_duty_update_or_deadtime_immediate_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duties must not be applied during init (defer to first update)");
    /* Dead-time immediate update API should not be used during init in this implementation path */
}

/* =========================
 * GROUP 3 - init: runtime update logic entry (first updates after init)
 * ========================= */
void test_TC_G3_001_first_update_applies_initial_plus_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL duty update per call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after first update = 25+10=35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after first update = 50+10=60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after first update = 75+10=85%");
}

void test_TC_G3_002_third_update_wraps_only_W_to_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 3 updates total */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Expected after 3 updates: U=55, V=80, W wraps to 10 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three HAL updates after three calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3 updates should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3 updates should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty wraps to step (10%) after reaching 95% then updating");
}

/* =========================
 * GROUP 4 - init: ISR / interrupt behavior (LED toggle)
 * ========================= */
void test_TC_G4_001_isr_single_toggle(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "ISR should toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_multiple_toggles_accumulate(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_togglePin_callCount, "ISR toggle count should accumulate across calls");
}

/* =========================
 * GROUP 5 - update: configuration values and call pattern
 * ========================= */
void test_TC_G5_001_update_immediate_call_count_once_per_call(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act & Assert */
    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "First update calls HAL exactly once");

    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Second update calls HAL exactly once more");
}

void test_TC_G5_002_update_passes_three_percentage_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: three logical channels, percent values */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver initialized with 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U passed as percent (35.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V passed as percent (60.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W passed as percent (85.0)");
}

/* =========================
 * GROUP 6 - update: runtime update logic (wrap and independence)
 * ========================= */
void test_TC_G6_001_wrap_logic_after_five_updates_V_equals_step_and_others_progress(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 5 updates */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5 */

    /* After 5 updates: U=75, V=10 (wrapped), W=30 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five HAL updates after five calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 5 updates should be 75%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty wraps to step (10%) at boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty continues progression after earlier wrap");
}

void test_TC_G6_002_hal_called_once_per_update_after_seven_calls_and_values_match(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 7 updates */
    for (unsigned int i = 0; i < 7U; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* After 7 updates: U=95, V=30, W=50 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL called once per update (7 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 7 updates should be 95%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 7 updates should be 30% (post-wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 7 updates should be 50% (post-wrap progression)");
}

/* =========================
 * GROUP 7 - interruptGtmAtom: ISR / interrupt behavior
 * ========================= */
void test_TC_G7_001_interruptGtmAtom_toggles_led(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "interruptGtmAtom should toggle LED exactly once");
}

void test_TC_G7_002_interruptGtmAtom_does_not_change_pin_mode(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert: ISR shouldn't reconfigure pin mode */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxPort_setPinModeOutput_callCount, "ISR must not call setPinModeOutput");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxPort_setPinMode_callCount, "ISR must not call setPinMode");
}

/* =========================
 * GROUP 8 - IfxGtm_periodEventFunction: configuration values (no side effects)
 * ========================= */
void test_TC_G8_001_period_callback_has_no_side_effects(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void*)0);

    /* Assert: no toggle, no duty update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update duties");
}

void test_TC_G8_002_period_callback_does_not_toggle_after_isr_toggle(void)
{
    /* Arrange: toggle once via ISR */
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "Precondition: one toggle from ISR");

    /* Act: invoke period callback */
    IfxGtm_periodEventFunction((void*)0);

    /* Assert: toggle count unchanged */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "Period callback must not change toggle count");
}

/* =========================
 * GROUP 9 - IfxGtm_periodEventFunction: ISR behavior (confirm still empty)
 * ========================= */
void test_TC_G9_001_period_callback_multiple_invocations_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void*)0);
    IfxGtm_periodEventFunction((void*)0);
    IfxGtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
}

void test_TC_G9_002_period_callback_allows_null_argument_and_no_hal_calls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert: ensure no HAL calls occurred as a side effect */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "No LED toggle expected");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No PWM duty update expected");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_when_disabled_enables_gtm_and_clocks);
    RUN_TEST(test_TC_G1_002_init_when_already_enabled_skips_enable);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_num_channels);
    RUN_TEST(test_TC_G2_002_init_does_not_apply_duty_update_or_deadtime_immediate_calls);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_applies_initial_plus_step);
    RUN_TEST(test_TC_G3_002_third_update_wraps_only_W_to_step);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_single_toggle);
    RUN_TEST(test_TC_G4_002_isr_multiple_toggles_accumulate);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_immediate_call_count_once_per_call);
    RUN_TEST(test_TC_G5_002_update_passes_three_percentage_duties);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_wrap_logic_after_five_updates_V_equals_step_and_others_progress);
    RUN_TEST(test_TC_G6_002_hal_called_once_per_update_after_seven_calls_and_values_match);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_interruptGtmAtom_toggles_led);
    RUN_TEST(test_TC_G7_002_interruptGtmAtom_does_not_change_pin_mode);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_period_callback_has_no_side_effects);
    RUN_TEST(test_TC_G8_002_period_callback_does_not_toggle_after_isr_toggle);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_period_callback_multiple_invocations_no_toggle);
    RUN_TEST(test_TC_G9_002_period_callback_allows_null_argument_and_no_hal_calls);

    return UNITY_END();
}
