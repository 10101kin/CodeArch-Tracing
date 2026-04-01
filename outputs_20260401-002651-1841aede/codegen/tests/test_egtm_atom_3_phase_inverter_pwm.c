#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and driver callback if not exposed by the production header */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_ISR_PRIORITY                  (20)
#define UT_TEST_MODULE_FREQ_HZ           (100000000u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
Group 1 - initEgtmAtom3phInv: initialization / enable guard
*/
void test_TC_G1_001_init_enables_cmu_when_disabled_and_sets_gclk_and_clk0_freq(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 0u; /* FALSE */
    mock_IfxEgtm_Cmu_getModuleFrequency_return_value = UT_TEST_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - CMU configured inside enable guard when module was disabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, "CLK0 index must be configured to 0 (CLK0)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_TEST_MODULE_FREQ_HZ, mock_IfxEgtm_Cmu_setClkFrequency_last_count, "CLK0 frequency count set from module frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_TEST_MODULE_FREQ_HZ, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "GCLK numerator set to module frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "GCLK denominator set to 1");
}

void test_TC_G1_002_init_skips_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u; /* TRUE */
    mock_IfxEgtm_Cmu_getModuleFrequency_return_value = UT_TEST_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - CMU setup stays untouched when already enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "GCLK frequency should not be configured when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_last_count, "CLK0 frequency should not be configured when EGTM already enabled");
}

/*
Group 2 - initEgtmAtom3phInv: configuration values
*/
void test_TC_G2_001_init_config_sets_three_channels_and_frequency_20k(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u; /* TRUE to isolate config verification */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - Verify PWM config applied via IfxEgtm_Pwm_init spy fields */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of PWM channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_config_sets_deadtime_1us_each_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - Dead-time per channel: rising=1us, falling=1us */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US, mock_IfxEgtm_Pwm_init_lastDtRising[0], "Phase U rising dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US, mock_IfxEgtm_Pwm_init_lastDtRising[1], "Phase V rising dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US, mock_IfxEgtm_Pwm_init_lastDtRising[2], "Phase W rising dead-time must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_init_lastDtFalling[0], "Phase U falling dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_init_lastDtFalling[1], "Phase V falling dead-time must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_init_lastDtFalling[2], "Phase W falling dead-time must be 1.0 us");
}

/*
Group 3 - initEgtmAtom3phInv: runtime update logic (state handoff to update)
*/
void test_TC_G3_001_after_init_first_update_steps_from_initial_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - first step from 25/50/75 by +10 each → 35/60/85 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U first step should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V first step should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W first step should be 85%");
}

void test_TC_G3_002_reinit_resets_state_and_first_update_steps_again(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(); /* advance once */

    /* Act - re-initialize to reset persistent state, then update again */
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty();

    /* Assert - after re-init, first step should again be 35/60/85 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U first step after re-init should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V first step after re-init should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W first step after re-init should be 85%");
}

/*
Group 4 - initEgtmAtom3phInv: ISR / interrupt behavior
*/
void test_TC_G4_001_isr_toggles_led_pin_matches_configured_led_pin(void)
{
    /* Arrange: init configures LED as output after PWM init */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    /* Capture configured LED pin from last port output config */
    unsigned int configuredLedPin = mock_IfxPort_setPinModeOutput_last_pinIndex;

    /* Act: call ISR */
    interruptEgtmAtom();

    /* Assert: ISR toggles the same LED pin that was configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(configuredLedPin, mock_IfxPort_togglePin_last_pinIndex, "ISR should toggle the configured LED pin");
}

void test_TC_G4_002_isr_multiple_invocations_keep_same_led_pin(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    unsigned int configuredLedPin = mock_IfxPort_setPinModeOutput_last_pinIndex;

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(configuredLedPin, mock_IfxPort_togglePin_last_pinIndex, "ISR should consistently toggle the same LED pin across calls");
}

/*
Group 5 - updateEgtmAtom3phInvDuty: configuration values (observed via first updates)
*/
void test_TC_G5_001_update_first_step_applies_all_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after first update is 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after first update is 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after first update is 85%");
}

void test_TC_G5_002_update_two_steps_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* step 1 */
    updateEgtmAtom3phInvDuty(); /* step 2 */

    /* Assert: 45/70/95 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after two updates is 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after two updates is 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after two updates is 95%");
}

/*
Group 6 - updateEgtmAtom3phInvDuty: runtime update logic
*/
void test_TC_G6_001_update_wraps_individually_when_exceeding_100(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();

    /* Act: 3 updates — W should wrap on 3rd */
    updateEgtmAtom3phInvDuty(); /* -> 35/60/85 */
    updateEgtmAtom3phInvDuty(); /* -> 45/70/95 */
    updateEgtmAtom3phInvDuty(); /* -> 55/80/10 (wrap W) */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 3 updates is 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 3 updates is 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W wraps to 10% after 3 updates");
}

void test_TC_G6_002_update_independent_wrap_points_across_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();

    /* Act: 5 updates — expect V and W wrapped, U not yet */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */
    updateEgtmAtom3phInvDuty(); /* 3 */
    updateEgtmAtom3phInvDuty(); /* 4 */
    updateEgtmAtom3phInvDuty(); /* 5 -> 75/10/30 */

    /* Assert at 5 updates */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 5 updates is 75%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V wraps to 10% after 5 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W is 30% after 5 updates");

    /* Act more: to 8 updates — expect U wraps on 8th */
    updateEgtmAtom3phInvDuty(); /* 6 */
    updateEgtmAtom3phInvDuty(); /* 7 */
    updateEgtmAtom3phInvDuty(); /* 8 -> 10/40/60 */

    /* Assert at 8 updates */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U wraps to 10% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V at 8 updates is 40%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W at 8 updates is 60%");
}

/*
Group 7 - interruptEgtmAtom: ISR / interrupt behavior
*/
void test_TC_G7_001_interruptEgtmAtom_toggles_led_pin_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    unsigned int configuredLedPin = mock_IfxPort_setPinModeOutput_last_pinIndex;

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(configuredLedPin, mock_IfxPort_togglePin_last_pinIndex, "interruptEgtmAtom should toggle LED P03.9 (configured LED pin)");
}

void test_TC_G7_002_interruptEgtmAtom_called_twice_keeps_same_pin(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    unsigned int configuredLedPin = mock_IfxPort_setPinModeOutput_last_pinIndex;

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(configuredLedPin, mock_IfxPort_togglePin_last_pinIndex, "LED pin index remains constant across ISR invocations");
}

/*
Group 8 - IfxEgtm_periodEventFunction: configuration values (no side effects)
*/
void test_TC_G8_001_periodEventFunction_does_nothing_to_led_toggle(void)
{
    /* Arrange: set a known toggle state via ISR then call the callback */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    interruptEgtmAtom();
    unsigned int priorTogglePin = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert: no change to toggle state */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorTogglePin, mock_IfxPort_togglePin_last_pinIndex, "Period event callback must not toggle LED");
}

void test_TC_G8_002_periodEventFunction_does_not_change_pwm_state_or_duties(void)
{
    /* Arrange: init and perform one update */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty();

    float priorU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float priorV = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float priorW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert: callback should not change duty snapshot in mock */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, priorU, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Callback should not affect Phase U duty spy");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, priorV, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Callback should not affect Phase V duty spy");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, priorW, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Callback should not affect Phase W duty spy");
}

/*
Group 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (verify empty)
*/
void test_TC_G9_001_periodEventFunction_safe_with_null_and_nonnull(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = 1u;
    initEgtmAtom3phInv();
    interruptEgtmAtom();
    unsigned int priorTogglePin = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction((void*)0x1234);

    /* Assert: still no toggle change */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorTogglePin, mock_IfxPort_togglePin_last_pinIndex, "Period callback must not toggle LED for NULL or non-NULL data");
}

void test_TC_G9_002_periodEventFunction_no_side_effects_on_clocks(void)
{
    /* Arrange */
    unsigned int priorClkIdx = mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex;
    unsigned int priorClkCnt = mock_IfxEgtm_Cmu_setClkFrequency_last_count;
    unsigned int priorGclkNum = mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator;
    unsigned int priorGclkDen = mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert: period callback should not touch CMU settings */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorClkIdx, mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, "Callback must not change CMU CLK index");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorClkCnt, mock_IfxEgtm_Cmu_setClkFrequency_last_count, "Callback must not change CMU CLK count");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorGclkNum, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "Callback must not change GCLK numerator");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(priorGclkDen, mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "Callback must not change GCLK denominator");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_cmu_when_disabled_and_sets_gclk_and_clk0_freq);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_config_sets_three_channels_and_frequency_20k);
    RUN_TEST(test_TC_G2_002_init_config_sets_deadtime_1us_each_channel);

    RUN_TEST(test_TC_G3_001_after_init_first_update_steps_from_initial_duties);
    RUN_TEST(test_TC_G3_002_reinit_resets_state_and_first_update_steps_again);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_pin_matches_configured_led_pin);
    RUN_TEST(test_TC_G4_002_isr_multiple_invocations_keep_same_led_pin);

    RUN_TEST(test_TC_G5_001_update_first_step_applies_all_channels);
    RUN_TEST(test_TC_G5_002_update_two_steps_without_wrap);

    RUN_TEST(test_TC_G6_001_update_wraps_individually_when_exceeding_100);
    RUN_TEST(test_TC_G6_002_update_independent_wrap_points_across_channels);

    RUN_TEST(test_TC_G7_001_interruptEgtmAtom_toggles_led_pin_after_init);
    RUN_TEST(test_TC_G7_002_interruptEgtmAtom_called_twice_keeps_same_pin);

    RUN_TEST(test_TC_G8_001_periodEventFunction_does_nothing_to_led_toggle);
    RUN_TEST(test_TC_G8_002_periodEventFunction_does_not_change_pwm_state_or_duties);

    RUN_TEST(test_TC_G9_001_periodEventFunction_safe_with_null_and_nonnull);
    RUN_TEST(test_TC_G9_002_periodEventFunction_no_side_effects_on_clocks);

    return UNITY_END();
}
