#include "unity.h"
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* Extern declarations for ISR/callback symbols defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);
extern void resultISR(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_NUM_CHANNELS                  (3)
#define UT_ADC_TRIG_NUM_CHANNELS         (1)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)

void setUp(void)   { mock_egtm_atom_adc_tmadc_multiple_channels_reset(); }
void tearDown(void) {}

/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
void test_TC_01_001_init_enables_egtm_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* EGTM disabled -> expect enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000U; /* arbitrary module freq */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "EGTM enable guard must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "EGTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be queried once when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "EGTM clocks must be enabled");

    /* Two PWM instances initialized: 3-phase + ADC trigger */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called for each PWM instance");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_init_getCallCount(), "init must be called for each PWM instance");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "EGTM trigger to ADC must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_enableModule_getCallCount(), "ADC module must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_runModule_getCallCount(), "TMADC engine must be started once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Heartbeat GPIO must be configured as output");
}

void test_TC_01_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* EGTM already enabled -> skip enable/clock path */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "EGTM enable guard must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "EGTM must NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should not be queried when EGTM is already enabled (guarded path)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Clocks should not be enabled when EGTM is already enabled (guarded path)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must still be called twice (two instances)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_init_getCallCount(), "init must still be called twice (two instances)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "EGTM trigger to ADC must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_enableModule_getCallCount(), "ADC module must be enabled once regardless of EGTM guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_runModule_getCallCount(), "TMADC engine must be started once regardless of EGTM guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Heartbeat GPIO must be configured as output");
}

/* GROUP 2 - initEgtmAtom3phInv: configuration values */
void test_TC_02_001_init_sets_pwm_frequency_to_20kHz_on_last_instance(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* focus on config values, skip enable branch */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: last init() corresponds to ADC trigger instance but frequency should match base */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM init frequency must be 20 kHz");
}

void test_TC_02_002_init_uses_single_channel_for_adc_trigger_on_second_instance(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: last init() is the ADC trigger instance with 1 logical channel */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_ADC_TRIG_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "ADC trigger PWM must initialize with 1 channel");
}

/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (integration with update API) */
void test_TC_03_001_after_init_single_update_calls_hal_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 duties[3] = {35.0f, 45.0f, 55.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, duties[0], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must match request (percent)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, duties[1], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must match request (percent)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, duties[2], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must match request (percent)");
}

void test_TC_03_002_multiple_updates_increment_call_count_and_overwrite_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 d1[3] = {10.0f, 20.0f, 30.0f};
    float32 d2[3] = {60.0f, 70.0f, 80.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(d1);
    updateEgtmAtom3phInvDuty(d2);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per API invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, d2[0], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Last U duty must reflect most recent update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, d2[1], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Last V duty must reflect most recent update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, d2[2], mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Last W duty must reflect most recent update");
}

/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (integration) */
void test_TC_04_001_isr_toggle_once_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle heartbeat GPIO once");
}

void test_TC_04_002_isr_toggle_accumulates_multiple_calls_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must toggle heartbeat GPIO on each invocation");
}

/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
void test_TC_05_001_update_uses_three_channels_array_order(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 duties[3] = {UT_INIT_DUTY_U_PERCENT, UT_INIT_DUTY_V_PERCENT, UT_INIT_DUTY_W_PERCENT};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL call expected for duty update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Index 0 must map to Phase U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Index 1 must map to Phase V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Index 2 must map to Phase W");
}

void test_TC_05_002_update_allows_boundary_values_0_100_percent_and_fractional(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 duties[3] = {0.0f, 100.0f, 55.5f};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL call expected for boundary update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,   mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Boundary 0% must be accepted");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Boundary 100% must be accepted");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.5f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Fractional percent must be passed through");
}

/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
void test_TC_06_001_update_called_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 duties[3] = {12.0f, 34.0f, 56.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update call must invoke HAL once");
}

void test_TC_06_002_sequential_updates_call_hal_each_time(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();
    float32 d1[3] = {1.0f, 2.0f, 3.0f};
    float32 d2[3] = {4.0f, 5.0f, 6.0f};
    float32 d3[3] = {7.0f, 8.0f, 9.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(d1);
    updateEgtmAtom3phInvDuty(d2);
    updateEgtmAtom3phInvDuty(d3);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called for each sequential update");
}

/* GROUP 7 - interruptEgtmAtom: initialization / enable guard */
void test_TC_07_001_interrupt_toggles_pin_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle heartbeat GPIO once");
}

void test_TC_07_002_interrupt_multiple_invocations_accumulate(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "Toggle count must accumulate across ISR invocations");
}

/* GROUP 8 - interruptEgtmAtom: configuration values (negative side-effects) */
void test_TC_08_001_interrupt_does_not_call_pwm_update_or_adc_apis(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert: ensure unrelated HAL calls remain zero */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not update PWM duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "ISR must not poll ADC results");
}

void test_TC_08_002_interrupt_does_not_reconfigure_gpio_mode(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR must not reconfigure GPIO mode");
}

/* GROUP 9 - interruptEgtmAtom: ISR / interrupt behavior */
void test_TC_09_001_toggle_does_not_depend_on_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle regardless of init state");
}

void test_TC_09_002_toggle_accumulates_without_reset_within_test(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle must accumulate within the same test");
}

/* GROUP 10 - IfxEgtm_periodEventFunction: configuration values */
void test_TC_10_001_period_callback_has_no_side_effect_on_gpio_or_pwm(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle GPIO");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

void test_TC_10_002_period_callback_multiple_calls_still_no_side_effect(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callback calls must not toggle GPIO");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callback calls must not update PWM");
}

/* GROUP 11 - IfxEgtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_11_001_period_callback_does_not_toggle_gpio(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must remain empty: no GPIO toggle");
}

void test_TC_11_002_period_callback_idempotent_no_hal_calls(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "Period callback must not access ADC APIs");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not access PWM update API");
}

/* GROUP 12 - resultISR: initialization / enable guard */
void test_TC_12_001_resultISR_clears_flags_for_all_channels_when_available(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = TRUE;

    /* Act */
    resultISR();

    /* Assert: five channels CH0..CH4 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "ISR must check availability for 5 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "ISR must clear result flags for all available channels");
}

void test_TC_12_002_resultISR_skips_clearing_when_no_results_available(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "Availability must still be checked for 5 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "No flags must be cleared when no results are available");
}

/* GROUP 13 - resultISR: configuration values */
void test_TC_13_001_resultISR_checks_five_channels(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "ISR must iterate over 5 configured TMADC channels");
}

void test_TC_13_002_resultISR_clears_five_flags_when_all_available(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = TRUE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "ISR must clear 5 flags when all channels have results");
}

/* GROUP 14 - resultISR: ISR / interrupt behavior */
void test_TC_14_001_resultISR_accumulates_clear_calls_across_invocations(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = TRUE;

    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "Two ISR runs must perform 10 availability checks total");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "Two ISR runs must clear 10 flags total");
}

void test_TC_14_002_resultISR_handles_mixed_availability_across_invocations(void)
{
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = TRUE;

    /* Act */
    resultISR(); /* first: all available */

    /* Arrange (second run: none available) */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "Two ISR runs must perform 10 availability checks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "Only first run clears 5 flags; total must be 5");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_01_001_init_enables_egtm_and_clocks_when_disabled);
    RUN_TEST(test_TC_01_002_init_skips_enable_when_already_enabled);

    RUN_TEST(test_TC_02_001_init_sets_pwm_frequency_to_20kHz_on_last_instance);
    RUN_TEST(test_TC_02_002_init_uses_single_channel_for_adc_trigger_on_second_instance);

    RUN_TEST(test_TC_03_001_after_init_single_update_calls_hal_once);
    RUN_TEST(test_TC_03_002_multiple_updates_increment_call_count_and_overwrite_duties);

    RUN_TEST(test_TC_04_001_isr_toggle_once_after_init);
    RUN_TEST(test_TC_04_002_isr_toggle_accumulates_multiple_calls_after_init);

    RUN_TEST(test_TC_05_001_update_uses_three_channels_array_order);
    RUN_TEST(test_TC_05_002_update_allows_boundary_values_0_100_percent_and_fractional);

    RUN_TEST(test_TC_06_001_update_called_once_per_call);
    RUN_TEST(test_TC_06_002_sequential_updates_call_hal_each_time);

    RUN_TEST(test_TC_07_001_interrupt_toggles_pin_once);
    RUN_TEST(test_TC_07_002_interrupt_multiple_invocations_accumulate);

    RUN_TEST(test_TC_08_001_interrupt_does_not_call_pwm_update_or_adc_apis);
    RUN_TEST(test_TC_08_002_interrupt_does_not_reconfigure_gpio_mode);

    RUN_TEST(test_TC_09_001_toggle_does_not_depend_on_init);
    RUN_TEST(test_TC_09_002_toggle_accumulates_without_reset_within_test);

    RUN_TEST(test_TC_10_001_period_callback_has_no_side_effect_on_gpio_or_pwm);
    RUN_TEST(test_TC_10_002_period_callback_multiple_calls_still_no_side_effect);

    RUN_TEST(test_TC_11_001_period_callback_does_not_toggle_gpio);
    RUN_TEST(test_TC_11_002_period_callback_idempotent_no_hal_calls);

    RUN_TEST(test_TC_12_001_resultISR_clears_flags_for_all_channels_when_available);
    RUN_TEST(test_TC_12_002_resultISR_skips_clearing_when_no_results_available);

    RUN_TEST(test_TC_13_001_resultISR_checks_five_channels);
    RUN_TEST(test_TC_13_002_resultISR_clears_five_flags_when_all_available);

    RUN_TEST(test_TC_14_001_resultISR_accumulates_clear_calls_across_invocations);
    RUN_TEST(test_TC_14_002_resultISR_handles_mixed_availability_across_invocations);

    return UNITY_END();
}
