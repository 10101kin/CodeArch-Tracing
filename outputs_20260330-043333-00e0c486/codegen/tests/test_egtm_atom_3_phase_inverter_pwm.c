#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                       (1e-4f)
#define UT_NUM_CHANNELS                        (3)
#define UT_PWM_FREQUENCY_HZ                    (20000.0f)
#define UT_INIT_DUTY_U_PERCENT                 (25.0f)
#define UT_INIT_DUTY_V_PERCENT                 (50.0f)
#define UT_INIT_DUTY_W_PERCENT                 (75.0f)
#define UT_STEP_PERCENT                        (10.0f)
#define UT_DEADTIME_RISING_US                  (1.0f)
#define UT_DEADTIME_FALLING_US                 (1.0f)
#define UT_EGTM_MODULE_CLOCK_HZ                (100000000U)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* -------------------------------------------------------------------------- */
/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard                */
/* -------------------------------------------------------------------------- */
void test_TC_G1_001_init_enabled_skips_enable_and_clock_config(void)
{
    /* Arrange: EGTM already enabled */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: target TC4xx APIs are used and called expected times */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "If EGTM is already enabled, IfxEgtm_enable must not be called");

    /* CMU configuration should not run when module already enabled (guarded path) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must not be called when EGTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must not be called when EGTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must not be called when EGTM is already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must not be called when EGTM is already enabled");

    /* Driver init and LED pin configuration must occur */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured once as output");
}

void test_TC_G1_002_init_disabled_enables_module_and_configures_clocks(void)
{
    /* Arrange: EGTM disabled so clock path must run */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_MODULE_CLOCK_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: enable guard path exercised */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when EGTM is disabled");

    /* CMU must be configured when enabling */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called once on enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called once on enable path");

    /* Driver init called */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
}

/* -------------------------------------------------------------------------- */
/* GROUP 2 - initEgtmAtom3phInv: configuration values                         */
/* -------------------------------------------------------------------------- */
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;  /* focus on config values */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert using init() spies (application values) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase complementary PWM");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_configures_led_pin_output_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as push-pull output exactly once at init");
}

/* -------------------------------------------------------------------------- */
/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (via update API)        */
/* -------------------------------------------------------------------------- */
void test_TC_G3_001_update_increments_duties_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: one step update */
    updateEgtmAtom3phInvDuty();

    /* Assert: HAL called once per update and duties in percent */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call unified driver exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must increment by 10% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must increment by 10% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must increment by 10% (percent units)");
}

void test_TC_G3_002_update_wraps_each_channel_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: three updates to cause W to wrap (75->85->95->10) */
    updateEgtmAtom3phInvDuty(); /* 25->35, 50->60, 75->85 */
    updateEgtmAtom3phInvDuty(); /* 35->45, 60->70, 85->95 */
    updateEgtmAtom3phInvDuty(); /* 45->55, 70->80, 95->10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3 steps must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3 steps must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must wrap to 10% after crossing 100%");
}

/* -------------------------------------------------------------------------- */
/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior                     */
/* -------------------------------------------------------------------------- */
void test_TC_G4_001_isr_toggle_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once per call");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR must accumulate LED toggles across calls");
}

/* -------------------------------------------------------------------------- */
/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values                   */
/* -------------------------------------------------------------------------- */
void test_TC_G5_001_update_called_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each call to update must invoke one HAL update");
}

void test_TC_G5_002_update_operates_on_three_logical_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: Confirm 3-channel configuration is retained and duties array covers 3 entries */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Unified driver must be initialized with 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Third channel (W) duty must be updated in percent");
}

/* -------------------------------------------------------------------------- */
/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic                   */
/* -------------------------------------------------------------------------- */
void test_TC_G6_001_update_two_steps_expected_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: two updates */
    updateEgtmAtom3phInvDuty(); /* step 1 */
    updateEgtmAtom3phInvDuty(); /* step 2 */

    /* Assert: U=45, V=70, W=95 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must invoke two HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 2 steps must be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 2 steps must be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 2 steps must be 95%");
}

void test_TC_G6_002_update_ten_steps_return_to_initial_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: ten updates cycle duties back to initial values */
    for (int i = 0; i < 10; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert: After 10 steps, duties must equal initial (modulo 100 with step=10) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Ten updates must invoke ten HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must return to initial after 10 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must return to initial after 10 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must return to initial after 10 steps");
}

/* -------------------------------------------------------------------------- */
/* GROUP 7 - interruptEgtmAtomPeriod: initialization / enable guard           */
/* -------------------------------------------------------------------------- */
void test_TC_G7_001_isr_does_not_trigger_pwm_update_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert: ISR must not call PWM duty update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update API");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once");
}

void test_TC_G7_002_isr_toggle_independent_of_egtm_enable_state(void)
{
    /* Arrange: even if EGTM reported disabled, ISR toggle is independent */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "LED toggle must increment regardless of EGTM enable state");
}

/* -------------------------------------------------------------------------- */
/* GROUP 8 - interruptEgtmAtomPeriod: configuration values                    */
/* -------------------------------------------------------------------------- */
void test_TC_G8_001_isr_does_not_change_pwm_frequency_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Capture baseline frequency from init spy */
    float baselineFreq = mock_IfxEgtm_Pwm_init_lastFrequency;

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert: ISR must not alter PWM configuration values */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, baselineFreq, "Baseline frequency must be 20 kHz after init");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "ISR must not modify PWM frequency config");
}

void test_TC_G8_002_isr_does_not_change_channel_count_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    int baselineChannels = mock_IfxEgtm_Pwm_init_lastNumChannels;

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert: channel count unaffected by ISR */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, baselineChannels, "Baseline number of channels must be 3 after init");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "ISR must not modify number of channels");
}

/* -------------------------------------------------------------------------- */
/* GROUP 9 - interruptEgtmAtomPeriod: ISR / interrupt behavior                */
/* -------------------------------------------------------------------------- */
void test_TC_G9_001_isr_multiple_calls_accumulate_from_zero(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "Three ISR invocations must produce three LED toggles");
}

void test_TC_G9_002_isr_and_update_do_not_interfere_with_each_other(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: call ISR once and perform one duty update */
    interruptEgtmAtomPeriod();
    updateEgtmAtom3phInvDuty();

    /* Assert: both operations accounted independently */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One update call must invoke one HAL duty update");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enabled_skips_enable_and_clock_config);
    RUN_TEST(test_TC_G1_002_init_disabled_enables_module_and_configures_clocks);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_configures_led_pin_output_once);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_increments_duties_below_boundary);
    RUN_TEST(test_TC_G3_002_update_wraps_each_channel_independently);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggle_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_called_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_operates_on_three_logical_channels);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_update_two_steps_expected_duties);
    RUN_TEST(test_TC_G6_002_update_ten_steps_return_to_initial_duties);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_isr_does_not_trigger_pwm_update_calls);
    RUN_TEST(test_TC_G7_002_isr_toggle_independent_of_egtm_enable_state);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_isr_does_not_change_pwm_frequency_config);
    RUN_TEST(test_TC_G8_002_isr_does_not_change_channel_count_config);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_isr_multiple_calls_accumulate_from_zero);
    RUN_TEST(test_TC_G9_002_isr_and_update_do_not_interfere_with_each_other);

    return UNITY_END();
}
