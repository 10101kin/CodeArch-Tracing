#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for production-local functions (ISR and callback) */
extern void IfxEgtm_periodEventFunction(void *data);
extern void interruptEgtmAtom(void);
extern void resultISR(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_NUM_CHANNELS_PHASE            (3u)
#define UT_NUM_CHANNELS_ADC_TRIG         (1u)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)
#define UT_DEAD_TIME_US                  (1.0f)

/* Spy variables from the mock header (provided) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern uint32  mock_IfxEgtm_Pwm_init_callCount;
extern uint32  mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern uint32  mock_togglePin_callCount;

extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
void test_TC_01_001_init_calls_target_api_when_module_disabled(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable should be called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "IfxEgtm_Cmu_enableClocks should be called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig should be called once per handle (inverter + ADC trigger)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_init_getCallCount(), "init should be called once per handle (inverter + ADC trigger)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger routing must be configured exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug pin should be configured as output once");
}

void test_TC_01_002_init_skips_enable_when_already_enabled(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable should NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks should NOT be re-enabled when already enabled by module guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig still called for both handles");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_init_getCallCount(), "init still called for both handles");
}

/* GROUP 2 - initEgtmAtom3phInv: configuration values */
void test_TC_02_001_last_init_frequency_is_20khz(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM init frequency should be 20 kHz on last init (ADC trigger handle)");
}

void test_TC_02_002_two_inits_performed_and_last_is_single_channel(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_init_callCount, "Two IfxEgtm_Pwm_init calls expected (inverter + ADC trigger)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS_ADC_TRIG, mock_IfxEgtm_Pwm_init_lastNumChannels, "Last init handle should be ADC trigger with 1 channel");
}

/* GROUP 3 - initEgtmAtom3phInv: runtime update logic */
void test_TC_03_001_init_does_not_call_update_duty_immediate(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Init should not call duty immediate update API");
}

void test_TC_03_002_init_routes_adc_trigger_once(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger routing must be configured exactly once during init");
}

/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (post-init pin setup) */
void test_TC_04_001_init_configures_isr_pin_output_once(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR debug pin must be configured as push-pull output once");
}

void test_TC_04_002_init_does_not_toggle_pin(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Init should not toggle debug pin");
}

/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
void test_TC_05_001_update_applies_phase_duties_in_percent(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 d[3] = { UT_INIT_DUTY_U_PERCENT, UT_INIT_DUTY_V_PERCENT, UT_INIT_DUTY_W_PERCENT };

    /* Act */
    updateEgtmAtom3phInvDuty(d);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty percent mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty percent mismatch");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty percent mismatch");
}

void test_TC_05_002_update_clamps_each_channel_independently(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 d[3] = { -10.0f, 150.0f, 50.0f };

    /* Act */
    updateEgtmAtom3phInvDuty(d);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,   mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should clamp to 0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should clamp to 100%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should remain 50%%");
}

/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
void test_TC_06_001_update_called_once_per_request_and_tracks_last(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 d1[3] = { 10.0f, 20.0f, 30.0f };
    float32 d2[3] = { 40.0f, 50.0f, 60.0f };

    /* Act */
    updateEgtmAtom3phInvDuty(d1);
    updateEgtmAtom3phInvDuty(d2);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two update requests should yield two HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U last duty should be 40%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V last duty should be 50%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W last duty should be 60%%");
}

void test_TC_06_002_update_preserves_fractional_percent_values(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 d[3] = { 33.3f, 66.6f, 99.9f };

    /* Act */
    updateEgtmAtom3phInvDuty(d);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL update call expected");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 33.3f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U fractional duty preserved");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 66.6f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V fractional duty preserved");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 99.9f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W fractional duty preserved");
}

/* GROUP 7 - interruptEgtmAtom: initialization / enable guard */
void test_TC_07_001_isr_toggle_once(void) {
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle pin once");
}

void test_TC_07_002_isr_toggle_accumulates(void) {
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle should accumulate across calls");
}

/* GROUP 8 - interruptEgtmAtom: configuration values */
void test_TC_08_001_isr_does_not_call_pwm_update(void) {
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update");
}

void test_TC_08_002_isr_does_not_call_pwm_init_or_trigger(void) {
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_callCount, "ISR must not call IfxEgtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Trigger_trigToAdc_callCount, "ISR must not call IfxEgtm_Trigger_trigToAdc");
}

/* GROUP 9 - interruptEgtmAtom: runtime update logic */
void test_TC_09_001_isr_multiple_calls_accumulate_toggle(void) {
    /* Act */
    for (uint32 i = 0; i < 5; ++i) {
        interruptEgtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "Five ISR calls should yield five toggles");
}

void test_TC_09_002_isr_makes_no_other_driver_calls(void) {
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_callCount, "ISR should not re-init PWM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not update PWM duties");
}

/* GROUP 10 - interruptEgtmAtom: ISR / interrupt behavior */
void test_TC_10_001_isr_toggle_once_after_init(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle once after init");
}

void test_TC_10_002_isr_toggle_accumulates_after_init(void) {
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug pin configured once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggles should accumulate after init");
}

/* GROUP 11 - IfxEgtm_periodEventFunction: configuration values */
void test_TC_11_001_period_callback_has_no_pin_side_effect(void) {
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback should not toggle pin");
}

void test_TC_11_002_period_callback_does_not_issue_pwm_update(void) {
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback should not update PWM duties");
}

/* GROUP 12 - IfxEgtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_12_001_period_callback_multiple_calls_still_no_effect(void) {
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callback calls should have no pin side effects");
}

void test_TC_12_002_period_callback_accepts_nonnull_data_pointer(void) {
    /* Act */
    int dummy = 123;
    IfxEgtm_periodEventFunction((void*)&dummy);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback with non-NULL data must still have no side effects");
}

/* GROUP 13 - resultISR: initialization / enable guard */
void test_TC_13_001_resultISR_checks_and_clears_tmadc_results(void) {
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = TRUE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "resultISR should toggle pin once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "resultISR should check 5 TMADC result registers (CH0..CH4)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "resultISR should clear 5 TMADC result flags when available");
}

void test_TC_13_002_resultISR_handles_no_available_results(void) {
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "resultISR should toggle pin even if no results are available");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxAdc_isTmadcResultAvailable_getCallCount(), "Availability should still be checked for all 5 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_clearTmadcResultFlag_getCallCount(), "No flags should be cleared when no results are available");
}

/* GROUP 14 - resultISR: ISR / interrupt behavior */
void test_TC_14_001_resultISR_toggle_accumulates(void) {
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "resultISR toggle should accumulate across calls");
}

void test_TC_14_002_resultISR_does_not_touch_pwm_update(void) {
    /* Arrange */
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "resultISR must not call PWM duty update");
}

int main(void) {
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_01_001_init_calls_target_api_when_module_disabled);
    RUN_TEST(test_TC_01_002_init_skips_enable_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_02_001_last_init_frequency_is_20khz);
    RUN_TEST(test_TC_02_002_two_inits_performed_and_last_is_single_channel);

    /* GROUP 3 */
    RUN_TEST(test_TC_03_001_init_does_not_call_update_duty_immediate);
    RUN_TEST(test_TC_03_002_init_routes_adc_trigger_once);

    /* GROUP 4 */
    RUN_TEST(test_TC_04_001_init_configures_isr_pin_output_once);
    RUN_TEST(test_TC_04_002_init_does_not_toggle_pin);

    /* GROUP 5 */
    RUN_TEST(test_TC_05_001_update_applies_phase_duties_in_percent);
    RUN_TEST(test_TC_05_002_update_clamps_each_channel_independently);

    /* GROUP 6 */
    RUN_TEST(test_TC_06_001_update_called_once_per_request_and_tracks_last);
    RUN_TEST(test_TC_06_002_update_preserves_fractional_percent_values);

    /* GROUP 7 */
    RUN_TEST(test_TC_07_001_isr_toggle_once);
    RUN_TEST(test_TC_07_002_isr_toggle_accumulates);

    /* GROUP 8 */
    RUN_TEST(test_TC_08_001_isr_does_not_call_pwm_update);
    RUN_TEST(test_TC_08_002_isr_does_not_call_pwm_init_or_trigger);

    /* GROUP 9 */
    RUN_TEST(test_TC_09_001_isr_multiple_calls_accumulate_toggle);
    RUN_TEST(test_TC_09_002_isr_makes_no_other_driver_calls);

    /* GROUP 10 */
    RUN_TEST(test_TC_10_001_isr_toggle_once_after_init);
    RUN_TEST(test_TC_10_002_isr_toggle_accumulates_after_init);

    /* GROUP 11 */
    RUN_TEST(test_TC_11_001_period_callback_has_no_pin_side_effect);
    RUN_TEST(test_TC_11_002_period_callback_does_not_issue_pwm_update);

    /* GROUP 12 */
    RUN_TEST(test_TC_12_001_period_callback_multiple_calls_still_no_effect);
    RUN_TEST(test_TC_12_002_period_callback_accepts_nonnull_data_pointer);

    /* GROUP 13 */
    RUN_TEST(test_TC_13_001_resultISR_checks_and_clears_tmadc_results);
    RUN_TEST(test_TC_13_002_resultISR_handles_no_available_results);

    /* GROUP 14 */
    RUN_TEST(test_TC_14_001_resultISR_toggle_accumulates);
    RUN_TEST(test_TC_14_002_resultISR_does_not_touch_pwm_update);

    return UNITY_END();
}
