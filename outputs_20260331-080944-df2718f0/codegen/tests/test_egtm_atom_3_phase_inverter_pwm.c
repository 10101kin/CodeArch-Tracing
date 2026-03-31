#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for ISR and period callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);
/* Extern for runtime update function (declared in production header or here) */
extern void updateEgtmAtom3phInv(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON               (1e-4f)
#define UT_NUM_CHANNELS                (3)
#define UT_INIT_DUTY_U_PERCENT         (25.0f)
#define UT_INIT_DUTY_V_PERCENT         (50.0f)
#define UT_INIT_DUTY_W_PERCENT         (75.0f)
#define UT_STEP_PERCENT                (10.0f)
#define UT_PWM_FREQUENCY_HZ            (20000.0f)
#define UT_DEADTIME_RISING_US          (1.0f)
#define UT_DEADTIME_FALLING_US         (1.0f)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/******************************
 * GROUP 1 - init: enable guard / CMU setup
 ******************************/
void test_TC_G1_001_init_enables_and_configures_when_disabled(void)
{
    /* Arrange: EGTM disabled at entry */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: TC4xx EGTM APIs used and clocks configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called once when module is disabled");

    {
        unsigned int cc_getModFreq = mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount();
        TEST_ASSERT_TRUE_MESSAGE(cc_getModFreq >= 1u, "IfxEgtm_Cmu_getModuleFrequency must be called at least once");
    }
    {
        unsigned int cc_setGclkFreq = mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount();
        TEST_ASSERT_TRUE_MESSAGE(cc_setGclkFreq >= 1u, "IfxEgtm_Cmu_setGclkFrequency must be configured");
    }
    {
        unsigned int cc_setClkFreq = mock_IfxEgtm_Cmu_setClkFrequency_getCallCount();
        TEST_ASSERT_TRUE_MESSAGE(cc_setClkFreq >= 1u, "IfxEgtm_Cmu_setClkFrequency must be configured");
    }
    {
        unsigned int cc_enableClocks = mock_IfxEgtm_Cmu_enableClocks_getCallCount();
        TEST_ASSERT_TRUE_MESSAGE(cc_enableClocks >= 1u, "IfxEgtm_Cmu_enableClocks must be called when enabling EGTM");
    }
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange: EGTM already enabled at entry */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: enable is not called; still verifies target API was used */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must not be called when already enabled");
}

/******************************
 * GROUP 2 - init: configuration values
 ******************************/
void test_TC_G2_001_init_applies_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* keep focus on config values */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: numChannels and frequency set via unified IfxEgtm_Pwm init */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_initEgtmAtom3phInv_lastNumChannels, "Number of PWM channels must be 3 (complementary pairs handled internally)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_initEgtmAtom3phInv_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_applies_initial_duties_and_deadtime(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: initial duties in percent for phases U/V/W */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[0], "Initial duty for phase U must be 25.0% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[1], "Initial duty for phase V must be 50.0% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[2], "Initial duty for phase W must be 75.0% (percent units)");

    /* Assert: dead-time 1.0 us rising/falling for each logical channel */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_setDeadtimeEgtmAtom3phInv_lastDtRising[0], "Dead-time rising[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_setDeadtimeEgtmAtom3phInv_lastDtRising[1], "Dead-time rising[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_setDeadtimeEgtmAtom3phInv_lastDtRising[2], "Dead-time rising[2] must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_setDeadtimeEgtmAtom3phInv_lastDtFalling[0], "Dead-time falling[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_setDeadtimeEgtmAtom3phInv_lastDtFalling[1], "Dead-time falling[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_setDeadtimeEgtmAtom3phInv_lastDtFalling[2], "Dead-time falling[2] must be 1.0 us");
}

/******************************
 * GROUP 3 - runtime update logic
 ******************************/
void test_TC_G3_001_update_increments_duties_by_step_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: single periodic update */
    updateEgtmAtom3phInv();

    /* Assert: 25/50/75 + 10 = 35/60/85 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[0], "Phase U duty must increment by +10% to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[1], "Phase V duty must increment by +10% to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_updateEgtmAtom3phInv_lastDuties[2], "Phase W duty must increment by +10% to 85%");
}

void test_TC_G3_002_update_wraps_independently_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: two updates to reach U/V/W = 45/70/95 */
    updateEgtmAtom3phInv();
    updateEgtmAtom3phInv();

    /* Third update: U->55, V->80, W wraps: 95+10 >=100 -> 10 */
    updateEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_updateEgtmAtom3phInv_lastDuties[0], "Phase U duty must be 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_updateEgtmAtom3phInv_lastDuties[1], "Phase V duty must be 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_updateEgtmAtom3phInv_lastDuties[2], "Phase W duty must wrap to 10% after exceeding 100%");
}

/******************************
 * GROUP 4 - ISR / interrupt behavior
 ******************************/
void test_TC_G4_001_isr_toggles_led_once_per_call(void)
{
    /* Arrange */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Precondition: LED toggle count must start at 0");

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Arrange */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Precondition: LED toggle count must start at 0");

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "LED toggle count must accumulate across ISR calls");
}

void test_TC_G4_003_period_callback_has_no_side_effect_on_led(void)
{
    /* Arrange: generate one toggle via ISR */
    interruptEgtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "Precondition: one LED toggle via ISR");

    /* Act: invoke the period-event callback (should be side-effect free) */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert: LED toggle count unchanged by the callback */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "Period-event callback must not toggle LED");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_applies_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_applies_initial_duties_and_deadtime);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_increments_duties_by_step_without_wrap);
    RUN_TEST(test_TC_G3_002_update_wraps_independently_per_channel);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);
    RUN_TEST(test_TC_G4_003_period_callback_has_no_side_effect_on_led);

    return UNITY_END();
}
