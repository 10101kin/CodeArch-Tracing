#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern for ISR defined in production C (IFX_INTERRUPT) */
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 **********************/
void test_TC_G1_001_init_enables_clocks_when_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when it was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be read when enabling GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK clocks must be enabled when GTM is enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output once");
}

void test_TC_G1_002_init_skips_enable_when_gtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* Not used when already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency read must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK clock enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output once");
}

/**********************
 * GROUP 2 - initGtmTom3phInv: configuration values
 **********************/
void test_TC_G2_001_init_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Simplify path */

    /* Act */
    initGtmTom3phInv();

    /* Assert (verify application values via init() spy, not initConfig) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init() must be configured for 3 logical channels (U,V,W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "init() must set PWM frequency to 20 kHz");
}

void test_TC_G2_002_init_calls_initConfig_and_init_once_each(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Simplify path */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update API should not be called during init");
}

/**********************
 * GROUP 3 - initGtmTom3phInv: runtime update logic
 * (Validate duty update behavior after proper initialization)
 **********************/
void test_TC_G3_001_update_once_increments_duty_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by +10% on first update");
}

void test_TC_G3_002_update_three_times_wraps_W_to_step(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* [35,60,85] */
    updateGtmTom3phInvDuty(); /* [45,70,95] */
    updateGtmTom3phInvDuty(); /* [55,80,10] - W wraps to step */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() even across multiple calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty after 3 updates must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty after 3 updates must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must wrap to step (10%) after exceeding boundary");
}

/**********************
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
 **********************/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggles_led_multiple_times(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "ISR toggle count must accumulate across multiple calls");
}

/**********************
 * GROUP 5 - updateGtmTom3phInvDuty: configuration values
 **********************/
void test_TC_G5_001_update_call_count_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "update() must call HAL exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver must be configured for 3 logical channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G5_002_update_values_are_in_percent_units(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert (values are PERCENT, not 0..1 normalized) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be reported in percent units");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be reported in percent units");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be reported in percent units");
}

/**********************
 * GROUP 6 - updateGtmTom3phInvDuty: runtime update logic
 **********************/
void test_TC_G6_001_wrap_behavior_for_each_channel_over_eight_updates(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 8 updates from [25,50,75] -> [10,40,60] */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() across 8 calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must wrap to 10% after crossing boundary by 8th update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 60% after 8 updates");
}

void test_TC_G6_002_ten_updates_produce_expected_values_and_call_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 10 updates from [25,50,75] -> [30,60,80] */
    for (int i = 0; i < 10; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() across 10 calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 30% after 10 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 60% after 10 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 80% after 10 updates");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_clocks_when_gtm_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_gtm_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_calls_initConfig_and_init_once_each);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_update_once_increments_duty_below_boundary);
    RUN_TEST(test_TC_G3_002_update_three_times_wraps_W_to_step);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_multiple_times);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_call_count_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_values_are_in_percent_units);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_wrap_behavior_for_each_channel_over_eight_updates);
    RUN_TEST(test_TC_G6_002_ten_updates_produce_expected_values_and_call_count);

    return UNITY_END();
}
