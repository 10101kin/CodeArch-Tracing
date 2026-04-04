#include "unity.h"
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* Extern declarations for functions not exposed in the production header */
extern void resultISR(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON            (1e-4f)
#define UT_NUM_CHANNELS             (3)
#define UT_INIT_DUTY_U_PERCENT      (25.0f)
#define UT_INIT_DUTY_V_PERCENT      (50.0f)
#define UT_INIT_DUTY_W_PERCENT      (75.0f)
#define UT_PWM_FREQ                 (20000.0f)
#define UT_STEP_PERCENT             (0.01f)
#define UT_DEADTIME_RISING_US       (1.0f)
#define UT_DEADTIME_FALLING_US      (1.0f)

void setUp(void)   { mock_egtm_atom_adc_tmadc_multiple_channels_reset(); }
void tearDown(void) {}

/*
GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
*/
void test_TC_G1_001_init_calls_target_tc4xx_apis_when_egtm_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - EGTM enable guard path + core init calls */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),         "IfxEgtm_enable must be called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "CMU setGclkFrequency must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CMU setClkFrequency (ECLK0) must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU enableClocks must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called once");

    /* Assert - ADC trigger and TMADC bring-up */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(),       "EGTM->ADC trigger must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(),   "TMADC initModuleConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_initModule_getCallCount(),         "TMADC initModule must be called once");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxAdc_Tmadc_initChannelConfig_getCallCount() >= 1,        "TMADC initChannelConfig must be called for at least one channel");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxAdc_Tmadc_initChannel_getCallCount() >= 1,              "TMADC initChannel must be called for at least one channel");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxAdc_Tmadc_enableChannelEvent_getCallCount() >= 1,       "TMADC enableChannelEvent must be called for at least one channel");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_runModule_getCallCount(),          "TMADC runModule must be called once");

    /* Assert - Debug GPIO configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(),        "Debug GPIO must be configured as push-pull output once");
}

void test_TC_G1_002_init_skips_enable_path_when_egtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - no enable sequence when already enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "IfxEgtm_isEnabled must still be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),         "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "CMU setGclkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CMU setClkFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU enableClocks must NOT be called when already enabled");

    /* Core init still occurs */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "EGTM->ADC trigger must still be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(),  "Debug GPIO must be configured once");
}

/*
GROUP 2 - initEgtmAtom3phInv: configuration values
*/
void test_TC_G2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_configures_adc_trigger_and_runs_tmadc(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "IfxEgtm_Trigger_trigToAdc must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxAdc_Tmadc_runModule_getCallCount(),    "TMADC must be started once with runModule");
}

/*
GROUP 3 - initEgtmAtom3phInv: runtime update logic
*/
void test_TC_G3_001_update_after_init_increments_duty_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert - one HAL update, expected duties increased by step */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty immediate update must be called once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increase by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increase by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increase by step");
}

void test_TC_G3_002_update_wraps_to_step_on_overflow(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 nearOverflow[3] = {99.99f, 99.99f, 99.99f};
    updateEgtmAtom3phInvDuty(nearOverflow);

    /* Act */
    updateEgtmAtomDuty();

    /* Assert - wrap to step (post-wrap value equals step) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate update must be called once per API invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must wrap to step after exceeding 100%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap to step after exceeding 100%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to step after exceeding 100%");
}

/*
GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior
*/
void test_TC_G4_001_resultISR_toggles_debug_gpio_once(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "resultISR must toggle the debug GPIO exactly once");
}

void test_TC_G4_002_resultISR_toggle_accumulates_over_multiple_calls(void)
{
    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "resultISR must accumulate toggles across calls");
}

/*
GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
*/
void test_TC_G5_001_updateDuty_immediate_calls_target_api_once_and_sets_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 req[3] = {10.0f, 20.0f, 30.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(req);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must match request");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must match request");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must match request");
}

void test_TC_G5_002_updateDuty_overwrites_previous_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 first[3]  = { 1.0f,  2.0f,  3.0f};
    float32 second[3] = { 4.0f,  5.0f,  6.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(first);
    updateEgtmAtom3phInvDuty(second);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called twice");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 4.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Latest Phase U duty must be applied");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 5.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Latest Phase V duty must be applied");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 6.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Latest Phase W duty must be applied");
}

/*
GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
*/
void test_TC_G6_001_updateDuty_accepts_full_scale_percent_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 req[3] = {0.0f, 100.0f, 50.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(req);

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,   mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 100%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 50%%");
}

void test_TC_G6_002_updateDuty_preserves_channel_independence(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 req[3] = {5.0f, 95.0f, 55.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(req);

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 5.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be independent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be independent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be independent");
}

/*
GROUP 7 - updateEgtmAtomDuty: initialization / enable guard
*/
void test_TC_G7_001_updateDuty_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each call to updateEgtmAtomDuty must invoke one HAL update");
}

void test_TC_G7_002_updateDuty_multiple_calls_increment_callcount_accordingly(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();
    updateEgtmAtomDuty();
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three calls to updateEgtmAtomDuty must yield three HAL updates");
}

/*
GROUP 8 - updateEgtmAtomDuty: configuration values
*/
void test_TC_G8_001_updateDuty_respects_percent_units_and_increments_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 req[3] = {10.0f, 20.0f, 30.0f};
    updateEgtmAtom3phInvDuty(req);

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be in percent and increase by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be in percent and increase by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be in percent and increase by step");
}

void test_TC_G8_002_updateDuty_clamps_negative_to_zero(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 neg[3] = {-0.50f, -1.00f,  0.00f};
    updateEgtmAtom3phInvDuty(neg);

    /* Act */
    updateEgtmAtomDuty();

    /* Assert - negatives clamp to 0 after update step handling */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Negative Phase U must clamp to 0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Negative Phase V must clamp to 0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Zero Phase W must increase by step");
}

/*
GROUP 9 - updateEgtmAtomDuty: runtime update logic
*/
void test_TC_G9_001_wrap_around_is_independent_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 req[3] = {99.99f, 50.00f,  0.00f};
    updateEgtmAtom3phInvDuty(req);

    /* Act */
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT,            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U wraps to step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.00f + UT_STEP_PERCENT,   mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V increments without wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.00f + UT_STEP_PERCENT,    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W increments from zero");
}

void test_TC_G9_002_multiple_updates_accumulate_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();
    float32 zero[3] = {0.0f, 0.0f, 0.0f};
    updateEgtmAtom3phInvDuty(zero);

    /* Act */
    updateEgtmAtomDuty();
    updateEgtmAtomDuty();
    updateEgtmAtomDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Total HAL updates: 1 (set) + 3 (increments)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 3.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must accumulate 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 3.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must accumulate 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 3.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must accumulate 3 steps");
}

/*
GROUP 10 - resultISR: initialization / enable guard
*/
void test_TC_G10_001_isr_does_not_call_pwm_update(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update API");
}

void test_TC_G10_002_isr_does_not_init_pwm_or_adc(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_getCallCount(),                "ISR must not call IfxEgtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_Tmadc_initModule_getCallCount(),         "ISR must not call IfxAdc_Tmadc_initModule");
}

/*
GROUP 11 - resultISR: ISR / interrupt behavior
*/
void test_TC_G11_001_resultISR_toggle_once(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "resultISR must toggle once");
}

void test_TC_G11_002_resultISR_toggle_three_times(void)
{
    /* Act */
    resultISR();
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "resultISR must toggle three times after three invocations");
}

/*
GROUP 12 - IfxEgtm_periodEventFunction: configuration values (no side effects)
*/
void test_TC_G12_001_period_callback_has_no_side_effect_on_toggle(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle any GPIO");
}

void test_TC_G12_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM update");
}

/*
GROUP 13 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (remains empty)
*/
void test_TC_G13_001_period_callback_multiple_invocations_no_toggle(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callback invocations must not toggle GPIO");
}

void test_TC_G13_002_period_callback_multiple_invocations_no_inits(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_getCallCount(),              "Period callback must not initialize PWM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxAdc_Tmadc_initModule_getCallCount(),       "Period callback must not initialize TMADC");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_target_tc4xx_apis_when_egtm_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_path_when_egtm_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_configures_adc_trigger_and_runs_tmadc);

    RUN_TEST(test_TC_G3_001_update_after_init_increments_duty_below_boundary);
    RUN_TEST(test_TC_G3_002_update_wraps_to_step_on_overflow);

    RUN_TEST(test_TC_G4_001_resultISR_toggles_debug_gpio_once);
    RUN_TEST(test_TC_G4_002_resultISR_toggle_accumulates_over_multiple_calls);

    RUN_TEST(test_TC_G5_001_updateDuty_immediate_calls_target_api_once_and_sets_duties);
    RUN_TEST(test_TC_G5_002_updateDuty_overwrites_previous_values);

    RUN_TEST(test_TC_G6_001_updateDuty_accepts_full_scale_percent_values);
    RUN_TEST(test_TC_G6_002_updateDuty_preserves_channel_independence);

    RUN_TEST(test_TC_G7_001_updateDuty_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G7_002_updateDuty_multiple_calls_increment_callcount_accordingly);

    RUN_TEST(test_TC_G8_001_updateDuty_respects_percent_units_and_increments_by_step);
    RUN_TEST(test_TC_G8_002_updateDuty_clamps_negative_to_zero);

    RUN_TEST(test_TC_G9_001_wrap_around_is_independent_per_channel);
    RUN_TEST(test_TC_G9_002_multiple_updates_accumulate_by_step);

    RUN_TEST(test_TC_G10_001_isr_does_not_call_pwm_update);
    RUN_TEST(test_TC_G10_002_isr_does_not_init_pwm_or_adc);

    RUN_TEST(test_TC_G11_001_resultISR_toggle_once);
    RUN_TEST(test_TC_G11_002_resultISR_toggle_three_times);

    RUN_TEST(test_TC_G12_001_period_callback_has_no_side_effect_on_toggle);
    RUN_TEST(test_TC_G12_002_period_callback_does_not_call_pwm_update);

    RUN_TEST(test_TC_G13_001_period_callback_multiple_invocations_no_toggle);
    RUN_TEST(test_TC_G13_002_period_callback_multiple_invocations_no_inits);

    return UNITY_END();
}
