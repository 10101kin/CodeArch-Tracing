#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Required externs for functions defined only in the production .c */
extern void EgtmAtomPeriodIsr(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEAD_TIME_SEC                 (1e-06f)
#define UT_ISR_PRIORITY                  (20U)

void setUp(void)
{
    mock_egtm_atom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/*
GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
*/
void test_TC_G1_001_init_enables_and_configures_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: TC4xx API usage and enable path taken */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency configuration must occur when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "ATOM CLK frequency configuration must occur when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: No re-enable, no CMU config; still initializes PWM and LED pin */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency config should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "ATOM CLK frequency config should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks enable should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once");
}

/*
GROUP 2 - initEgtmAtom3phInv: configuration values
*/
void test_TC_G2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* skip CMU path */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of PWM channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_calls_init_once_with_expected_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "init() must be configured for 3 logical channels");
}

/*
GROUP 3 - initEgtmAtom3phInv: runtime update logic (exercise update after init)
*/
void test_TC_G3_001_first_update_increments_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step");
}

void test_TC_G3_002_multiple_updates_wrap_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 3 updates; W will wrap on 3rd */
    updateEgtmAtom3phInvDuty(); /* 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* 45,70,95 */
    updateEgtmAtom3phInvDuty(); /* 55,80,10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty wraps to step value");
}

/*
GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (LED toggle)
*/
void test_TC_G4_001_isr_toggles_led_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Precondition: toggle count must start at 0");

    /* Act */
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G4_002_isr_accumulates_toggles_across_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must toggle LED on every invocation");
}

/*
GROUP 5 - updateEgtmAtom3phInvDuty: configuration values around update path
*/
void test_TC_G5_001_update_calls_hal_once_and_preserves_channel_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Channel count must remain 3 after update");
}

void test_TC_G5_002_update_does_not_change_configured_pwm_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency remains as configured (20 kHz)");
}

/*
GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
*/
void test_TC_G6_001_wrap_behavior_after_three_updates_affects_W_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 3 updates → W wraps */
    updateEgtmAtom3phInvDuty(); /* 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* 45,70,95 */
    updateEgtmAtom3phInvDuty(); /* 55,80,10 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W wraps to step value after boundary");
}

void test_TC_G6_002_wrap_behavior_after_eight_updates_affects_U_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: 8 updates → U wraps on 8th */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 updates: U=10, V=40, W=60 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U wraps to step value after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after 8 updates");
}

/*
GROUP 7 - EgtmAtomPeriodIsr: initialization / enable guard around ISR
*/
void test_TC_G7_001_isr_toggles_when_module_was_disabled_then_enabled_on_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* force enable path in init */
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even when module was initially disabled");
}

void test_TC_G7_002_isr_toggles_when_module_was_already_enabled_on_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* skip enable path */
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED regardless of enable guard path");
}

/*
GROUP 8 - EgtmAtomPeriodIsr: configuration values (no side effects beyond toggle)
*/
void test_TC_G8_001_isr_does_not_invoke_pwm_duty_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call duty update HAL");
}

void test_TC_G8_002_isr_multiple_calls_still_do_not_invoke_pwm_duty_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must never call duty update HAL across invocations");
}

/*
GROUP 9 - EgtmAtomPeriodIsr: ISR / interrupt behavior (additional)
*/
void test_TC_G9_001_isr_three_calls_toggle_count_is_three(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();
    EgtmAtomPeriodIsr();
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR must toggle LED on each of three invocations");
}

void test_TC_G9_002_isr_toggle_independent_of_pwm_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    EgtmAtomPeriodIsr();
    updateEgtmAtom3phInvDuty();
    EgtmAtomPeriodIsr();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggles LED regardless of PWM duty updates in between");
}

/*
GROUP 10 - IfxEgtm_periodEventFunction: configuration values (no-op callback)
*/
void test_TC_G10_001_period_callback_is_noop_and_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Precondition: no toggles before callback");

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G10_002_period_callback_multiple_calls_still_no_side_effects(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must remain no-op across multiple calls");
}

/*
GROUP 11 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (confirm no HAL)
*/
void test_TC_G11_001_period_callback_does_not_call_duty_update_hal(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not request duty update");
}

void test_TC_G11_002_period_callback_does_not_affect_pwm_configuration_spies(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32 preCalls = mock_IfxEgtm_Pwm_init_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(preCalls, mock_IfxEgtm_Pwm_init_getCallCount(), "Period callback must not re-initialize PWM");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_calls_init_once_with_expected_channel_count);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_without_wrap);
    RUN_TEST(test_TC_G3_002_multiple_updates_wrap_independently);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_isr_accumulates_toggles_across_calls);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_and_preserves_channel_config);
    RUN_TEST(test_TC_G5_002_update_does_not_change_configured_pwm_frequency);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_wrap_behavior_after_three_updates_affects_W_only);
    RUN_TEST(test_TC_G6_002_wrap_behavior_after_eight_updates_affects_U_only);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_isr_toggles_when_module_was_disabled_then_enabled_on_init);
    RUN_TEST(test_TC_G7_002_isr_toggles_when_module_was_already_enabled_on_init);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_isr_does_not_invoke_pwm_duty_update);
    RUN_TEST(test_TC_G8_002_isr_multiple_calls_still_do_not_invoke_pwm_duty_update);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_isr_three_calls_toggle_count_is_three);
    RUN_TEST(test_TC_G9_002_isr_toggle_independent_of_pwm_updates);

    /* GROUP 10 */
    RUN_TEST(test_TC_G10_001_period_callback_is_noop_and_does_not_toggle_led);
    RUN_TEST(test_TC_G10_002_period_callback_multiple_calls_still_no_side_effects);

    /* GROUP 11 */
    RUN_TEST(test_TC_G11_001_period_callback_does_not_call_duty_update_hal);
    RUN_TEST(test_TC_G11_002_period_callback_does_not_affect_pwm_configuration_spies);

    return UNITY_END();
}
