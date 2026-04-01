#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for ISR and driver callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_TEST_MODULE_FREQ_HZ           (300000000U)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* =============================
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 * ============================= */
void test_TC_G1_001_init_enables_peripherals_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_TEST_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled() must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable() must be called when EGTM is disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be queried once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must be configured once inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "Sub-clock (CLK0) frequency must be configured once inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Required CMU clocks (FXCLK0/DTM0) must be enabled once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig() must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init() must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output exactly once after PWM init");
}

void test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_TEST_MODULE_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled() must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable() must NOT be called when EGTM is already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU configuration must be skipped when EGTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK configuration must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 configuration must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Clock enabling must be skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig() must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init() must still be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output exactly once after PWM init");
}

/* =============================
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 * ============================= */
void test_TC_G2_001_init_configures_three_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* keep CMU quiet to focus on config */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_configures_led_pin_output_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED output configuration must occur exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty updates should occur during init phase");
}

/* =============================
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (verified via update API behavior)
 * ============================= */
void test_TC_G3_001_first_update_increments_all_duties_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One multi-channel duty update expected");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must step by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must step by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must step by +10% on first update");
}

void test_TC_G3_002_wrap_occurs_independently_per_channel_after_three_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1: 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* 2: 45,70,95 */
    updateEgtmAtom3phInvDuty(); /* 3: 55,80,10 (wrap on W only) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three multi-channel duty updates expected");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 3 updates must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 3 updates must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 10% on 3rd update");
}

/* =============================
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (validate ISR separately)
 * ============================= */
void test_TC_G4_001_isr_toggle_led_once(void)
{
    /* Arrange */
    /* No init required to validate ISR toggling behavior */

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggle_led_accumulates(void)
{
    /* Arrange */

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle count must accumulate across multiple invocations");
}

/* =============================
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 * ============================= */
void test_TC_G5_001_update_does_not_change_channel_count_or_frequency_from_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Channel count reported by init spy must remain 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Init spy must retain configured 20 kHz frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one duty update must be issued by update() call");
}

void test_TC_G5_002_update_multiple_calls_increase_hal_call_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() invocation");
}

/* =============================
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 * ============================= */
void test_TC_G6_001_update_wraps_independently_after_three_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */
    updateEgtmAtom3phInvDuty(); /* 3 */

    /* Assert (same as G3_002 to reinforce independent wrap behavior) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 10% after 3 updates");
}

void test_TC_G6_002_update_after_nine_calls_expected_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 9; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* U: 25 -> 20 after 9 updates; V: 50 -> 50; W: 75 -> 70 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(9, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Nine HAL duty updates expected after 9 calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 20% after 9 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 50% after 9 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 70% after 9 updates");
}

/* =============================
 * GROUP 7 - interruptEgtmAtom: ISR / interrupt behavior
 * ============================= */
void test_TC_G7_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G7_002_isr_toggles_led_twice(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must toggle LED twice after two invocations");
}

/* =============================
 * GROUP 8 - IfxEgtm_periodEventFunction: configuration values (no side effects)
 * ============================= */
void test_TC_G8_001_period_callback_has_no_side_effects_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t toggle_before = mock_togglePin_callCount;
    uint32_t dutyUpdates_before = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_callCount, "Period callback must NOT toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(dutyUpdates_before, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must NOT trigger duty updates");
}

void test_TC_G8_002_period_callback_repeated_calls_no_side_effects(void)
{
    /* Arrange */
    uint32_t toggle_before = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_callCount, "Repeated period callbacks must NOT toggle LED");
}

/* =============================
 * GROUP 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (remains empty)
 * ============================= */
void test_TC_G9_001_period_callback_does_not_influence_init_spies(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    uint32_t numCh_before = mock_IfxEgtm_Pwm_init_lastNumChannels;
    float freq_before = mock_IfxEgtm_Pwm_init_lastFrequency;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(numCh_before, mock_IfxEgtm_Pwm_init_lastNumChannels, "Callback must not change configured channel count");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freq_before, mock_IfxEgtm_Pwm_init_lastFrequency, "Callback must not change configured PWM frequency");
}

void test_TC_G9_002_period_callback_safe_with_null_and_has_no_toggle(void)
{
    /* Arrange */
    uint32_t toggle_before = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_callCount, "Callback with NULL data must have no side effects");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_peripherals_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_configures_three_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_configures_led_pin_output_once);

    RUN_TEST(test_TC_G3_001_first_update_increments_all_duties_by_step);
    RUN_TEST(test_TC_G3_002_wrap_occurs_independently_per_channel_after_three_updates);

    RUN_TEST(test_TC_G4_001_isr_toggle_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_led_accumulates);

    RUN_TEST(test_TC_G5_001_update_does_not_change_channel_count_or_frequency_from_init);
    RUN_TEST(test_TC_G5_002_update_multiple_calls_increase_hal_call_count);

    RUN_TEST(test_TC_G6_001_update_wraps_independently_after_three_calls);
    RUN_TEST(test_TC_G6_002_update_after_nine_calls_expected_values);

    RUN_TEST(test_TC_G7_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G7_002_isr_toggles_led_twice);

    RUN_TEST(test_TC_G8_001_period_callback_has_no_side_effects_once);
    RUN_TEST(test_TC_G8_002_period_callback_repeated_calls_no_side_effects);

    RUN_TEST(test_TC_G9_001_period_callback_does_not_influence_init_spies);
    RUN_TEST(test_TC_G9_002_period_callback_safe_with_null_and_has_no_toggle);

    return UNITY_END();
}
