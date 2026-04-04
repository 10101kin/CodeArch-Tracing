#include "unity.h"
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* Extern ISR symbol (defined in production .c) */
extern void interruptEgtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_NUM_PWM_CHANNELS              (3U)
#define UT_DEAD_TIME_US                  (1.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (0.01f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)

void setUp(void)   { mock_egtm_atom_adc_tmadc_multiple_channels_reset(); }
void tearDown(void) {}

/*
  SPY VARIABLES (provided by mock_egtm_atom_adc_tmadc_multiple_channels.h):
    - mock_IfxEgtm_Pwm_init_lastNumChannels
    - mock_IfxEgtm_Pwm_init_lastFrequency
    - mock_IfxEgtm_Pwm_init_lastDtRising[]
    - mock_IfxEgtm_Pwm_init_lastDtFalling[]
    - mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[]
    - mock_togglePin_callCount
*/

/* ============================= GROUP 1: initEgtmAtom — initialization / enable guard ============================= */
void test_TC_G1_001_init_sets_frequency_and_numChannels(void)
{
    /* Act */
    initEgtmAtom();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz (switching frequency), not CMU clock");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_PWM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "initEgtmAtom must configure 3 logical channels for 3-phase PWM");
}

void test_TC_G1_002_init_idempotent_second_call_preserves_config(void)
{
    /* Act */
    initEgtmAtom();
    initEgtmAtom();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Second init must keep PWM frequency at 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_PWM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Second init must keep number of logical channels at 3");
}

/* ============================= GROUP 2: initEgtmAtom — configuration values ============================= */
void test_TC_G2_001_init_applies_deadtime_us_for_three_inverter_channels(void)
{
    /* Act */
    initEgtmAtom();

    /* Assert dead-time (rising and falling) = 1.0 us for U, V, W */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtRising[0], "Phase U rising dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtFalling[0], "Phase U falling dead-time must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtRising[1], "Phase V rising dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtFalling[1], "Phase V falling dead-time must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtRising[2], "Phase W rising dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_US, mock_IfxEgtm_Pwm_init_lastDtFalling[2], "Phase W falling dead-time must be 1.0 us");
}

void test_TC_G2_002_init_sets_pwm_frequency_20khz_confirm_again(void)
{
    /* Act */
    initEgtmAtom();

    /* Assert (reaffirm frequency requirement here in config group) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Unified IfxEgtm_Pwm_init must be configured for 20 kHz switching");
}

/* ============================= GROUP 3: initEgtmAtom — runtime update logic ============================= */
void test_TC_G3_001_update_increments_each_phase_by_step(void)
{
    /* Arrange */
    initEgtmAtom();

    /* Act: one update */
    updateEgtmAtomDuty();

    /* Assert duties in PERCENT (API uses percent) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step (percent)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step (percent)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step (percent)");
}

void test_TC_G3_002_update_accumulates_across_multiple_calls(void)
{
    /* Arrange */
    initEgtmAtom();

    /* Act: two updates */
    updateEgtmAtomDuty();
    updateEgtmAtomDuty();

    /* Assert accumulated duties */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + 2.0f * UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must accumulate across updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + 2.0f * UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must accumulate across updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + 2.0f * UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must accumulate across updates");
}

/* ============================= GROUP 4: initEgtmAtom — ISR / interrupt behavior ============================= */
void test_TC_G4_001_isr_toggles_diagnostic_pin_once(void)
{
    /* Arrange */
    initEgtmAtom();

    /* Act */
    interruptEgtmAtom();

    /* Assert: ISR must toggle diagnostic pin exactly once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "ISR must toggle diagnostic GPIO once per invocation");
}

void test_TC_G4_002_isr_toggle_accumulates_across_multiple_calls(void)
{
    /* Arrange */
    initEgtmAtom();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert: cumulative toggles */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_togglePin_callCount, "ISR toggle count must accumulate across multiple invocations");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_sets_frequency_and_numChannels);
    RUN_TEST(test_TC_G1_002_init_idempotent_second_call_preserves_config);

    RUN_TEST(test_TC_G2_001_init_applies_deadtime_us_for_three_inverter_channels);
    RUN_TEST(test_TC_G2_002_init_sets_pwm_frequency_20khz_confirm_again);

    RUN_TEST(test_TC_G3_001_update_increments_each_phase_by_step);
    RUN_TEST(test_TC_G3_002_update_accumulates_across_multiple_calls);

    RUN_TEST(test_TC_G4_001_isr_toggles_diagnostic_pin_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_multiple_calls);

    return UNITY_END();
}
