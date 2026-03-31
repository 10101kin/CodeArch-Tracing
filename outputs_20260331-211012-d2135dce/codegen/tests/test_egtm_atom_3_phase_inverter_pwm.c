#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback implemented in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_CMU_TARGET_FREQ_HZ            (100000000U)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 */
void test_TC_01_001_init_calls_unified_api_and_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* eGTM disabled path */
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - TC4xx API usage and CMU enable-sequence executed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once in disabled branch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must be configured once in disabled branch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 frequency must be configured once in disabled branch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled once in disabled branch");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as push-pull output exactly once");
}

void test_TC_01_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;  /* eGTM already enabled */
    mock_IfxEgtm_Cmu_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - EGTM enable/CMU setup skipped, but PWM init sequence still executed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must NOT be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 frequency must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
}

/*
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 */
void test_TC_02_001_init_config_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - Verify PWM frequency and number of channels via init() spy */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 (complementary handled internally)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_02_002_led_configured_push_pull_output(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - LED configuration via IfxPort API once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as push-pull output exactly once");
}

/*
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (duty update behavior after init)
 */
void test_TC_03_001_update_increments_duty_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - One bulk HAL call, duties in percent: 35/60/85 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 35% after one step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 60% after one step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should be 85% after one step");
}

void test_TC_03_002_update_wraps_w_channel_at_95_to_10_after_next_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - 3 updates: W goes 75->85->95->wrap to 10 */
    updateEgtmAtom3phInvDuty(); /* -> 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* -> 45,70,95 */
    updateEgtmAtom3phInvDuty(); /* -> 55,80,10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called once per update (3 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 55% after three steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 80% after three steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should wrap to 10% after three steps");
}

/*
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior
 */
void test_TC_04_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t before = mock_togglePin_getCallCount();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_togglePin_getCallCount(), "ISR must toggle LED exactly once");
}

void test_TC_04_002_isr_toggles_led_cumulatively(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_togglePin_getCallCount(), "Two ISR invocations must toggle LED twice");
}

/*
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 */
void test_TC_05_001_update_calls_hal_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update must result in one HAL call (3 total)");
}

void test_TC_05_002_update_applies_three_channel_array(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - Verify three logical channels map to U/V/W duties */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Index 0 must map to Phase U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Index 1 must map to Phase V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Index 2 must map to Phase W");
}

/*
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 */
void test_TC_06_001_wrap_independent_channels_V_wraps_at_step5(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - 5 updates: expected U=75, V=10 (wrap at step5), W=30 */
    updateEgtmAtom3phInvDuty(); /* 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* 45,70,95 */
    updateEgtmAtom3phInvDuty(); /* 55,80,10 */
    updateEgtmAtom3phInvDuty(); /* 65,90,20 */
    updateEgtmAtom3phInvDuty(); /* 75,10,30 (V wraps) */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 5 steps must be 75%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap to 10% at step 5");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after 5 steps must be 30%");
}

void test_TC_06_002_wrap_independent_channels_U_wraps_at_step8(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - 8 updates: expected [10,40,60] with U wrapping now */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */
    updateEgtmAtom3phInvDuty(); /* 3 */
    updateEgtmAtom3phInvDuty(); /* 4 */
    updateEgtmAtom3phInvDuty(); /* 5 */
    updateEgtmAtom3phInvDuty(); /* 6 */
    updateEgtmAtom3phInvDuty(); /* 7 */
    updateEgtmAtom3phInvDuty(); /* 8 -> U wraps */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must wrap to 10% at step 8");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 8 steps must be 40%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after 8 steps must be 60%");
}

/*
 * GROUP 7 - interruptEgtmAtom: initialization / enable guard
 */
void test_TC_07_001_isr_toggle_with_egtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t before = mock_togglePin_getCallCount();

    /* Act */
    interruptEgtmAtom();

    /* Assert - ISR toggles LED, no CMU enable sequence in already-enabled case */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_togglePin_getCallCount(), "ISR must toggle LED once when module already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must not be re-enabled in already-enabled case");
}

void test_TC_07_002_isr_toggle_with_egtm_disabled_enables_clocks_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - EGTM/CMU enable path taken */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled once when disabled");

    /* ISR behavior */
    uint32_t before = mock_togglePin_getCallCount();
    interruptEgtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_togglePin_getCallCount(), "ISR must toggle LED after initialization");
}

/*
 * GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior
 */
void test_TC_08_001_multiple_isr_calls_accumulate(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_togglePin_getCallCount(), "Three ISR invocations must toggle LED three times");
}

void test_TC_08_002_isr_does_not_call_pwm_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert - ISR should not touch PWM duty update API */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update API");
}

/*
 * GROUP 9 - IfxEgtm_periodEventFunction: configuration values
 */
void test_TC_09_001_period_callback_is_noop_does_not_toggle(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t before = mock_togglePin_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert - No toggle occurs in period callback */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_getCallCount(), "Period-event callback must be a no-op (no LED toggle)");
}

void test_TC_09_002_period_callback_accepts_nonnull_data_and_is_noop(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t before = mock_togglePin_getCallCount();

    /* Act */
    int dummy = 0x1234;
    IfxEgtm_periodEventFunction(&dummy);

    /* Assert - Still no toggle */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_getCallCount(), "Period-event callback with non-NULL data must be a no-op");
}

/*
 * GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior
 */
void test_TC_10_001_period_callback_multiple_calls_no_toggle(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t before = mock_togglePin_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_getCallCount(), "Multiple period-event callbacks must not toggle LED");
}

void test_TC_10_002_period_callback_does_not_invoke_pwm_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period-event callback must not call PWM duty update API");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_01_001_init_calls_unified_api_and_cmu_when_disabled);
    RUN_TEST(test_TC_01_002_init_skips_enable_when_already_enabled);

    RUN_TEST(test_TC_02_001_init_config_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_02_002_led_configured_push_pull_output);

    RUN_TEST(test_TC_03_001_update_increments_duty_below_boundary);
    RUN_TEST(test_TC_03_002_update_wraps_w_channel_at_95_to_10_after_next_step);

    RUN_TEST(test_TC_04_001_isr_toggles_led_once);
    RUN_TEST(test_TC_04_002_isr_toggles_led_cumulatively);

    RUN_TEST(test_TC_05_001_update_calls_hal_once_per_call);
    RUN_TEST(test_TC_05_002_update_applies_three_channel_array);

    RUN_TEST(test_TC_06_001_wrap_independent_channels_V_wraps_at_step5);
    RUN_TEST(test_TC_06_002_wrap_independent_channels_U_wraps_at_step8);

    RUN_TEST(test_TC_07_001_isr_toggle_with_egtm_already_enabled);
    RUN_TEST(test_TC_07_002_isr_toggle_with_egtm_disabled_enables_clocks_once);

    RUN_TEST(test_TC_08_001_multiple_isr_calls_accumulate);
    RUN_TEST(test_TC_08_002_isr_does_not_call_pwm_update);

    RUN_TEST(test_TC_09_001_period_callback_is_noop_does_not_toggle);
    RUN_TEST(test_TC_09_002_period_callback_accepts_nonnull_data_and_is_noop);

    RUN_TEST(test_TC_10_001_period_callback_multiple_calls_no_toggle);
    RUN_TEST(test_TC_10_002_period_callback_does_not_invoke_pwm_update);

    return UNITY_END();
}
