#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_NUM_CHANNELS                 (3)
#define UT_INIT_DUTY_U_PERCENT          (25.0f)
#define UT_INIT_DUTY_V_PERCENT          (50.0f)
#define UT_INIT_DUTY_W_PERCENT          (75.0f)
#define UT_STEP_PERCENT                 (10.0f)
#define UT_PWM_FREQUENCY_HZ             (20000.0f)
#define UT_DT_RISING_US                 (1.0f)
#define UT_DT_FALLING_US                (1.0f)
#define UT_TEST_EGTM_MODULE_FREQ_HZ     (300000000u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 */
void test_TC_01_001_init_enables_cmu_when_egtm_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = FALSE; /* EGTM disabled to exercise enable guard */
    mock_IfxEgtm_Cmu_getModuleFrequency_return_value = UT_TEST_EGTM_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: CMU clock configuration performed using the module frequency */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_TEST_EGTM_MODULE_FREQ_HZ, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "GCLK numerator should match module frequency when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "GCLK denominator should be 1 when directly using module frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, "CLK index should be 0 (CLK0)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_TEST_EGTM_MODULE_FREQ_HZ, mock_IfxEgtm_Cmu_setClkFrequency_last_count, "CLK0 frequency/count should match module frequency");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Cmu_enableClocks_last_clkMask != 0u, "Expected FXCLK0 and DTM CLK0 to be enabled (clk mask non-zero)");

    /* Assert: PWM init applied with expected app-level values */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "Number of channels should be 3 for 3-phase complementary PWM");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_initEgtmAtom3phInv_lastFrequency, "PWM switching frequency should be 20 kHz");
}

void test_TC_01_002_init_skips_cmu_when_egtm_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE; /* EGTM already enabled: CMU actions must be skipped */
    mock_IfxEgtm_Cmu_getModuleFrequency_return_value = UT_TEST_EGTM_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: CMU configuration should be skipped (remain at reset defaults) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "GCLK numerator should remain 0 when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "GCLK denominator should remain 0 when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, "CLK index should remain 0 at reset default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_last_count, "CLK count should remain 0 at reset default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_last_clkMask, "Clock enable mask should remain 0 when EGTM already enabled");

    /* But PWM init must still occur and capture app-config values */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "PWM init should still configure 3 channels even if EGTM already enabled");
}

/*
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 */
void test_TC_02_001_init_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE; /* CMU path irrelevant for this test */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "init: numChannels must be 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_initEgtmAtom3phInv_lastFrequency, "init: frequency must be 20 kHz");
}

void test_TC_02_002_init_sets_deadtime_rising_falling(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert dead-time configuration for all three channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_initEgtmAtom3phInv_lastDtRising[0], "DT rising for phase U must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_initEgtmAtom3phInv_lastDtRising[1], "DT rising for phase V must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US, mock_initEgtmAtom3phInv_lastDtRising[2], "DT rising for phase W must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[0], "DT falling for phase U must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[1], "DT falling for phase V must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[2], "DT falling for phase W must be 1.0 us");
}

/*
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (validated through first update)
 */
void test_TC_03_001_first_update_increments_all_duties_from_initial(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act: one update step */
    updateEgtmAtom3phInvDuty();

    /* Assert: duties in percent after +10% step */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "Phase U duty should be 35.0% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "Phase V duty should be 60.0% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "Phase W duty should be 85.0% after first update");
}

void test_TC_03_002_three_updates_wraps_W_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act: three updates: U=25->35->45->55; V=50->60->70->80; W=75->85->95->wrap to 10 */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert: independent wrap behavior */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "Phase U should be 55.0% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "Phase V should be 80.0% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "Phase W should wrap to 10.0% after 3 updates");
}

/*
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (invoke ISR after init)
 */
void test_TC_04_001_isr_toggles_led_pin_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    interruptEgtmAtom();

    /* Assert: toggle pin index should change from pre-ISR value */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_togglePin_last_pinIndex != before, "ISR should call IfxPort_togglePin on LED pin (pin index should change)");
}

void test_TC_04_002_multiple_isr_calls_leave_last_pin_index_constant(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    unsigned int afterFirst = mock_IfxPort_togglePin_last_pinIndex;
    interruptEgtmAtom();

    /* Assert: subsequent ISR calls toggle the same LED pin index (value remains that pin) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(afterFirst, mock_IfxPort_togglePin_last_pinIndex, "ISR should consistently toggle the configured LED pin");
}

/*
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 */
void test_TC_05_001_update_preserves_init_num_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: init configuration stays intact */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "update: numChannels should remain 3 after updates");
}

void test_TC_05_002_update_does_not_change_pwm_frequency_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    float freqBefore = mock_initEgtmAtom3phInv_lastFrequency;

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: PWM switching frequency is an init-time parameter and must be unchanged */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freqBefore, mock_initEgtmAtom3phInv_lastFrequency, "update: PWM frequency config should not be altered by runtime duty updates");
}

/*
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 */
void test_TC_06_001_duty_increment_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: all duties below 100 after one step */
    TEST_ASSERT_TRUE_MESSAGE(mock_updateEgtmAtom3phInvDuty_lastDuties[0] < 100.0f, "Phase U duty should be below 100% after one update");
    TEST_ASSERT_TRUE_MESSAGE(mock_updateEgtmAtom3phInvDuty_lastDuties[1] < 100.0f, "Phase V duty should be below 100% after one update");
    TEST_ASSERT_TRUE_MESSAGE(mock_updateEgtmAtom3phInvDuty_lastDuties[2] < 100.0f, "Phase W duty should be below 100% after one update");
}

void test_TC_06_002_wraparound_occurs_independently_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act: 3 updates cause W to wrap, U/V do not */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U independent of W wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V independent of W wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W wraps to STEP (10.0%)");
}

/*
 * GROUP 7 - interruptEgtmAtom: ISR / interrupt behavior
 */
void test_TC_07_001_interruptEgtmAtom_toggles_led_pin(void)
{
    /* Arrange: no init required for ISR semantics */
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_togglePin_last_pinIndex != before, "ISR should toggle LED via IfxPort_togglePin");
}

void test_TC_07_002_interruptEgtmAtom_multiple_calls_no_side_effects_on_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    unsigned int idxBefore = mock_IfxPort_togglePin_last_pinIndex;
    interruptEgtmAtom();
    unsigned int idxAfterFirst = mock_IfxPort_togglePin_last_pinIndex;
    interruptEgtmAtom();

    /* Assert: ISR calls do not modify PWM config spies */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "ISR should not modify PWM numChannels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_initEgtmAtom3phInv_lastFrequency, "ISR should not modify PWM frequency");
    TEST_ASSERT_TRUE_MESSAGE(idxAfterFirst != idxBefore, "First ISR should change toggle pin index from pre-ISR value");
}

/*
 * GROUP 8 - IfxEgtm_periodEventFunction: configuration values
 */
void test_TC_08_001_period_callback_has_no_effect_on_led_toggle(void)
{
    /* Arrange */
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxPort_togglePin_last_pinIndex, "Period callback must not toggle LED pin");
}

void test_TC_08_002_period_callback_does_not_alter_pwm_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    int numBefore = mock_initEgtmAtom3phInv_lastNumChannels;
    float freqBefore = mock_initEgtmAtom3phInv_lastFrequency;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(numBefore, mock_initEgtmAtom3phInv_lastNumChannels, "Period callback should not change numChannels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freqBefore, mock_initEgtmAtom3phInv_lastFrequency, "Period callback should not change PWM frequency");
}

/*
 * GROUP 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (should be empty)
 */
void test_TC_09_001_period_callback_no_side_effect_on_duty_array(void)
{
    /* Arrange: call callback without any init/update; duty spy should remain at reset (0s) */

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert: expect reset defaults (0.0f) as callback is empty */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "Period callback must not alter phase U duty at reset");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "Period callback must not alter phase V duty at reset");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "Period callback must not alter phase W duty at reset");
}

void test_TC_09_002_period_callback_multiple_invocations_no_state_change(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(); /* set known, non-zero duties */
    float u_before = mock_updateEgtmAtom3phInvDuty_lastDuties[0];
    float v_before = mock_updateEgtmAtom3phInvDuty_lastDuties[1];
    float w_before = mock_updateEgtmAtom3phInvDuty_lastDuties[2];

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert: callback should not alter runtime duty state */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, u_before, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "Period callback must not change phase U duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, v_before, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "Period callback must not change phase V duty");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, w_before, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "Period callback must not change phase W duty");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_01_001_init_enables_cmu_when_egtm_disabled);
    RUN_TEST(test_TC_01_002_init_skips_cmu_when_egtm_enabled);

    RUN_TEST(test_TC_02_001_init_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_02_002_init_sets_deadtime_rising_falling);

    RUN_TEST(test_TC_03_001_first_update_increments_all_duties_from_initial);
    RUN_TEST(test_TC_03_002_three_updates_wraps_W_only);

    RUN_TEST(test_TC_04_001_isr_toggles_led_pin_once);
    RUN_TEST(test_TC_04_002_multiple_isr_calls_leave_last_pin_index_constant);

    RUN_TEST(test_TC_05_001_update_preserves_init_num_channels);
    RUN_TEST(test_TC_05_002_update_does_not_change_pwm_frequency_config);

    RUN_TEST(test_TC_06_001_duty_increment_below_boundary);
    RUN_TEST(test_TC_06_002_wraparound_occurs_independently_per_channel);

    RUN_TEST(test_TC_07_001_interruptEgtmAtom_toggles_led_pin);
    RUN_TEST(test_TC_07_002_interruptEgtmAtom_multiple_calls_no_side_effects_on_config);

    RUN_TEST(test_TC_08_001_period_callback_has_no_effect_on_led_toggle);
    RUN_TEST(test_TC_08_002_period_callback_does_not_alter_pwm_config);

    RUN_TEST(test_TC_09_001_period_callback_no_side_effect_on_duty_array);
    RUN_TEST(test_TC_09_002_period_callback_multiple_invocations_no_state_change);

    return UNITY_END();
}
