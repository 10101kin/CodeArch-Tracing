#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern for ISR and callback defined in production .c but not declared in .h */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                    (1e-4f)
#define UT_NUM_CHANNELS                     (3)
#define UT_PWM_FREQUENCY_HZ                 (20000)
#define UT_INIT_DUTY_U_PERCENT              (25.0f)
#define UT_INIT_DUTY_V_PERCENT              (50.0f)
#define UT_INIT_DUTY_W_PERCENT              (75.0f)
#define UT_STEP_PERCENT                     (10.0f)
#define UT_DEADTIME_RISING_US               (1.0f)
#define UT_DEADTIME_FALLING_US              (1.0f)
#define UT_ISR_PRIORITY                     (20)
#define UT_CMU_TARGET_FREQ_HZ               (100000000)
#define UT_WAIT_TIME_MS                     (500)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/***********************
 * GROUP 1 - init (enable guard)
 ***********************/
void test_TC_G1_001_init_calls_tc4xx_apis_when_egtm_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* eGTM disabled path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: verify TC4xx API usage and CMU setup performed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),     "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),        "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK frequency must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 frequency must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured once as push-pull output");
}

void test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* eGTM already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: enable path skipped, but init still performed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),     "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),        "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clock enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured once as push-pull output");
}

/***********************
 * GROUP 2 - init (configuration values)
 ***********************/
void test_TC_G2_001_init_config_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* keep CMU path quiet */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Num channels must be 3 (U,V,W)");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQUENCY_HZ, (int)mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
}

void test_TC_G2_002_led_gpio_configured_push_pull_on_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as push-pull during init");
}

/***********************
 * GROUP 3 - init: runtime update logic (duty update path)
 ***********************/
void test_TC_G3_001_update_increments_all_duties_by_step_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be applied once as a bulk update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 25+10=35% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 50+10=60% after one update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 75+10=85% after one update");
}

void test_TC_G3_002_update_wraps_independently_when_crossing_100(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: three updates to wrap W only */
    updateEgtmAtom3phInvDuty(); /* -> U=35 V=60 W=85 */
    updateEgtmAtom3phInvDuty(); /* -> U=45 V=70 W=95 */
    updateEgtmAtom3phInvDuty(); /* -> U=55 V=80 W=10 (wrap) */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 55% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 80% after three updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must wrap to 10% after three updates");
}

/***********************
 * GROUP 4 - init: ISR / interrupt behavior
 ***********************/
void test_TC_G4_001_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED once per call");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle must accumulate across multiple calls");
}

/***********************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 ***********************/
void test_TC_G5_001_update_applies_three_channel_bulk_update_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Must call unified multi-channel duty update exactly once");
}

void test_TC_G5_002_after_init_update_duties_match_expected_percentages(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: expect percent units */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 35% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 60% after first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 85% after first update");
}

/***********************
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 ***********************/
void test_TC_G6_001_multiple_updates_wrap_v_and_w_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: four updates (W wrapped earlier; V approaches boundary) */
    updateEgtmAtom3phInvDuty(); /* 35, 60, 85 */
    updateEgtmAtom3phInvDuty(); /* 45, 70, 95 */
    updateEgtmAtom3phInvDuty(); /* 55, 80, 10 */
    updateEgtmAtom3phInvDuty(); /* 65, 90, 20 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U must be 65% after four updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 90.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V must be 90% after four updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W must be 20% after four updates (wrapped once)");
}

void test_TC_G6_002_wrap_all_channels_by_advancing_to_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: advance 9 updates so all phases have wrapped at least once */
    for (int i = 0; i < 9; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 9 updates: U=20, V=50, W=70 (see sequence derivation) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U must be 20% after nine updates (wrapped once)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V must be 50% after nine updates (wrapped once)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W must be 70% after nine updates (wrapped once)");
}

/***********************
 * GROUP 7 - interruptEgtmAtom: initialization / enable guard
 ***********************/
void test_TC_G7_001_isr_works_without_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even without prior init");
}

void test_TC_G7_002_isr_after_init_still_toggles_led(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED after init as well");
}

/***********************
 * GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior
 ***********************/
void test_TC_G8_001_isr_multiple_invocations_accumulate(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "LED toggle count must equal number of ISR invocations");
}

void test_TC_G8_002_isr_does_not_call_pwm_update(void)
{
    /* Act: call ISR only */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert: ISR must not invoke PWM duty update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call IfxEgtm_Pwm_updateChannelsDutyImmediate");
}

/***********************
 * GROUP 9 - IfxEgtm_periodEventFunction: configuration values
 ***********************/
void test_TC_G9_001_callback_does_not_toggle_led(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period-event callback must not toggle LED");
}

void test_TC_G9_002_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period-event callback must not update PWM duties");
}

/***********************
 * GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (no-op)
 ***********************/
void test_TC_G10_001_callback_multiple_calls_still_no_toggle(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Callback remains no-op across multiple calls");
}

void test_TC_G10_002_callback_accepts_null_argument_and_returns(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert: verify no side effects */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Callback with NULL data must not update PWM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Callback with NULL data must not toggle LED");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_tc4xx_apis_when_egtm_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_config_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_led_gpio_configured_push_pull_on_init);

    RUN_TEST(test_TC_G3_001_update_increments_all_duties_by_step_without_wrap);
    RUN_TEST(test_TC_G3_002_update_wraps_independently_when_crossing_100);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);

    RUN_TEST(test_TC_G5_001_update_applies_three_channel_bulk_update_once);
    RUN_TEST(test_TC_G5_002_after_init_update_duties_match_expected_percentages);

    RUN_TEST(test_TC_G6_001_multiple_updates_wrap_v_and_w_independently);
    RUN_TEST(test_TC_G6_002_wrap_all_channels_by_advancing_to_boundary);

    RUN_TEST(test_TC_G7_001_isr_works_without_init);
    RUN_TEST(test_TC_G7_002_isr_after_init_still_toggles_led);

    RUN_TEST(test_TC_G8_001_isr_multiple_invocations_accumulate);
    RUN_TEST(test_TC_G8_002_isr_does_not_call_pwm_update);

    RUN_TEST(test_TC_G9_001_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G9_002_callback_does_not_call_pwm_update);

    RUN_TEST(test_TC_G10_001_callback_multiple_calls_still_no_toggle);
    RUN_TEST(test_TC_G10_002_callback_accepts_null_argument_and_returns);

    return UNITY_END();
}
