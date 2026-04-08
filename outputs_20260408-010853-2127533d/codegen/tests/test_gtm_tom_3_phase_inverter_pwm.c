#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for functions not necessarily declared in the production header */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                         (1e-4f)
#define UT_NUM_CHANNELS                          (3)
#define UT_PWM_FREQ_HZ                           (20000.0f)
#define UT_INIT_DUTY_U_PERCENT                   (25.0f)
#define UT_INIT_DUTY_V_PERCENT                   (50.0f)
#define UT_INIT_DUTY_W_PERCENT                   (75.0f)
#define UT_STEP_PERCENT                          (10.0f)
/* Update rule per behavior_description: wrap if next >= 90, then duty = 10 */
#define UT_WRAP_THRESHOLD_PERCENT                (90.0f)
#define UT_DEADTIME_RISING_S                     (5e-07f)
#define UT_DEADTIME_FALLING_S                    (5e-07f)
#define UT_MIN_PULSE_S                           (1e-06f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ------------------------------------------------------------
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 * ------------------------------------------------------------ */
void test_TC_G1_001_init_does_not_enable_when_already_enabled(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable state must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Diagnostic LED pin must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must not be enabled if GTM already enabled");
}

void test_TC_G1_002_init_enables_and_configures_clocks_when_disabled(void)
{
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable state must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should be read when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK clocks must be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK should be configured once during enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "CLK should be configured once during enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
}

/* ------------------------------------------------------------
 * GROUP 2 - initGtmTom3phInv: configuration values
 * ------------------------------------------------------------ */
void test_TC_G2_001_init_sets_frequency_to_20kHz(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_sets_num_channels_to_3(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3 (U,V,W)");
}

/* ------------------------------------------------------------
 * GROUP 3 - initGtmTom3phInv: runtime update logic (post-init behavior)
 * ------------------------------------------------------------ */
void test_TC_G3_001_update_once_increments_duty_below_wrap(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be applied once per call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step");
}

void test_TC_G3_002_update_twice_wraps_W_only(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be applied once per call (2 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 2 updates: U = 25+10+10 = 45% (no wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 2 updates: V = 50+10+10 = 70% (no wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 2 updates: W wraps to 10% (since 75+10 >= 90 on second step)");
}

/* ------------------------------------------------------------
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior (LED toggle)
 * ------------------------------------------------------------ */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G4_002_isr_accumulates_toggle_count(void)
{
    interruptGtmAtom();
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

/* ------------------------------------------------------------
 * GROUP 5 - updateGtmTom3phInvDuty: initialization / enable guard
 * ------------------------------------------------------------ */
void test_TC_G5_001_update_called_once_per_call_when_gmt_enabled(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL update call per update() when GTM already enabled");
}

void test_TC_G5_002_update_called_once_per_call_when_gmt_was_disabled(void)
{
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL update call per update() when GTM was disabled at init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should have been enabled during init for this path");
}

/* ------------------------------------------------------------
 * GROUP 6 - updateGtmTom3phInvDuty: configuration values
 * ------------------------------------------------------------ */
void test_TC_G6_001_update_outputs_percent_units(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Duties are in percent (U=35.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Duties are in percent (V=60.0)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Duties are in percent (W=85.0)");
}

void test_TC_G6_002_update_does_not_change_frequency_or_channel_count(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    float freq_before = mock_IfxGtm_Pwm_init_lastFrequency;
    int   ch_before   = mock_IfxGtm_Pwm_init_lastNumChannels;

    updateGtmTom3phInvDuty();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freq_before, mock_IfxGtm_Pwm_init_lastFrequency, "update() must not change configured PWM frequency");
    TEST_ASSERT_EQUAL_INT_MESSAGE(ch_before, mock_IfxGtm_Pwm_init_lastNumChannels, "update() must not change number of channels");
}

/* ------------------------------------------------------------
 * GROUP 7 - updateGtmTom3phInvDuty: runtime update logic
 * ------------------------------------------------------------ */
void test_TC_G7_001_multiple_updates_wrap_each_channel_independently_case_W(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    for (int i = 0; i < 4; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update (4 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 4 updates: U=65%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 4 updates: V wraps to 10%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 4 updates: W stepped to 30% after earlier wrap");
}

void test_TC_G7_002_six_updates_wrap_U_later_than_V_and_W(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    for (int i = 0; i < 6; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(6, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update (6 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 6 updates: U=85% (wrap later at 7th)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 6 updates: V progressed post-wrap to 30%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 6 updates: W progressed post-wrap to 50%");
}

/* ------------------------------------------------------------
 * GROUP 8 - interruptGtmAtom: initialization / enable guard
 * ------------------------------------------------------------ */
void test_TC_G8_001_isr_toggle_without_init(void)
{
    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even if init not called");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update");
}

void test_TC_G8_002_isr_toggle_after_init(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED after init as well");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update after init");
}

/* ------------------------------------------------------------
 * GROUP 9 - interruptGtmAtom: ISR / interrupt behavior
 * ------------------------------------------------------------ */
void test_TC_G9_001_isr_multiple_toggles(void)
{
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggles must accumulate (3 calls)");
}

void test_TC_G9_002_isr_does_not_call_pwm_update(void)
{
    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not update PWM duties");
}

/* ------------------------------------------------------------
 * GROUP 10 - IfxGtm_periodEventFunction: initialization / enable guard
 * ------------------------------------------------------------ */
void test_TC_G10_001_period_callback_no_side_effects_before_init(void)
{
    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

void test_TC_G10_002_period_callback_no_side_effects_after_init(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED after init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties after init");
}

/* ------------------------------------------------------------
 * GROUP 11 - IfxGtm_periodEventFunction: configuration values
 * ------------------------------------------------------------ */
void test_TC_G11_001_period_callback_does_not_change_frequency(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    float freq_before = mock_IfxGtm_Pwm_init_lastFrequency;

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freq_before, mock_IfxGtm_Pwm_init_lastFrequency, "Period callback must not change configured PWM frequency");
}

void test_TC_G11_002_period_callback_does_not_change_duties(void)
{
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    updateGtmTom3phInvDuty();
    float u_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float v_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float w_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, u_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Period callback must not modify U duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, v_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Period callback must not modify V duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, w_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Period callback must not modify W duty");
}

/* ------------------------------------------------------------
 * GROUP 12 - IfxGtm_periodEventFunction: ISR / interrupt behavior
 * ------------------------------------------------------------ */
void test_TC_G12_001_period_callback_multiple_calls_no_toggle(void)
{
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must never toggle LED");
}

void test_TC_G12_002_period_callback_multiple_calls_no_pwm_update(void)
{
    for (int i = 0; i < 5; ++i)
    {
        IfxGtm_periodEventFunction(NULL);
    }

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must never call PWM duty update");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_does_not_enable_when_already_enabled);
    RUN_TEST(test_TC_G1_002_init_enables_and_configures_clocks_when_disabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_to_20kHz);
    RUN_TEST(test_TC_G2_002_init_sets_num_channels_to_3);

    RUN_TEST(test_TC_G3_001_update_once_increments_duty_below_wrap);
    RUN_TEST(test_TC_G3_002_update_twice_wraps_W_only);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_accumulates_toggle_count);

    RUN_TEST(test_TC_G5_001_update_called_once_per_call_when_gmt_enabled);
    RUN_TEST(test_TC_G5_002_update_called_once_per_call_when_gmt_was_disabled);

    RUN_TEST(test_TC_G6_001_update_outputs_percent_units);
    RUN_TEST(test_TC_G6_002_update_does_not_change_frequency_or_channel_count);

    RUN_TEST(test_TC_G7_001_multiple_updates_wrap_each_channel_independently_case_W);
    RUN_TEST(test_TC_G7_002_six_updates_wrap_U_later_than_V_and_W);

    RUN_TEST(test_TC_G8_001_isr_toggle_without_init);
    RUN_TEST(test_TC_G8_002_isr_toggle_after_init);

    RUN_TEST(test_TC_G9_001_isr_multiple_toggles);
    RUN_TEST(test_TC_G9_002_isr_does_not_call_pwm_update);

    RUN_TEST(test_TC_G10_001_period_callback_no_side_effects_before_init);
    RUN_TEST(test_TC_G10_002_period_callback_no_side_effects_after_init);

    RUN_TEST(test_TC_G11_001_period_callback_does_not_change_frequency);
    RUN_TEST(test_TC_G11_002_period_callback_does_not_change_duties);

    RUN_TEST(test_TC_G12_001_period_callback_multiple_calls_no_toggle);
    RUN_TEST(test_TC_G12_002_period_callback_multiple_calls_no_pwm_update);

    return UNITY_END();
}
