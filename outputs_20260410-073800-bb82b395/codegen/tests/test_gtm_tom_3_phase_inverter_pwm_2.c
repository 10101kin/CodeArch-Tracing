#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm_2.h"
#include "gtm_tom_3_phase_inverter_pwm_2.h"

/* Extern declarations for ISR and period callback implemented in production .c */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Unit-test configuration constants */
#define UT_FLOAT_EPSILON              (1e-4f)
#define UT_PWM_FREQ_HZ                (20000.0f)
#define UT_NUM_CHANNELS               (3)
#define UT_INIT_DUTY_U_PERCENT        (25.0f)
#define UT_INIT_DUTY_V_PERCENT        (50.0f)
#define UT_INIT_DUTY_W_PERCENT        (75.0f)
#define UT_DUTY_STEP_PERCENT          (10.0f)
#define UT_DEADTIME_SEC               (1e-6f)
#define UT_TEST_CMU_FREQ_HZ           (100000000.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_2_reset(); }
void tearDown(void) {}

/* ==========================================================================
   GROUP 1 - initGtmTom3phInv: initialization / enable guard
   ========================================================================== */
void test_TC_01_001_init_enables_GTM_and_configures_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_TEST_CMU_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency queried once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should be set when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency should be set when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Functional clocks should be enabled when GTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output once after init");
}

void test_TC_01_002_init_skips_enable_and_clocks_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should not be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency set should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK frequency set should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Clock enable should be skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output once after init");
}

/* ==========================================================================
   GROUP 2 - initGtmTom3phInv: configuration values
   ========================================================================== */
void test_TC_02_001_init_sets_frequency_20kHz_and_numChannels_3(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE; /* keep clocks path quiet */

    /* Act */
    initGtmTom3phInv();

    /* Assert application-configured values captured at init(), not initConfig() */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency should be 20 kHz at init()");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels should be 3");
}

void test_TC_02_002_after_init_first_update_reflects_initial_duties_plus_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be issued once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U first update should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V first update should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W first update should be 85%");
}

/* ==========================================================================
   GROUP 3 - initGtmTom3phInv: runtime update logic
   ========================================================================== */
void test_TC_03_001_update_increments_each_channel_by_step_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update should call HAL once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U = 35% after 1 update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V = 60% after 1 update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W = 85% after 1 update");
}

void test_TC_03_002_update_wraps_independently_per_channel_after_three_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update should be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U = 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V = 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W wraps to 10% after 3 updates");
}

/* ==========================================================================
   GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
   ========================================================================== */
void test_TC_04_001_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED exactly once");
}

void test_TC_04_002_isr_multiple_calls_accumulate_toggle_count(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR should toggle LED three times across three invocations");
}

/* ==========================================================================
   GROUP 5 - updateGtmTom3phInvDuty: configuration values
   ========================================================================== */
void test_TC_05_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be issued exactly once per call");
}

void test_TC_05_002_update_operates_on_three_logical_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init() must configure exactly 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Index 0 duty present after update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Index 1 duty present after update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Index 2 duty present after update");
}

/* ==========================================================================
   GROUP 6 - updateGtmTom3phInvDuty: runtime update logic
   ========================================================================== */
void test_TC_06_001_update_second_call_accumulates_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update should be called once per update call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U = 45% after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V = 70% after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W = 95% after 2 updates");
}

void test_TC_06_002_update_wrap_rule_applies_independently_after_five_calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: 5 updates → U=75, V wraps to 10, W=30 */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */
    updateGtmTom3phInvDuty(); /* 4 */
    updateGtmTom3phInvDuty(); /* 5 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update should be called five times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U = 75% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V wraps to 10% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W = 30% after 5 updates");
}

/* ==========================================================================
   GROUP 7 - interruptGtmAtom: initialization / enable guard
   ========================================================================== */
void test_TC_07_001_isr_does_not_call_pwm_or_cmu_functions(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert: No PWM/CMU functions should be called by ISR */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR must not call IfxGtm_Pwm_init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not update PWM duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "ISR must not configure CMU clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "ISR must not configure GCLK");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "ISR must not enable CMU clocks");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR still toggles LED exactly once");
}

void test_TC_07_002_isr_toggle_independent_of_init_configuration(void)
{
    /* Act: do not call init, just ISR */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR should toggle LED even without prior init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR should not configure LED pin (init does)");
}

/* ==========================================================================
   GROUP 8 - interruptGtmAtom: configuration values
   ========================================================================== */
void test_TC_08_001_isr_does_not_affect_pwm_configuration_values(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert: Frequency/numChannels remain as configured by init */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "ISR must not change PWM frequency configuration");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "ISR must not change number of channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured exactly once by init, not ISR");
}

void test_TC_08_002_isr_does_not_invoke_pwm_duty_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: only ISR, no update() */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not trigger duty update");
}

/* ==========================================================================
   GROUP 9 - interruptGtmAtom: ISR / interrupt behavior
   ========================================================================== */
void test_TC_09_001_isr_single_call_results_in_single_toggle(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "Single ISR call → one LED toggle");
}

void test_TC_09_002_isr_three_calls_result_in_three_toggles(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "Three ISR calls → three LED toggles");
}

/* ==========================================================================
   GROUP 10 - IfxGtm_periodEventFunction: configuration values
   ========================================================================== */
void test_TC_10_001_period_callback_has_no_side_effect_on_led_or_pwm(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction((void*)0);

    /* Assert: No LED toggle, no PWM duty update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

void test_TC_10_002_period_callback_keeps_pwm_configuration_intact(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction((void*)0);

    /* Assert: Configuration remains as set by init */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Period callback must not modify PWM frequency config");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Period callback must not modify channel count");
}

/* ==========================================================================
   GROUP 11 - IfxGtm_periodEventFunction: ISR / interrupt behavior
   ========================================================================== */
void test_TC_11_001_period_callback_does_not_toggle_led_even_if_called_multiple_times(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void*)0);
    IfxGtm_periodEventFunction((void*)0);
    IfxGtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must remain empty and not toggle LED");
}

void test_TC_11_002_period_callback_does_not_call_hal_update_functions(void)
{
    /* Act */
    IfxGtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not invoke PWM duty update HAL");
}

/* ========================================================================== */
int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_01_001_init_enables_GTM_and_configures_clocks_when_disabled);
    RUN_TEST(test_TC_01_002_init_skips_enable_and_clocks_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_02_001_init_sets_frequency_20kHz_and_numChannels_3);
    RUN_TEST(test_TC_02_002_after_init_first_update_reflects_initial_duties_plus_step);

    /* Group 3 */
    RUN_TEST(test_TC_03_001_update_increments_each_channel_by_step_below_boundary);
    RUN_TEST(test_TC_03_002_update_wraps_independently_per_channel_after_three_calls);

    /* Group 4 */
    RUN_TEST(test_TC_04_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_04_002_isr_multiple_calls_accumulate_toggle_count);

    /* Group 5 */
    RUN_TEST(test_TC_05_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_05_002_update_operates_on_three_logical_channels);

    /* Group 6 */
    RUN_TEST(test_TC_06_001_update_second_call_accumulates_step);
    RUN_TEST(test_TC_06_002_update_wrap_rule_applies_independently_after_five_calls);

    /* Group 7 */
    RUN_TEST(test_TC_07_001_isr_does_not_call_pwm_or_cmu_functions);
    RUN_TEST(test_TC_07_002_isr_toggle_independent_of_init_configuration);

    /* Group 8 */
    RUN_TEST(test_TC_08_001_isr_does_not_affect_pwm_configuration_values);
    RUN_TEST(test_TC_08_002_isr_does_not_invoke_pwm_duty_update);

    /* Group 9 */
    RUN_TEST(test_TC_09_001_isr_single_call_results_in_single_toggle);
    RUN_TEST(test_TC_09_002_isr_three_calls_result_in_three_toggles);

    /* Group 10 */
    RUN_TEST(test_TC_10_001_period_callback_has_no_side_effect_on_led_or_pwm);
    RUN_TEST(test_TC_10_002_period_callback_keeps_pwm_configuration_intact);

    /* Group 11 */
    RUN_TEST(test_TC_11_001_period_callback_does_not_toggle_led_even_if_called_multiple_times);
    RUN_TEST(test_TC_11_002_period_callback_does_not_call_hal_update_functions);

    return UNITY_END();
}
