#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for internal ISR/callback symbols defined in production .c */
extern void IfxGtm_periodEventFunction(void *data);
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON             (1e-4f)
#define UT_NUM_CHANNELS              (3u)
#define UT_PWM_FREQ_HZ               (20000.0f)
#define UT_INIT_DUTY_U_PERCENT       (25.0f)
#define UT_INIT_DUTY_V_PERCENT       (50.0f)
#define UT_INIT_DUTY_W_PERCENT       (75.0f)
#define UT_STEP_PERCENT              (10.0f)
#define UT_DUTY_MIN_PERCENT          (10.0f)
#define UT_DUTY_MAX_PERCENT          (90.0f)
#define UT_DEAD_TIME_S               (5e-07f)
#define UT_MIN_PULSE_S               (1e-06f)

void setUp(void)    { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* Available extern spy variables from the mock */
extern unsigned int mock_IfxGtm_Pwm_init_lastNumChannels;
extern float        mock_IfxGtm_Pwm_init_lastFrequency;
extern float        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern float        mock_dt_lastDtRising[];
extern float        mock_dt_lastDtFalling[];

extern unsigned int mock_IfxGtm_isEnabled_returnValue;
extern unsigned int mock_IfxGtm_Tom_Timer_init_returnValue;

/* ========================= GROUP 1 - initGtmTom3phInv: initialization / enable guard ========================= */
void test_TC_G1_001_init_calls_expected_when_GTM_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;  /* Simulate GTM disabled */
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be checked once when GTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(),    "GTM should be enabled when previously disabled");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_getModuleFrequency_getCallCount() >= 1, "CMU module frequency should be read during enable guard path");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_enableClocks_getCallCount() >= 1,       "CMU clocks should be enabled for TOM/FXCLK when GTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(),   "IfxGtm_Pwm_initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(),         "IfxGtm_Pwm_init should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(),      "Timer init should be called exactly once");

    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_setGclkFrequency_getCallCount() >= 1,  "GCLK frequency should be configured at least once");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Cmu_setClkFrequency_getCallCount() >= 1,   "CLK frequency should be configured at least once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(),  "LED GPIO should be configured as push-pull output once during init");
}

void test_TC_G1_002_init_does_not_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;   /* Simulate GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should still be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(),    "GTM enable must NOT be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Pwm initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(),       "Pwm init should be called once");
}

/* ========================= GROUP 2 - initGtmTom3phInv: configuration values ========================= */
void test_TC_G2_001_init_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert - use init() spies per timing rule */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_sets_deadtime_rising_and_falling(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert dead-times (seconds) for all 3 channels */
    for (unsigned int i = 0; i < UT_NUM_CHANNELS; ++i)
    {
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_dt_lastDtRising[i],  "Rising-edge dead-time must be 0.5 us");
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_dt_lastDtFalling[i], "Falling-edge dead-time must be 0.5 us");
    }
}

/* ========================= GROUP 3 - initGtmTom3phInv: runtime update logic ========================= */
void test_TC_G3_001_after_init_single_update_increments_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - single step */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(),          "disableUpdate must be called once per duty update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(),  "updateChannelsDutyImmediate must be called once per duty update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),            "applyUpdate must be called once per duty update");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should increment by step to 35% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should increment by step to 60% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should increment by step to 85% after first update");
}

void test_TC_G3_002_after_init_three_updates_wraps_phase_W_only(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - three updates: U->55, V->80, W->10 (wrap on 3rd) */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(),          "disableUpdate must be called once per each update (3x)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(),  "updateChannelsDutyImmediate must be called once per each update (3x)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),            "applyUpdate must be called once per each update (3x)");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10% after three updates");
}

/* ========================= GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior ========================= */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(), "ISR should toggle the LED exactly once");
}

void test_TC_G4_002_isr_toggles_led_three_times(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_getCallCount(), "ISR should toggle the LED three times after three invocations");
}

/* ========================= GROUP 5 - updateGtmTom3phInvDuty: configuration values ========================= */
void test_TC_G5_001_post_update_num_channels_remain_3(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - values come from init() */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels should remain 3 after update");
}

void test_TC_G5_002_post_update_frequency_remains_20kHz(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - PWM frequency is configured at init() */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency should remain 20 kHz after update");
}

/* ========================= GROUP 6 - updateGtmTom3phInvDuty: runtime update logic ========================= */
void test_TC_G6_001_update_calls_hal_once_and_updates_duty(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(),          "disableUpdate must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(),  "updateChannelsDutyImmediate must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),            "applyUpdate must be called once");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty should be 35% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty should be 60% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty should be 85% after one update");
}

void test_TC_G6_002_update_clamps_above_max_and_preserves_order(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act - two updates: U->45, V->70, W->90 (clamped from 95) */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert - HAL call counts */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(),          "disableUpdate must be called once per update (2x)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(),  "updateChannelsDutyImmediate must be called once per update (2x)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),            "applyUpdate must be called once per update (2x)");

    /* Assert - clamp to 90% without wrap when 95% */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 45% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 70% after two updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 90.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be clamped to 90% (from 95%) after two updates");
}

/* ========================= GROUP 7 - interruptGtmAtom: initialization / enable guard ========================= */
void test_TC_G7_001_isr_only_toggles_gpio_no_pwm_update_calls(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert - ISR should not interact with PWM update APIs */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(),                               "ISR must toggle LED exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(),  "ISR must not call PWM duty update API");
}

void test_TC_G7_002_isr_does_not_trigger_pwm_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert - No PWM init from ISR */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR must not initialize PWM driver");
}

/* ========================= GROUP 8 - interruptGtmAtom: configuration values ========================= */
void test_TC_G8_001_isr_does_not_change_pwm_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert - PWM config persistence */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "ISR must not alter configured PWM frequency");
}

void test_TC_G8_002_isr_does_not_change_deadtime(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert - Dead-time unchanged for all channels */
    for (unsigned int i = 0; i < UT_NUM_CHANNELS; ++i)
    {
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_dt_lastDtRising[i],  "ISR must not alter rising dead-time");
        TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_dt_lastDtFalling[i], "ISR must not alter falling dead-time");
    }
}

/* ========================= GROUP 9 - interruptGtmAtom: ISR / interrupt behavior ========================= */
void test_TC_G9_001_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_getCallCount(), "LED toggle count should equal ISR invocation count");
}

void test_TC_G9_002_isr_does_not_call_pin_mode_config(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR should not (re)configure pin mode");
}

/* ========================= GROUP 10 - IfxGtm_periodEventFunction: configuration values ========================= */
void test_TC_G10_001_period_callback_has_no_gpio_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Period callback must not toggle LED");
}

void test_TC_G10_002_period_callback_keeps_pwm_config_values(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - Config remains as initialized */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Callback must not change number of channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Callback must not change PWM frequency");
}

/* ========================= GROUP 11 - IfxGtm_periodEventFunction: ISR / interrupt behavior ========================= */
void test_TC_G11_001_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM duty update");
}

void test_TC_G11_002_period_callback_accepts_null_pointer(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - Still no toggle */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Period callback with NULL data must have no GPIO side effects");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_expected_when_GTM_disabled);
    RUN_TEST(test_TC_G1_002_init_does_not_enable_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_sets_deadtime_rising_and_falling);

    RUN_TEST(test_TC_G3_001_after_init_single_update_increments_duties);
    RUN_TEST(test_TC_G3_002_after_init_three_updates_wraps_phase_W_only);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_three_times);

    RUN_TEST(test_TC_G5_001_post_update_num_channels_remain_3);
    RUN_TEST(test_TC_G5_002_post_update_frequency_remains_20kHz);

    RUN_TEST(test_TC_G6_001_update_calls_hal_once_and_updates_duty);
    RUN_TEST(test_TC_G6_002_update_clamps_above_max_and_preserves_order);

    RUN_TEST(test_TC_G7_001_isr_only_toggles_gpio_no_pwm_update_calls);
    RUN_TEST(test_TC_G7_002_isr_does_not_trigger_pwm_init);

    RUN_TEST(test_TC_G8_001_isr_does_not_change_pwm_frequency);
    RUN_TEST(test_TC_G8_002_isr_does_not_change_deadtime);

    RUN_TEST(test_TC_G9_001_isr_toggle_accumulates_across_calls);
    RUN_TEST(test_TC_G9_002_isr_does_not_call_pin_mode_config);

    RUN_TEST(test_TC_G10_001_period_callback_has_no_gpio_toggle);
    RUN_TEST(test_TC_G10_002_period_callback_keeps_pwm_config_values);

    RUN_TEST(test_TC_G11_001_period_callback_does_not_call_pwm_update);
    RUN_TEST(test_TC_G11_002_period_callback_accepts_null_pointer);

    return UNITY_END();
}
