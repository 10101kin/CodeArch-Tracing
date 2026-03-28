#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for production-local functions (callback/ISR) */
extern void IfxGtm_periodEventFunction(void *data);
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                     (1e-4f)
#define UT_NUM_CHANNELS                      (3)
#define UT_PWM_FREQUENCY_HZ_FLOAT            (20000.0f)
#define UT_INIT_DUTY_U_PERCENT               (25.0f)
#define UT_INIT_DUTY_V_PERCENT               (50.0f)
#define UT_INIT_DUTY_W_PERCENT               (75.0f)
#define UT_STEP_PERCENT                      (10.0f)
#define UT_DEADTIME_RISING_US                (1.0f)
#define UT_DEADTIME_FALLING_US               (1.0f)
#define UT_MIN_PULSE_US                      (1.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/************************************/
/* GROUP 1 - initGtmTomPwm: enable guard */
/************************************/
void test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000; /* any valid module frequency */

    /* Act */
    initGtmTomPwm();

    /* Assert driver call sequence under enable guard */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "init must check IfxGtm_isEnabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "init must enable GTM when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "init must read GTM module frequency when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "init must enable FXCLK clocks when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "init must call IfxGtm_Pwm_initConfig exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must call IfxGtm_Pwm_init exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "init must configure LED pin mode once after PWM init");

    /* Assert config via init-spy values */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM init must configure 3 logical channels (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ_FLOAT, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTomPwm();

    /* Assert - no reconfiguration of clocks when already enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "init must check IfxGtm_isEnabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "init must NOT enable GTM when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "init must NOT read module frequency when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "init must NOT re-enable clocks when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "init must still call IfxGtm_Pwm_initConfig once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must still call IfxGtm_Pwm_init once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once after PWM init");
}

/**********************************************/
/* GROUP 2 - initGtmTomPwm: configuration values */
/**********************************************/
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* allow full enable path; not strictly required */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000;

    /* Act */
    initGtmTomPwm();

    /* Assert application-set values captured at init() time */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM must be configured for 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ_FLOAT, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz at init");
}

void test_TC_G2_002_init_does_not_call_duty_update(void)
{
    /* Arrange & Act */
    initGtmTomPwm();

    /* Assert - no duty update during init; duties applied from config */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "init must not call immediate duty update");
}

/**********************************************/
/* GROUP 3 - initGtmTomPwm: runtime update logic */
/**********************************************/
void test_TC_G3_001_update_after_init_increments_duty(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert - percent-based duties increment by step */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must advance by step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must advance by step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must advance by step to 85%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once per update");
}

void test_TC_G3_002_three_updates_wrap_W_only(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act - 3 updates: U=55, V=80, W wraps to 10 */
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 3 updates, U must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 3 updates, V must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 3 updates, W must wrap to 10%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update invocation");
}

/**********************************************/
/* GROUP 4 - initGtmTomPwm: ISR / interrupt behavior */
/**********************************************/
void test_TC_G4_001_isr_toggle_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert - prefer mock_togglePin_callCount (single underlying counter) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G4_002_isr_toggle_accumulates_multiple_calls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR must accumulate toggles across calls");
}

/*****************************************************/
/* GROUP 5 - updateGtmTomPwmDutyCycles: configuration values */
/*****************************************************/
void test_TC_G5_001_update_sets_percent_values_for_all_channels(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert - percent inputs (not fractions) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 35% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 60% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 85% after one update");
}

void test_TC_G5_002_init_reports_expected_frequency_and_channel_count(void)
{
    /* Arrange & Act */
    initGtmTomPwm();

    /* Assert - via init spies */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ_FLOAT, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must be 20 kHz");
}

/**********************************************/
/* GROUP 6 - updateGtmTomPwmDutyCycles: runtime update logic */
/**********************************************/
void test_TC_G6_001_increment_below_boundary_single_call(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U must increment to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V must increment to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W must increment to 85%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL call per update");
}

void test_TC_G6_002_wrap_logic_independent_across_channels(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act - 5 updates to trigger V wrap, W already wrapped earlier */
    for (int i = 0; i < 5; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    /* After 5 updates starting from {25,50,75}: U=75, V=10 (wrap), W=30 (wrapped at 3rd) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U must be 75% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V must wrap to 10% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W must be 30% after 5 updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called once per update (5 times)");
}

/****************************************************/ 
/* GROUP 7 - IfxGtm_periodEventFunction: configuration values */
/****************************************************/
void test_TC_G7_001_period_callback_has_no_side_effects_single_call(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - empty body: no LED toggle, no PWM duty update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not trigger duty update");
}

void test_TC_G7_002_period_callback_does_not_change_config_values(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - config remains as set by init */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Callback must not alter number of channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ_FLOAT, mock_IfxGtm_Pwm_init_lastFrequency, "Callback must not alter PWM frequency");
}

/****************************************************/ 
/* GROUP 8 - IfxGtm_periodEventFunction: ISR / interrupt behavior */
/****************************************************/
void test_TC_G8_001_period_callback_multiple_calls_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED on any call");
}

void test_TC_G8_002_period_callback_multiple_calls_no_pwm_update(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not issue PWM duty updates");
}

/************************************************************/ 
/* GROUP 9 - interruptGtmAtom: initialization / enable guard */
/************************************************************/
void test_TC_G9_001_isr_toggles_without_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert - ISR independent of PWM init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even if PWM not initialized");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR must not call PWM init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call duty update");
}

void test_TC_G9_002_isr_toggles_after_init_as_well(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED after PWM init as well");
}

/****************************************************/ 
/* GROUP 10 - interruptGtmAtom: configuration values */
/****************************************************/
void test_TC_G10_001_isr_does_not_change_pwm_config_values(void)
{
    /* Arrange */
    initGtmTomPwm();

    /* Act */
    interruptGtmAtom();

    /* Assert - PWM config unchanged */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "ISR must not change number of channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ_FLOAT, mock_IfxGtm_Pwm_init_lastFrequency, "ISR must not change PWM frequency");
}

void test_TC_G10_002_isr_does_not_call_duty_update_even_after_updates(void)
{
    /* Arrange */
    initGtmTomPwm();
    updateGtmTomPwmDutyCycles(); /* One duty update to set baseline */
    float lastU = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float lastV = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float lastW = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Act - ISR should not modify duties */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert - duty values unchanged; only LED toggled */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, lastU, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "ISR must not change U duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, lastV, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "ISR must not change V duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, lastW, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "ISR must not change W duty");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must toggle LED twice");
}

/********************************************/ 
/* GROUP 11 - interruptGtmAtom: ISR behavior */
/********************************************/
void test_TC_G11_001_isr_toggle_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G11_002_isr_toggle_five_times(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        interruptGtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "ISR must toggle LED five times");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_does_not_call_duty_update);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_after_init_increments_duty);
    RUN_TEST(test_TC_G3_002_three_updates_wrap_W_only);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggle_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_sets_percent_values_for_all_channels);
    RUN_TEST(test_TC_G5_002_init_reports_expected_frequency_and_channel_count);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_increment_below_boundary_single_call);
    RUN_TEST(test_TC_G6_002_wrap_logic_independent_across_channels);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_period_callback_has_no_side_effects_single_call);
    RUN_TEST(test_TC_G7_002_period_callback_does_not_change_config_values);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_period_callback_multiple_calls_no_toggle);
    RUN_TEST(test_TC_G8_002_period_callback_multiple_calls_no_pwm_update);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_isr_toggles_without_init);
    RUN_TEST(test_TC_G9_002_isr_toggles_after_init_as_well);

    /* Group 10 */
    RUN_TEST(test_TC_G10_001_isr_does_not_change_pwm_config_values);
    RUN_TEST(test_TC_G10_002_isr_does_not_call_duty_update_even_after_updates);

    /* Group 11 */
    RUN_TEST(test_TC_G11_001_isr_toggle_once);
    RUN_TEST(test_TC_G11_002_isr_toggle_five_times);

    return UNITY_END();
}
