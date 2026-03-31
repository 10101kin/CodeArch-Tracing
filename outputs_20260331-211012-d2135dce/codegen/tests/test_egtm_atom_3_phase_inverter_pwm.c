#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback implemented in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                  (1e-4f)
#define UT_NUM_CHANNELS                   (3)
#define UT_PWM_FREQ_HZ                    (20000.0f)
#define UT_INIT_DUTY_U_PERCENT            (25.0f)
#define UT_INIT_DUTY_V_PERCENT            (50.0f)
#define UT_INIT_DUTY_W_PERCENT            (75.0f)
#define UT_STEP_PERCENT                   (10.0f)
#define UT_DT_RISING_US                   (1.0f)
#define UT_DT_FALLING_US                  (1.0f)
#define UT_ISR_PRIORITY                   (20)
#define UT_CMU_TARGET_FREQ_HZ             (100000000u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ========================================= */
/* GROUP 1 - initEgtmAtom3phInv: init/enable guard */
/* ========================================= */
void test_TC_G1_001_init_enables_and_configures_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* eGTM disabled → enable path taken */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - target API usage and clock setup on disabled path */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(),       "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(),          "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK frequency must be set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 frequency must be set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),                "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(),        "LED GPIO must be configured as push-pull output");
}

void test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;  /* eGTM already enabled → skip enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - still calls init APIs but skips enable/CMU */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(),       "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(),          "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK frequency set must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 frequency set must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU enable clocks must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),                "IfxEgtm_Pwm_init must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(),        "LED GPIO must be configured as push-pull output");
}

/* ========================================= */
/* GROUP 2 - initEgtmAtom3phInv: configuration values */
/* ========================================= */
void test_TC_G2_001_init_sets_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - verify config captured at init() time */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of PWM logical channels must be 3 (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_calls_init_and_initConfig_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - call counts */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),       "init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update should occur during init");
}

/* ========================================= */
/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (post-init behavior) */
/* ========================================= */
void test_TC_G3_001_first_update_increments_duty_percentages(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - percent units and single bulk update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be issued once as a bulk call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 35% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 60% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should be 85% after one update");
}

void test_TC_G3_002_three_updates_wrap_W_channel_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - perform three updates */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* After 3 updates: U=55, V=80, W wraps: 75->85->95->10 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10% after three updates");
}

/* ========================================= */
/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior */
/* ========================================= */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/* ========================================= */
/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
/* ========================================= */
void test_TC_G5_001_update_calls_hal_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update must result in one HAL call");
}

void test_TC_G5_002_update_multiple_calls_increment_hal_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update call count must equal number of updates");
}

/* ========================================= */
/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
/* ========================================= */
void test_TC_G6_001_three_updates_match_expected_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U = 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V = 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W wraps to 10% after three updates");
}

void test_TC_G6_002_five_updates_independent_wraps(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 5 updates: U=75, V wraps at 5th to 10, W wraps at 3rd then 30 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update (5x)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U = 75% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V wraps to 10% after five updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W = 30% after five updates (wrapped at 3rd)");
}

/* ========================================= */
/* GROUP 7 - interruptEgtmAtom: init/enable guard semantics */
/* ========================================= */
void test_TC_G7_001_isr_can_be_called_before_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR should toggle even if init was not called");
}

void test_TC_G7_002_isr_does_not_invoke_pwm_hal(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update API");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_init_getCallCount(), "ISR must not call PWM init API");
}

/* ========================================= */
/* GROUP 8 - interruptEgtmAtom: ISR behavior */
/* ========================================= */
void test_TC_G8_001_isr_toggle_count_matches_getter(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert - prefer single counter; also verify getter matches underlying counter */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "Underlying toggle counter must be 2 after two ISR calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_IfxPort_togglePin_getCallCount(), "Mock getter should report two toggles as well");
}

void test_TC_G8_002_isr_three_calls_results_in_three_toggles(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "Three ISR calls must produce three toggles");
}

/* ========================================= */
/* GROUP 9 - IfxEgtm_periodEventFunction: configuration values (no side effects) */
/* ========================================= */
void test_TC_G9_001_period_callback_has_no_side_effect_on_toggle(void)
{
    /* Arrange */
    /* Ensure some known baseline of toggle count (zero) */

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM update API");
}

/* ========================================= */
/* GROUP 10 - IfxEgtm_periodEventFunction: ISR/interrupt behavior (empty) */
/* ========================================= */
void test_TC_G10_001_period_callback_multiple_calls_no_toggle_change(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Multiple period callbacks must not change toggle count");
}

void test_TC_G10_002_period_callback_does_not_change_state_after_isr(void)
{
    /* Arrange */
    interruptEgtmAtom(); /* toggle once */

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "Period callback must not affect ISR-driven toggle state");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_and_configures_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_num_channels);
    RUN_TEST(test_TC_G2_002_init_calls_init_and_initConfig_once);

    RUN_TEST(test_TC_G3_001_first_update_increments_duty_percentages);
    RUN_TEST(test_TC_G3_002_three_updates_wrap_W_channel_only);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);

    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_call);
    RUN_TEST(test_TC_G5_002_update_multiple_calls_increment_hal_count);

    RUN_TEST(test_TC_G6_001_three_updates_match_expected_values);
    RUN_TEST(test_TC_G6_002_five_updates_independent_wraps);

    RUN_TEST(test_TC_G7_001_isr_can_be_called_before_init);
    RUN_TEST(test_TC_G7_002_isr_does_not_invoke_pwm_hal);

    RUN_TEST(test_TC_G8_001_isr_toggle_count_matches_getter);
    RUN_TEST(test_TC_G8_002_isr_three_calls_results_in_three_toggles);

    RUN_TEST(test_TC_G9_001_period_callback_has_no_side_effect_on_toggle);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_call_pwm_update);

    RUN_TEST(test_TC_G10_001_period_callback_multiple_calls_no_toggle_change);
    RUN_TEST(test_TC_G10_002_period_callback_does_not_change_state_after_isr);

    return UNITY_END();
}
