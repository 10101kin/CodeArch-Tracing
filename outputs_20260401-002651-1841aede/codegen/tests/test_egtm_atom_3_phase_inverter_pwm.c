#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for ISR and period callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_DEADTIME_RISE_US              (1.0f)
#define UT_DEADTIME_FALL_US              (1.0f)
#define UT_LED_PIN_Px_y                  "P03.9"
#define UT_EGTM_CLUSTER                  (0U)
#define UT_SYS_FREQ_HZ                   (300000000U)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* =============================
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 * ============================= */
void test_TC_1_001_init_enables_module_and_configures_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* peripheral disabled */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_SYS_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "TC4xx: IfxEgtm_isEnabled must be checked once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "TC4xx: IfxEgtm_enable must be called once when EGTM is disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "TC4xx: Module frequency must be read once for CMU setup");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "TC4xx: GCLK frequency configured exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),  "TC4xx: Sub-clock (CLK0) frequency configured exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),     "TC4xx: CMU FXCLK0 and DTM clock 0 enabled once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "TC4xx: IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "TC4xx: IfxEgtm_Pwm_init must be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED must be configured as output once after PWM init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update should occur during init");
}

void test_TC_1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* peripheral already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),    "IfxEgtm_enable must not be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU configuration must be inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),    "GCLK config skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),     "CLK0 config skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),        "Clock enable skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig still called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "init still called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED output configured once");
}

/* =============================
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 * ============================= */
void test_TC_2_001_init_sets_pwm_frequency_and_num_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_2_002_init_calls_initConfig_then_init_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "init must be called exactly once");
}

/* =============================
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (post-init behavior)
 * ============================= */
void test_TC_3_001_after_init_first_update_steps_duty_from_initial_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - expect +10% on each channel */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should step to 35.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should step to 60.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should step to 85.0%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update");
}

void test_TC_3_002_after_three_updates_wrap_applies_to_phase_W_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - perform 3 updates: W will wrap on 3rd (75->85->95->10) */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + 3.0f * UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55.0% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + 3.0f * UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80.0% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f,                                              mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10.0% on 3rd step");
}

/* =============================
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior
 * ============================= */
void test_TC_4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

/* =============================
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 * ============================= */
void test_TC_5_001_update_uses_three_channel_array_and_applies_percent_units(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - values in percent after one step */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 35.0% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 60.0% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 85.0% (percent units)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Driver configured with exactly 3 logical channels");
}

void test_TC_5_002_update_does_not_change_configured_pwm_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency remains 20 kHz after updates");
}

/* =============================
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 * ============================= */
void test_TC_6_001_multiple_updates_wrap_each_channel_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - perform 8 updates to force wrap on U and advance V/W accordingly */
    for (unsigned i = 0; i < 8U; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Expected after 8 steps: U=10, V=40, W=60 (per step-wise wrap rule) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must wrap to 10.0% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 40.0% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 60.0% after 8 steps");
}

void test_TC_6_002_single_update_invokes_hal_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL call per updateEgtmAtom3phInvDuty invocation");
}

/* =============================
 * GROUP 7 - interruptEgtmAtom: ISR / interrupt behavior
 * ============================= */
void test_TC_7_001_interruptEgtmAtom_toggles_led_once_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "interruptEgtmAtom must toggle LED once");
}

void test_TC_7_002_interruptEgtmAtom_toggles_led_five_times(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (unsigned i = 0; i < 5U; ++i)
    {
        interruptEgtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "interruptEgtmAtom must toggle LED 5 times across 5 calls");
}

/* =============================
 * GROUP 8 - IfxEgtm_periodEventFunction: configuration values
 * ============================= */
void test_TC_8_001_period_callback_is_callable_and_side_effect_free(void)
{
    /* Arrange */
    /* No init needed; ensure no toggles before call */

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert - must not toggle LED */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period event callback must be empty (no LED toggle)");
}

void test_TC_8_002_period_callback_does_not_trigger_duty_update(void)
{
    /* Arrange */
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert - no duty update HAL call */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period event callback must not update duties");
}

/* =============================
 * GROUP 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (must remain empty)
 * ============================= */
void test_TC_9_001_period_callback_multiple_calls_do_not_toggle_led(void)
{
    /* Arrange */
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period event callback must not toggle LED even after multiple calls");
}

void test_TC_9_002_period_callback_does_not_install_interrupt_handler(void)
{
    /* Arrange */
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxCpu_Irq_installInterruptHandler_getCallCount(), "Period event callback must not install IRQ handlers");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_1_001_init_enables_module_and_configures_clocks_when_disabled);
    RUN_TEST(test_TC_1_002_init_skips_enable_and_cmu_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_2_001_init_sets_pwm_frequency_and_num_channels);
    RUN_TEST(test_TC_2_002_init_calls_initConfig_then_init_once_each);

    /* Group 3 */
    RUN_TEST(test_TC_3_001_after_init_first_update_steps_duty_from_initial_values);
    RUN_TEST(test_TC_3_002_after_three_updates_wrap_applies_to_phase_W_only);

    /* Group 4 */
    RUN_TEST(test_TC_4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_4_002_isr_toggle_accumulates_over_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_5_001_update_uses_three_channel_array_and_applies_percent_units);
    RUN_TEST(test_TC_5_002_update_does_not_change_configured_pwm_frequency);

    /* Group 6 */
    RUN_TEST(test_TC_6_001_multiple_updates_wrap_each_channel_independently);
    RUN_TEST(test_TC_6_002_single_update_invokes_hal_once);

    /* Group 7 */
    RUN_TEST(test_TC_7_001_interruptEgtmAtom_toggles_led_once_after_init);
    RUN_TEST(test_TC_7_002_interruptEgtmAtom_toggles_led_five_times);

    /* Group 8 */
    RUN_TEST(test_TC_8_001_period_callback_is_callable_and_side_effect_free);
    RUN_TEST(test_TC_8_002_period_callback_does_not_trigger_duty_update);

    /* Group 9 */
    RUN_TEST(test_TC_9_001_period_callback_multiple_calls_do_not_toggle_led);
    RUN_TEST(test_TC_9_002_period_callback_does_not_install_interrupt_handler);

    return UNITY_END();
}
