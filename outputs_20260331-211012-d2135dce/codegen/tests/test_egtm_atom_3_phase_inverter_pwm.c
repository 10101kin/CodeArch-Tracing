#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for functions not in the header (ISR and period callback) */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                  (1e-4f)
#define UT_NUM_CHANNELS                   (3)
#define UT_PWM_FREQUENCY_HZ               (20000)
#define UT_INIT_DUTY_U_PERCENT            (25.0f)
#define UT_INIT_DUTY_V_PERCENT            (50.0f)
#define UT_INIT_DUTY_W_PERCENT            (75.0f)
#define UT_STEP_PERCENT                   (10.0f)
#define UT_DT_RISING_US                   (1.0f)
#define UT_DT_FALLING_US                  (1.0f)
#define UT_ISR_PRIORITY                   (20)
#define UT_CMU_TARGET_FREQ_HZ             (100000000)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ========================= GROUP 1 - initEgtmAtom3phInv: initialization / enable guard ========================= */
void test_TC_G1_001_init_enables_module_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* Force not-enabled path */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "isEnabled must be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),         "enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU:getModuleFrequency must be called in disabled path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),    "CMU:setGclkFrequency must be called in disabled path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),     "CMU:setClkFrequency must be called in disabled path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),        "CMU:enableClocks must be called in disabled path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),        "Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once as push-pull output");
}

void test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* Force already-enabled path */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "isEnabled must be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),         "enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU:getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),    "CMU:setGclkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),     "CMU:setClkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),        "CMU:enableClocks must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),        "Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once as push-pull output");
}

/* ========================= GROUP 2 - initEgtmAtom3phInv: configuration values ========================= */
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* simplify path */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert application-set values via init() spies (not initConfig defaults) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE((float)UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_post_init_first_update_applies_expected_initial_duties_plus_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: First update from initial duties [25,50,75] with step 10 => [35,60,85] */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U first update should be 35% ");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V first update should be 60% ");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W first update should be 85% ");
}

/* ========================= GROUP 3 - initEgtmAtom3phInv: runtime update logic ========================= */
void test_TC_G3_001_update_increments_each_channel_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update should call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should become 35% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should become 60% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should become 85% after one update");
}

void test_TC_G3_002_update_wraps_each_channel_independently_after_multiple_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 9 updates */
    for (int i = 0; i < 9; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 9 updates: U=10, V=50, W=70 (wraps occur independently) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(9, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should wrap to 10% after 9 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should be 50% after 9 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should be 70% after 9 updates");
}

/* ========================= GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior ========================= */
void test_TC_G4_001_isr_single_call_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_isr_multiple_calls_accumulate_toggle_count(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxPort_togglePin_getCallCount(), "ISR toggles must accumulate across calls");
}

/* ========================= GROUP 5 - updateEgtmAtom3phInvDuty: configuration values ========================= */
void test_TC_G5_001_update_call_count_is_one_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "updateChannelsDutyImmediate must be called once per update");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of channels must remain 3");
}

void test_TC_G5_002_update_twice_accumulates_duties_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: two updates from [25,50,75] => [45,70,95] */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates should yield two HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 45% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should be 70% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should be 95% after two updates");
}

/* ========================= GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic ========================= */
void test_TC_G6_001_update_wrap_behavior_after_5_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: five updates. U: 75, V: 10 (wrap), W: 30 */
    for (int i = 0; i < 5; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates should yield five HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 75% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should wrap to 10% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should be 30% after five updates");
}

void test_TC_G6_002_update_wrap_behavior_after_3_updates_affects_phase_w(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: three updates. W should wrap to 10%. U: 55, V: 80 */
    for (int i = 0; i < 3; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three updates should yield three HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V should be 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W should wrap to 10% after three updates");
}

/* ========================= GROUP 7 - interruptEgtmAtom: initialization / enable guard ========================= */
void test_TC_G7_001_isr_can_toggle_without_prior_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED even without prior init");
}

void test_TC_G7_002_isr_does_not_invoke_pwm_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update");
}

/* ========================= GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior ========================= */
void test_TC_G8_001_isr_two_calls_increment_toggle_to_two(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxPort_togglePin_getCallCount(), "Two ISR invocations should toggle LED twice");
}

void test_TC_G8_002_isr_three_calls_increment_toggle_to_three(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxPort_togglePin_getCallCount(), "Three ISR invocations should toggle LED three times");
}

/* ========================= GROUP 9 - IfxEgtm_periodEventFunction: configuration values ========================= */
void test_TC_G9_001_period_callback_has_no_side_effects_on_port_toggle(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_has_no_side_effects_on_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM update");
}

/* ========================= GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior ========================= */
void test_TC_G10_001_period_callback_multiple_calls_still_no_effect(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Multiple period callbacks must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not call PWM update");
}

void test_TC_G10_002_period_callback_after_init_still_no_effect(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback after init must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback after init must not call PWM update");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_module_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_post_init_first_update_applies_expected_initial_duties_plus_step);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_update_increments_each_channel_below_boundary);
    RUN_TEST(test_TC_G3_002_update_wraps_each_channel_independently_after_multiple_updates);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_single_call_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_multiple_calls_accumulate_toggle_count);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_call_count_is_one_per_invocation);
    RUN_TEST(test_TC_G5_002_update_twice_accumulates_duties_without_wrap);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_update_wrap_behavior_after_5_updates);
    RUN_TEST(test_TC_G6_002_update_wrap_behavior_after_3_updates_affects_phase_w);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_isr_can_toggle_without_prior_init);
    RUN_TEST(test_TC_G7_002_isr_does_not_invoke_pwm_update);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_isr_two_calls_increment_toggle_to_two);
    RUN_TEST(test_TC_G8_002_isr_three_calls_increment_toggle_to_three);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_period_callback_has_no_side_effects_on_port_toggle);
    RUN_TEST(test_TC_G9_002_period_callback_has_no_side_effects_on_pwm_update);

    /* GROUP 10 */
    RUN_TEST(test_TC_G10_001_period_callback_multiple_calls_still_no_effect);
    RUN_TEST(test_TC_G10_002_period_callback_after_init_still_no_effect);

    return UNITY_END();
}
