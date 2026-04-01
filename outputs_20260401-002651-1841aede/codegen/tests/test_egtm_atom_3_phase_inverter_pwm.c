#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and callback implemented in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_DT_RISING_US                  (1.0f)
#define UT_DT_FALLING_US                 (1.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init guard
 **********************/
void test_TC_G1_001_init_enables_clocks_when_module_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = FALSE; /* EGTM disabled initially */
    mock_IfxEgtm_Cmu_getModuleFrequency_return_value = 100000000U; /* 100 MHz */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: CMU setup performed using the dynamic module frequency */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(100000000U, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, "GCLK numerator should equal module frequency when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U,           mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "GCLK denominator should be 1");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U,           mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, "Sub-clock index should be 0 for CLK0");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(100000000U,   mock_IfxEgtm_Cmu_setClkFrequency_last_count, "Sub-clock count should equal module frequency when enabling EGTM");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Cmu_enableClocks_last_clkMask != 0U, "Expected enableClocks to be called with a non-zero clock mask (FXCLK0 and DTM CLK0)");
}

void test_TC_G1_002_init_skips_cmu_config_when_module_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE; /* EGTM already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: CMU setup remains untouched when already enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator,   "GCLK numerator should remain default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, "GCLK denominator should remain default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex,     "CLK index should remain default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setClkFrequency_last_count,        "CLK count should remain default when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_enableClocks_last_clkMask,         "Clock enable mask should remain 0 when EGTM already enabled");
}

/***********************************
 * GROUP 2 - init configuration values
 ***********************************/
void test_TC_G2_001_init_sets_expected_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_initEgtmAtom3phInv_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_sets_deadtime_rise_fall_to_1us_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: Dead-time applied equally on all three channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US,  mock_initEgtmAtom3phInv_lastDtRising[0], "DT rising channel 0 = 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[0], "DT falling channel 0 = 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US,  mock_initEgtmAtom3phInv_lastDtRising[1], "DT rising channel 1 = 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[1], "DT falling channel 1 = 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_RISING_US,  mock_initEgtmAtom3phInv_lastDtRising[2], "DT rising channel 2 = 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DT_FALLING_US, mock_initEgtmAtom3phInv_lastDtFalling[2], "DT falling channel 2 = 1.0 us");
}

/*****************************************
 * GROUP 3 - initEgtmAtom3phInv: update logic linkage
 *****************************************/
void test_TC_G3_001_first_update_increments_duty_below_wrap_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: expected one-step increase from initial duties */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U duty should increase by step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V duty should increase by step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W duty should increase by step to 85%");
}

void test_TC_G3_002_update_wraps_only_channels_reaching_100(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Drive W close to boundary: from 75 -> 95 with two updates */
    updateEgtmAtom3phInvDuty(); /* U:35 V:60 W:85 */
    updateEgtmAtom3phInvDuty(); /* U:45 V:70 W:95 */

    /* Next update: W wraps to 10, others increment normally */
    updateEgtmAtom3phInvDuty(); /* U:55 V:80 W:10 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U increments normally below wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V increments normally below wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W wraps to step value 10% at boundary");
}

/***********************************
 * GROUP 4 - ISR / interrupt behavior (from init context)
 ***********************************/
void test_TC_G4_001_isr_toggles_led_pin_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert: toggled pin should be the LED pin configured in init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(mock_IfxPort_setPinModeOutput_last_pinIndex,
                                     mock_IfxPort_togglePin_last_pinIndex,
                                     "ISR must toggle the same LED pin configured by init");
}

void test_TC_G4_002_isr_toggles_led_pin_consistently_across_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert: same pin index remains targeted across calls */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(mock_IfxPort_setPinModeOutput_last_pinIndex,
                                     mock_IfxPort_togglePin_last_pinIndex,
                                     "ISR must consistently toggle the LED pin on repeated calls");
}

/*******************************************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 *******************************************/
void test_TC_G5_001_update_reports_duty_in_percent_units(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: duties are expressed in percent, not fraction */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U=35% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V=60% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W=85% after first update");
}

void test_TC_G5_002_update_operates_on_three_logical_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: driver configured for 3 logical channels and all updated */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "Update should operate on 3 logical channels (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "Phase U updated");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "Phase V updated");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "Phase W updated");
}

/*******************************************
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 *******************************************/
void test_TC_G6_001_multiple_updates_cause_expected_wrap_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 8 updates */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 steps: U=10, V=40, W=60 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U wraps to 10% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V at 40% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W at 60% after 8 steps");
}

void test_TC_G6_002_per_channel_wrap_occurs_at_individual_times(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 5 updates */
    for (int i = 0; i < 5; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 5 steps: U=75, V=10 (wrap at step 5), W=30 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[0], "U at 75% after 5 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[1], "V wraps to 10% at its boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_updateEgtmAtom3phInvDuty_lastDuties[2], "W at 30% after 5 steps");
}

/***********************************
 * GROUP 7 - interruptEgtmAtom: ISR behavior
 ***********************************/
void test_TC_G7_001_interruptEgtmAtom_toggles_led_pin_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(mock_IfxPort_setPinModeOutput_last_pinIndex,
                                     mock_IfxPort_togglePin_last_pinIndex,
                                     "interruptEgtmAtom must toggle LED P03.9 pin");
}

void test_TC_G7_002_interruptEgtmAtom_repeated_calls_target_same_pin(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert: always the LED pin */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(mock_IfxPort_setPinModeOutput_last_pinIndex,
                                     mock_IfxPort_togglePin_last_pinIndex,
                                     "ISR must keep toggling the configured LED pin");
}

/********************************************
 * GROUP 8 - IfxEgtm_periodEventFunction: configuration values (no-op)
 ********************************************/
void test_TC_G8_001_period_callback_is_noop_on_null_data(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction((void *)0);

    /* Assert: no GPIO toggle side effect from period callback */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxPort_togglePin_last_pinIndex, "Period callback must have no side effects");
}

void test_TC_G8_002_period_callback_is_noop_on_nonnull_data(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;
    int dummy = 123;

    /* Act */
    IfxEgtm_periodEventFunction(&dummy);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxPort_togglePin_last_pinIndex, "Period callback must not toggle LED even with non-NULL data");
}

/********************************************
 * GROUP 9 - IfxEgtm_periodEventFunction: ISR behavior (should be none)
 ********************************************/
void test_TC_G9_001_period_callback_does_not_toggle_led(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxPort_togglePin_last_pinIndex, "Period callback must not toggle LED");
}

void test_TC_G9_002_multiple_period_callbacks_have_no_side_effects(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_return_value = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxPort_togglePin_last_pinIndex;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxPort_togglePin_last_pinIndex, "Multiple period callbacks must have no side effects");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_clocks_when_module_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_config_when_module_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_expected_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_sets_deadtime_rise_fall_to_1us_per_channel);

    RUN_TEST(test_TC_G3_001_first_update_increments_duty_below_wrap_boundary);
    RUN_TEST(test_TC_G3_002_update_wraps_only_channels_reaching_100);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_pin_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_pin_consistently_across_calls);

    RUN_TEST(test_TC_G5_001_update_reports_duty_in_percent_units);
    RUN_TEST(test_TC_G5_002_update_operates_on_three_logical_channels);

    RUN_TEST(test_TC_G6_001_multiple_updates_cause_expected_wrap_values);
    RUN_TEST(test_TC_G6_002_per_channel_wrap_occurs_at_individual_times);

    RUN_TEST(test_TC_G7_001_interruptEgtmAtom_toggles_led_pin_after_init);
    RUN_TEST(test_TC_G7_002_interruptEgtmAtom_repeated_calls_target_same_pin);

    RUN_TEST(test_TC_G8_001_period_callback_is_noop_on_null_data);
    RUN_TEST(test_TC_G8_002_period_callback_is_noop_on_nonnull_data);

    RUN_TEST(test_TC_G9_001_period_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G9_002_multiple_period_callbacks_have_no_side_effects);

    return UNITY_END();
}
