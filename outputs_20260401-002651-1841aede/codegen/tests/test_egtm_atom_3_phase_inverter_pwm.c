#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for ISR and driver callback implemented in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_PWM_FREQUENCY                 (20000.0f)
#define UT_DT_RISING_US                  (1.0f)
#define UT_DT_FALLING_US                 (1.0f)
#define UT_ISR_PRIORITY                  (20)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 **********************/
void test_TC_G1_001_init_enables_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* EGTM disabled initially */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when EGTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be queried when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must be configured when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency must be configured when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Required EGTM clocks must be enabled when EGTM is disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output exactly once after PWM init");
}

void test_TC_G1_002_init_skips_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* EGTM enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must not be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must not be queried when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK configuration must be skipped when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK configuration must be skipped when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Clock enabling must be skipped when EGTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output exactly once after PWM init");
}

/**********************
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 **********************/
void test_TC_G2_001_init_sets_frequency_and_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of PWM channels must be 3 for U,V,W");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_sets_deadtime_to_1us_for_all_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[0], "DT rising[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[1], "DT rising[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[2], "DT rising[2] must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[0], "DT falling[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[1], "DT falling[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[2], "DT falling[2] must be 1.0 us");
}

/**********************
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (duty stepping semantics)
 **********************/
void test_TC_G3_001_update_increments_duties_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should increment by step");
}

void test_TC_G3_002_update_wraps_w_channel_at_or_above_100_then_sets_to_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 3 updates → W wraps: 75→85→95→10 */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */
    updateEgtmAtom3phInvDuty(); /* 3 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update (3 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3 steps should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3 steps should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must wrap to step value 10% after crossing 100%");
}

/**********************
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (LED toggle in ISR)
 **********************/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_isr_toggles_led_multiple_times_accumulatively(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggles must accumulate across multiple calls");
}

/**********************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values (post-init invariants)
 **********************/
void test_TC_G5_001_update_preserves_channel_count_and_frequency_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Channel count must remain 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL update expected after one call");
}

void test_TC_G5_002_update_applies_expected_duties_after_two_steps(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* step 1 */
    updateEgtmAtom3phInvDuty(); /* step 2 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two HAL updates expected after two calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 2 steps should be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 2 steps should be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 2 steps should be 95%");
}

/**********************
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic (independent wraps)
 **********************/
void test_TC_G6_001_wraps_independently_across_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 8 updates to exercise independent wraps */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 steps: expected [10, 40, 60] */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight HAL updates expected after eight calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should wrap to 10% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should be 40% after 8 steps (one wrap earlier)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should be 60% after 8 steps (wrapped earlier)");
}

void test_TC_G6_002_hal_called_once_per_update_not_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called exactly once per update() invocation");
}

/**********************
 * GROUP 7 - interruptEgtmAtom: ISR / interrupt behavior
 **********************/
void test_TC_G7_001_isr_does_not_invoke_pwm_update_api(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update API");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once");
}

void test_TC_G7_002_isr_multiple_invocations_cumulative_toggle(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "Two ISR invocations should toggle LED twice");
}

/**********************
 * GROUP 8 - IfxEgtm_periodEventFunction: configuration values (no side effects)
 **********************/
void test_TC_G8_001_period_callback_is_noop_single_call(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G8_002_period_callback_is_noop_multiple_calls_no_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED on any call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not trigger PWM duty update");
}

/**********************
 * GROUP 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (remains empty)
 **********************/
void test_TC_G9_001_period_callback_does_not_toggle_led_even_between_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    updateEgtmAtom3phInvDuty(); /* one update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Precondition: one duty update executed");

    /* Act: call the empty period callback */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert: no LED toggle and update call count unchanged */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not change duty update call count");
}

void test_TC_G9_002_period_callback_accepts_null_pointer_and_has_no_side_effects(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "No LED toggle expected from period callback");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No PWM update expected from period callback");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_init_sets_deadtime_to_1us_for_all_channels);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_update_increments_duties_below_boundary);
    RUN_TEST(test_TC_G3_002_update_wraps_w_channel_at_or_above_100_then_sets_to_step);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_multiple_times_accumulatively);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_preserves_channel_count_and_frequency_values);
    RUN_TEST(test_TC_G5_002_update_applies_expected_duties_after_two_steps);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_wraps_independently_across_channels);
    RUN_TEST(test_TC_G6_002_hal_called_once_per_update_not_per_channel);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_isr_does_not_invoke_pwm_update_api);
    RUN_TEST(test_TC_G7_002_isr_multiple_invocations_cumulative_toggle);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_period_callback_is_noop_single_call);
    RUN_TEST(test_TC_G8_002_period_callback_is_noop_multiple_calls_no_pwm_update);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_period_callback_does_not_toggle_led_even_between_updates);
    RUN_TEST(test_TC_G9_002_period_callback_accepts_null_pointer_and_has_no_side_effects);

    return UNITY_END();
}
