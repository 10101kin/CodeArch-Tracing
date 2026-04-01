#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern ISR symbol defined in production .c */
extern void interruptEgtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON              (1e-4f)
#define UT_NUM_CHANNELS               (3)
#define UT_PWM_FREQUENCY_HZ           (20000.0f)
#define UT_INIT_DUTY_U_PERCENT        (25.0f)
#define UT_INIT_DUTY_V_PERCENT        (50.0f)
#define UT_INIT_DUTY_W_PERCENT        (75.0f)
#define UT_DEADTIME_RISING_US         (1.0f)
#define UT_DEADTIME_FALLING_US        (1.0f)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init: enable guard and base calls
 **********************/
void test_TC_G1_001_init_enables_module_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* force enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000U; /* 300 MHz example */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - target EGTM APIs used */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),     "IfxEgtm_enable must be called once when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be queried once on enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must be enabled when module is disabled");

    /* Unified PWM init path must execute exactly once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called exactly once");

    /* Trigger route and LED configuration must be executed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger route must be configured exactly once");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "LED GPIO must be configured as output at least once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* already enabled */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - guard: do not enable if already enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),     "IfxEgtm_enable must NOT be called when module is already enabled");

    /* Still must perform unified init and essential configuration */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger route must be configured exactly once");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* keep path simple */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - verify application-selected values captured at init() */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase PWM");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_configures_deadtime_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - dead-time applied for both edges to all channels */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(), "Dead-time configuration must be applied exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[0],  "Rising DT for channel 0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[1],  "Rising DT for channel 1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_US,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[2],  "Rising DT for channel 2 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[0], "Falling DT for channel 0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[1], "Falling DT for channel 1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[2], "Falling DT for channel 2 must be 1.0 us");
}

/**********************
 * GROUP 3 - init: additional side effects (ADC route, LED pin)
 **********************/
void test_TC_G3_001_init_routes_adc_trigger_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "IfxEgtm_Trigger_trigToAdc must be called exactly once during init");
}

void test_TC_G3_002_init_configures_led_gpio_output(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - at least one output pin configured (LED) */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1, "IfxPort_setPinModeOutput must be called to configure LED as output");
}

/**********************
 * GROUP 4 - ISR / interrupt behavior
 **********************/
void test_TC_G4_001_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/**********************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values via update path
 **********************/
void test_TC_G5_001_update_writes_exact_duty_values_in_percent(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 req[UT_NUM_CHANNELS] = {10.0f, 20.0f, 30.0f};
    unsigned int pre = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    updateEgtmAtom3phInvDuty(req);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre + 1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 10.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 20.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 30.0%%");
}

void test_TC_G5_002_update_clamps_out_of_range_to_bounds(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 req[UT_NUM_CHANNELS] = {-5.0f, 150.0f, 200.0f};
    unsigned int pre = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    updateEgtmAtom3phInvDuty(req);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre + 1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,   mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must clamp to 0.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must clamp to 100.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must clamp to 100.0%%");
}

/**********************
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 **********************/
void test_TC_G6_001_multiple_updates_call_hal_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 a[UT_NUM_CHANNELS] = { 5.0f, 10.0f, 15.0f};
    float32 b[UT_NUM_CHANNELS] = {25.0f, 50.0f, 75.0f};
    unsigned int pre = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    updateEgtmAtom3phInvDuty(a);
    updateEgtmAtom3phInvDuty(b);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre + 2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must result in two HAL calls");
}

void test_TC_G6_002_independent_channel_updates_persist(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 first[UT_NUM_CHANNELS]  = { 0.0f, 55.5f, 100.0f};
    float32 second[UT_NUM_CHANNELS] = {100.0f, 0.0f,  50.0f};

    /* Act 1 */
    updateEgtmAtom3phInvDuty(first);

    /* Assert 1 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON,  0.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "First update: U must be 0.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.5f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "First update: V must be 55.5%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON,100.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "First update: W must be 100.0%%");

    /* Act 2 */
    updateEgtmAtom3phInvDuty(second);

    /* Assert 2 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON,100.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Second update: U must be 100.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON,  0.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Second update: V must be 0.0%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f,  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Second update: W must be 50.0%%");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_module_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_configures_deadtime_values);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_init_routes_adc_trigger_once);
    RUN_TEST(test_TC_G3_002_init_configures_led_gpio_output);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_writes_exact_duty_values_in_percent);
    RUN_TEST(test_TC_G5_002_update_clamps_out_of_range_to_bounds);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_multiple_updates_call_hal_once_each);
    RUN_TEST(test_TC_G6_002_independent_channel_updates_persist);

    return UNITY_END();
}
