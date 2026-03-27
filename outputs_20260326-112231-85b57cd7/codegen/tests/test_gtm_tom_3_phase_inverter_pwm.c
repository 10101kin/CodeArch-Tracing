#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Provided callback symbol from production */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)

/* PWM configuration */
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_DUTY_INIT_U                   (25.0f)
#define UT_DUTY_INIT_V                   (50.0f)
#define UT_DUTY_INIT_W                   (75.0f)
#define UT_DUTY_STEP_PERCENT             (10.0f)

/* Deadtime configuration (ns) */
#define UT_DEADTIME_RISING_NS            (1000U)
#define UT_DEADTIME_FALLING_NS           (1000U)

/* GTM clocking */
#define UT_GTM_ASSUMED_FREQ_HZ           (100000000U)

/* LED / ISR configuration */
#define UT_LED_ISR_PRIORITY              (20U)
#define UT_LED_TOGGLE_FREQ_HZ            (2U)

/* Pin documentation (not used directly in assertions) */
#define UT_PHASE_U_HS_PIN                "P02.0"
#define UT_PHASE_U_LS_PIN                "P02.7"
#define UT_PHASE_V_HS_PIN                "P02.1"
#define UT_PHASE_V_LS_PIN                "P02.4"
#define UT_PHASE_W_HS_PIN                "P02.2"
#define UT_PHASE_W_LS_PIN                "P02.5"
#define UT_LED_PIN                       "P13.0"

/* Indices for phases in arrays captured by mocks */
#define UT_PHASE_U_IDX                   (0)
#define UT_PHASE_V_IDX                   (1)
#define UT_PHASE_W_IDX                   (2)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init guard
 **********************/
void test_TC_G1_001_init_enables_gtm_and_configures_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enabled check should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency should be read once after enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should be enabled once after enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "LED pin should be configured as output at least once");
}

void test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enabled check should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable should NOT be called if already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency should NOT be read when GTM is already enabled and guard is respected");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks should NOT be enabled when GTM is already enabled and guard is respected");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should still be called once");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "LED pin should be configured as output at least once");
}

/************************************
 * GROUP 2 - init configuration values
 ************************************/
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* ensure CMU path runs */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM should be configured for exactly 3 channels (U,V,W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_GTM_ASSUMED_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM init frequency should match CMU module frequency");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "InitConfig should be prepared for 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_GTM_ASSUMED_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "InitConfig frequency should match CMU module frequency");
}

void test_TC_G2_002_init_calls_init_config_and_init_once_and_configures_led_pin(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* use already enabled path */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once during init");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "LED pin should be configured as push-pull output");
}

/****************************************
 * GROUP 3 - init runtime/update interaction
 ****************************************/
void test_TC_G3_001_update_called_after_init_steps_duty_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be applied once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_U_IDX], "Phase U duty should step 25->35");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_V_IDX], "Phase V duty should step 50->60");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_W_IDX], "Phase W duty should step 75->85");
}

void test_TC_G3_002_update_called_twice_results_in_two_hal_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update should be called once per logical update, not per channel");
}

/*****************************************
 * GROUP 4 - init ISR / LED toggle behavior
 *****************************************/
void test_TC_G4_001_after_init_no_led_toggles_occur_until_isr_runs(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "LED toggle function should not be called during init");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "LED pin should be configured during init");
}

void test_TC_G4_002_led_not_toggled_by_update_function(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "LED toggle should not be triggered by PWM duty update");
}

/*********************************************
 * GROUP 5 - update initialization / enable guard
 *********************************************/
void test_TC_G5_001_update_before_init_makes_no_hal_calls(void)
{
    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update before init should not call HAL due to initialization guard");
}

void test_TC_G5_002_update_after_init_calls_hal_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update after init should call HAL exactly once");
}

/****************************************
 * GROUP 6 - update configuration values
 ****************************************/
void test_TC_G6_001_first_update_produces_expected_duties_35_60_85(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_U_IDX], "U duty after first update should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_V_IDX], "V duty after first update should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_W_IDX], "W duty after first update should be 85%");
}

void test_TC_G6_002_second_update_produces_expected_duties_45_70_95(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 35,60,85 */
    updateGtmTom3phInvDuty(); /* 45,70,95 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_U_IDX], "U duty after second update should be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_V_IDX], "V duty after second update should be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_W_IDX], "W duty after second update should be 95%");
}

/****************************************
 * GROUP 7 - update runtime update logic
 ****************************************/
void test_TC_G7_001_wrap_after_three_updates_W_wraps_to_10(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act: 3 updates from 25/50/75 -> 55/80/10 */
    updateGtmTom3phInvDuty(); /* 35/60/85 */
    updateGtmTom3phInvDuty(); /* 45/70/95 */
    updateGtmTom3phInvDuty(); /* 55/80/10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update should be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_U_IDX], "U after 3 updates should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_V_IDX], "V after 3 updates should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_W_IDX], "W should wrap to 10% after reaching 95% + 10%");
}

void test_TC_G7_002_wrap_after_eight_updates_U_to_10_V_to_40_W_to_60(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;
    initGtmTom3phInv();

    /* Act: 8 updates */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update should be called once per update (8 times)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_U_IDX], "U should wrap to 10% after 8 updates from 25%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_V_IDX], "V should be 40% after 8 updates from 50%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_PHASE_W_IDX], "W should be 60% after 8 updates from 75%");
}

/************************************************
 * GROUP 8 - IfxGtm_periodEventFunction config/no-op
 ************************************************/
void test_TC_G8_001_period_event_callback_does_nothing_no_driver_calls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback should not update PWM duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Period callback should not toggle LED");
}

void test_TC_G8_002_period_event_callback_multiple_invocations_have_no_effect(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks should still not update duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Multiple period callbacks should not toggle LED");
}

/************************************************
 * GROUP 9 - IfxGtm_periodEventFunction ISR behavior
 ************************************************/
void test_TC_G9_001_period_event_callback_does_not_toggle_led(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Period callback must not toggle LED");
}

void test_TC_G9_002_period_event_plus_update_only_updates_duty_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_ASSUMED_FREQ_HZ;

    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    initGtmTom3phInv();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Only the update API should trigger a duty update (once)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "LED toggle should still be zero since LED ISR was not invoked");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_gtm_and_configures_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_and_cmu_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_calls_init_config_and_init_once_and_configures_led_pin);

    RUN_TEST(test_TC_G3_001_update_called_after_init_steps_duty_once);
    RUN_TEST(test_TC_G3_002_update_called_twice_results_in_two_hal_updates);

    RUN_TEST(test_TC_G4_001_after_init_no_led_toggles_occur_until_isr_runs);
    RUN_TEST(test_TC_G4_002_led_not_toggled_by_update_function);

    RUN_TEST(test_TC_G5_001_update_before_init_makes_no_hal_calls);
    RUN_TEST(test_TC_G5_002_update_after_init_calls_hal_once);

    RUN_TEST(test_TC_G6_001_first_update_produces_expected_duties_35_60_85);
    RUN_TEST(test_TC_G6_002_second_update_produces_expected_duties_45_70_95);

    RUN_TEST(test_TC_G7_001_wrap_after_three_updates_W_wraps_to_10);
    RUN_TEST(test_TC_G7_002_wrap_after_eight_updates_U_to_10_V_to_40_W_to_60);

    RUN_TEST(test_TC_G8_001_period_event_callback_does_nothing_no_driver_calls);
    RUN_TEST(test_TC_G8_002_period_event_callback_multiple_invocations_have_no_effect);

    RUN_TEST(test_TC_G9_001_period_event_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G9_002_period_event_plus_update_only_updates_duty_once);

    return UNITY_END();
}
