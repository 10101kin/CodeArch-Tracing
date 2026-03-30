#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for internal ISR/callback symbols defined in production .c */
extern void IfxEgtm_periodEventFunction(void *data);
extern void interruptEgtmAtom(void);

/* UT constants derived from configuration */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 */
void test_TC_G1_001_init_calls_target_apis_when_disabled(void)
{
    /* Arrange: eGTM disabled, expect enable + CMU setup */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000.0f; /* example system clock */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - TC4xx API usage and enable path taken */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_isEnabled_getCallCount(),       "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_enable_getCallCount(),          "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "CMU GCLK frequency must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CMU CLK0 frequency must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_init_getCallCount(),        "IfxEgtm_Pwm_init must be called exactly once");
}

void test_TC_G1_002_init_skips_cmu_when_already_enabled(void)
{
    /* Arrange: eGTM already enabled, CMU configuration should be skipped */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - enable path skipped, still using TC4xx APIs */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_isEnabled_getCallCount(),       "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_enable_getCallCount(),          "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must NOT be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "CMU GCLK frequency must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CMU CLK0 frequency must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must NOT be enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxEgtm_Pwm_init_getCallCount(),        "IfxEgtm_Pwm_init must be called exactly once");
}

/*
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 */
void test_TC_G2_001_init_applies_expected_frequency_and_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - PWM frequency and number of logical channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3 (U,V,W)");
}

void test_TC_G2_002_init_sets_initial_duty_and_deadtime(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - initial duties in PERCENT and dead-times in microseconds */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Initial duty for phase U must be 25% (percent, not fraction)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Initial duty for phase V must be 50% (percent, not fraction)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Initial duty for phase W must be 75% (percent, not fraction)");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[0], "Rising dead-time for channel 0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[1], "Rising dead-time for channel 1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[2], "Rising dead-time for channel 2 must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[0], "Falling dead-time for channel 0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[1], "Falling dead-time for channel 1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[2], "Falling dead-time for channel 2 must be 1.0 us");
}

/*
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (via period-event callback)
 * Note: The period-event callback updates duties; it must NOT toggle the LED.
 */
void test_TC_G3_001_period_event_increments_duties_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: one period event increases each duty by +10% */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert: duties updated to 35, 60, 85 percent */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by +10% to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by +10% to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by +10% to 85%");
}

void test_TC_G3_002_period_event_wraps_duty_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: call 3 times so W wraps once, others do not */
    IfxEgtm_periodEventFunction(NULL); /* U:35 V:60 W:85 */
    IfxEgtm_periodEventFunction(NULL); /* U:45 V:70 W:95 */
    IfxEgtm_periodEventFunction(NULL); /* U:55 V:80 W:10 (wrap to STEP) */

    /* Assert: wrap semantics and independence across channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55% after 3 updates (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80% after 3 updates (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to STEP (10%) after 3 updates");
}

/*
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior
 */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert: ISR toggles the LED exactly once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_togglePin_callCount, "ISR must toggle LED once per interrupt");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert: toggle count accumulates */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_calls_target_apis_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_applies_expected_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_init_sets_initial_duty_and_deadtime);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_period_event_increments_duties_without_wrap);
    RUN_TEST(test_TC_G3_002_period_event_wraps_duty_independently);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    return UNITY_END();
}
