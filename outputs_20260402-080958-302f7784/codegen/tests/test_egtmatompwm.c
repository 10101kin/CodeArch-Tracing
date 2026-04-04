#include "unity.h"
#include "mock_egtmatompwm.h"
#include "egtmatompwm.h"

/* Extern declarations for functions that may not be in the header */
extern void resultISR(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (0.01f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_DEADTIME_RISE_US              (1.0f)
#define UT_DEADTIME_FALL_US              (1.0f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)

void setUp(void)   { mock_egtmatompwm_reset(); }
void tearDown(void) {}

/* ===================================================================== */
/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard           */
/* ===================================================================== */
void test_TC_G1_001_init_when_EGTM_already_enabled_does_not_enable_module(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "If EGTM is already enabled, IfxEgtm_enable must NOT be called");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must NOT be enabled when EGTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
}

void test_TC_G1_002_init_when_EGTM_disabled_enables_module_and_clocks(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 100000000u; /* 100 MHz mock */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "If EGTM was disabled, IfxEgtm_enable must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should be queried once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled once when EGTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
}

/* ===================================================================== */
/* GROUP 2 - initEgtmAtom3phInv: configuration values                    */
/* ===================================================================== */
void test_TC_G2_001_init_sets_numChannels_to_3(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxEgtm_Pwm_init_lastNumChannels, "numChannels must be 3 (one per complementary phase pair)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
}

void test_TC_G2_002_init_sets_pwm_frequency_to_20kHz(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

/* ===================================================================== */
/* GROUP 3 - initEgtmAtom3phInv: runtime update logic                    */
/* ===================================================================== */
void test_TC_G3_001_after_init_first_update_increments_duties_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once after one update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step");
}

void test_TC_G3_002_after_many_updates_wraps_high_phase_W_to_step_and_preserves_others(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 2500 updates → U: 25→50, V: 50→75, W wraps to step (0.01) */
    for (unsigned int i = 0; i < 2500u; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2500u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per update invocation (2500 times)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U expected 50.00% after 2500 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V expected 75.00% after 2500 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to step value after crossing 100%");
}

/* ===================================================================== */
/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (init-time HW) */
/* ===================================================================== */
void test_TC_G4_001_init_configures_LED_pin_output(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as push-pull output exactly once during init");
}

void test_TC_G4_002_init_configures_adc_trigger_mapping_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger mux must be configured exactly once during init");
}

/* ===================================================================== */
/* GROUP 5 - updateEgtmAtomDuty: initialization / enable guard           */
/* ===================================================================== */
void test_TC_G5_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* EGTM state for init path */
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update should call HAL once");
}

void test_TC_G5_002_update_multiple_calls_accumulate_hal_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* Exercise enable path during init */
    initEgtmAtom3phInv();

    /* Act */
    for (unsigned int i = 0; i < 10u; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per invocation (10 times)");
}

/* ===================================================================== */
/* GROUP 6 - updateEgtmAtomDuty: configuration values                    */
/* ===================================================================== */
void test_TC_G6_001_first_update_results_match_initial_plus_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 25.01% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 50.01% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 75.01% after first update");
}

void test_TC_G6_002_after_100_updates_results_match_expected(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (unsigned int i = 0; i < 100u; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 26.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 26.00% after 100 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 51.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 51.00% after 100 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 76.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 76.00% after 100 updates");
}

/* ===================================================================== */
/* GROUP 7 - updateEgtmAtomDuty: runtime update logic                    */
/* ===================================================================== */
void test_TC_G7_001_wrap_each_channel_independently_after_large_steps(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 7500 updates → U wraps to 0.01, V=25.01, W=75.01 (see wrap semantics) */
    for (unsigned int i = 0; i < 7500u; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U must wrap to step value after crossing 100%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 25.01f,        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V expected 25.01% after 7500 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.01f,        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W expected 75.01% after 7500 updates");
}

void test_TC_G7_002_no_wrap_when_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 10 updates keeps all duties well below 100% */
    for (unsigned int i = 0; i < 10u; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + 10.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must increase by 10 steps without wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + 10.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must increase by 10 steps without wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + 10.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must increase by 10 steps without wrap");
}

/* ===================================================================== */
/* GROUP 8 - resultISR: initialization / enable guard                    */
/* ===================================================================== */
void test_TC_G8_001_resultISR_toggles_led_once(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "resultISR must toggle LED exactly once");
}

void test_TC_G8_002_resultISR_multiple_calls_accumulate_toggle_count(void)
{
    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "resultISR toggles LED on every invocation (2 times)");
}

/* ===================================================================== */
/* GROUP 9 - resultISR: configuration values                             */
/* ===================================================================== */
void test_TC_G9_001_resultISR_does_not_update_pwm_duty(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount, "resultISR must not call PWM duty update");
}

void test_TC_G9_002_resultISR_does_not_install_or_enable_cpu_interrupts(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxCpu_Irq_installInterruptHandler_getCallCount(), "resultISR should not install interrupt handler");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxCpu_enableInterrupts_getCallCount(), "resultISR should not enable CPU interrupts");
}

/* ===================================================================== */
/* GROUP 10 - resultISR: ISR / interrupt behavior                         */
/* ===================================================================== */
void test_TC_G10_001_resultISR_toggle_accumulates_across_calls(void)
{
    /* Act */
    resultISR();
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "LED toggle must accumulate across ISR calls (3 toggles)");
}

void test_TC_G10_002_resultISR_does_not_configure_adc_trigger_mux(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ISR must not call IfxEgtm_Trigger_trigToAdc");
}

/* ===================================================================== */
/* GROUP 11 - IfxEgtm_periodEventFunction: configuration values           */
/* ===================================================================== */
void test_TC_G11_001_period_callback_has_no_side_effects_on_toggle(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G11_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount, "Period callback must not update PWM duties");
}

/* ===================================================================== */
/* GROUP 12 - IfxEgtm_periodEventFunction: ISR / interrupt behavior       */
/* ===================================================================== */
void test_TC_G12_001_period_callback_multiple_calls_still_no_side_effects(void)
{
    /* Act */
    for (unsigned int i = 0; i < 5u; ++i)
    {
        IfxEgtm_periodEventFunction((void *)0);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must have no side effects even after multiple calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "Period callback must not configure ADC trigger");
}

void test_TC_G12_002_period_callback_does_not_enable_module_or_clocks(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void *)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "Period callback must not enable EGTM module");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Period callback must not enable CMU clocks");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_when_EGTM_already_enabled_does_not_enable_module);
    RUN_TEST(test_TC_G1_002_init_when_EGTM_disabled_enables_module_and_clocks);

    RUN_TEST(test_TC_G2_001_init_sets_numChannels_to_3);
    RUN_TEST(test_TC_G2_002_init_sets_pwm_frequency_to_20kHz);

    RUN_TEST(test_TC_G3_001_after_init_first_update_increments_duties_by_step);
    RUN_TEST(test_TC_G3_002_after_many_updates_wraps_high_phase_W_to_step_and_preserves_others);

    RUN_TEST(test_TC_G4_001_init_configures_LED_pin_output);
    RUN_TEST(test_TC_G4_002_init_configures_adc_trigger_mapping_once);

    RUN_TEST(test_TC_G5_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_multiple_calls_accumulate_hal_count);

    RUN_TEST(test_TC_G6_001_first_update_results_match_initial_plus_step);
    RUN_TEST(test_TC_G6_002_after_100_updates_results_match_expected);

    RUN_TEST(test_TC_G7_001_wrap_each_channel_independently_after_large_steps);
    RUN_TEST(test_TC_G7_002_no_wrap_when_below_boundary);

    RUN_TEST(test_TC_G8_001_resultISR_toggles_led_once);
    RUN_TEST(test_TC_G8_002_resultISR_multiple_calls_accumulate_toggle_count);

    RUN_TEST(test_TC_G9_001_resultISR_does_not_update_pwm_duty);
    RUN_TEST(test_TC_G9_002_resultISR_does_not_install_or_enable_cpu_interrupts);

    RUN_TEST(test_TC_G10_001_resultISR_toggle_accumulates_across_calls);
    RUN_TEST(test_TC_G10_002_resultISR_does_not_configure_adc_trigger_mux);

    RUN_TEST(test_TC_G11_001_period_callback_has_no_side_effects_on_toggle);
    RUN_TEST(test_TC_G11_002_period_callback_does_not_call_pwm_update);

    RUN_TEST(test_TC_G12_001_period_callback_multiple_calls_still_no_side_effects);
    RUN_TEST(test_TC_G12_002_period_callback_does_not_enable_module_or_clocks);

    return UNITY_END();
}
