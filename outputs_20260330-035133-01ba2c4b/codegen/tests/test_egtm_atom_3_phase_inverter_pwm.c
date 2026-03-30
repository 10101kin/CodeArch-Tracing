#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern ISR (defined in production .c) */
extern void interruptEgtmAtom(void);

/* Self-documenting UT macros derived from configuration */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEAD_TIME_US_RISING           (1.0f)
#define UT_DEAD_TIME_US_FALLING          (1.0f)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init: enable guard and target API usage
 **********************/
void test_TC_G1_001_init_when_already_enabled_uses_tc4xx_apis_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must NOT be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once after init");
}

void test_TC_G1_002_init_enables_module_and_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000U; /* arbitrary system freq */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled should be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called once when enabling clocks");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_applies_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: verify application-set values via init() spies (not initConfig) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_led_gpio_configured_output_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED (P03.9) must be configured as push-pull output once after PWM init");
}

/**********************
 * GROUP 3 - init: runtime update linkage (state prepared for updates)
 **********************/
void test_TC_G3_001_update_once_after_init_applies_step_from_initial_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: HAL update called once and duties are initial + step (percent) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be applied exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after first update must be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after first update must be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after first update must be 85%");
}

void test_TC_G3_002_three_updates_wrap_W_only_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 3 updates */
    updateEgtmAtom3phInvDuty(); /* -> U=35 V=60 W=85 */
    updateEgtmAtom3phInvDuty(); /* -> U=45 V=70 W=95 */
    updateEgtmAtom3phInvDuty(); /* -> U=55 V=80 W=10 (wrap) */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3rd update must be 55% (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3rd update must be 80% (no wrap yet)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 3rd update must wrap to 10%");
}

/**********************
 * GROUP 4 - ISR / interrupt behavior
 **********************/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    /* No init required for ISR mock toggle behavior */

    /* Act */
    interruptEgtmAtom();

    /* Assert: prefer mock_togglePin_callCount per rules */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle count must accumulate across multiple invocations");
}

/**********************
 * GROUP 5 - update: initialization / call-rate checks
 **********************/
void test_TC_G5_001_update_calls_driver_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update invocation must call HAL exactly once");
}

void test_TC_G5_002_update_twice_results_in_two_hal_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two update invocations must call HAL twice");
}

/**********************
 * GROUP 6 - update: configuration values visibility and units
 **********************/
void test_TC_G6_001_config_values_persist_after_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: init-configured values remain as expected */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of channels must remain 3 after updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after updates");
}

void test_TC_G6_002_update_outputs_duty_in_percent_units(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert: percentages (not fractions) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty must be 35.0 percent (not 0.35)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty must be 60.0 percent (not 0.60)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty must be 85.0 percent (not 0.85)");
}

/**********************
 * GROUP 7 - update: runtime duty logic (increment and wrap)
 **********************/
void test_TC_G7_001_duty_increment_below_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should increment by 10 to 35");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should increment by 10 to 60");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should increment by 10 to 85");
}

void test_TC_G7_002_wrap_around_when_duty_plus_step_reaches_100(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 8 updates to force wraps on all channels as per step logic */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 updates: U=10, V=40, W=60 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should wrap to 10 after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should be 40 after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should be 60 after 8 updates");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_when_already_enabled_uses_tc4xx_apis_only);
    RUN_TEST(test_TC_G1_002_init_enables_module_and_cmu_when_disabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_applies_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_led_gpio_configured_output_after_init);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_update_once_after_init_applies_step_from_initial_duties);
    RUN_TEST(test_TC_G3_002_three_updates_wrap_W_only_independently);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_calls_driver_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_twice_results_in_two_hal_calls);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_config_values_persist_after_update);
    RUN_TEST(test_TC_G6_002_update_outputs_duty_in_percent_units);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_duty_increment_below_boundary);
    RUN_TEST(test_TC_G7_002_wrap_around_when_duty_plus_step_reaches_100);

    return UNITY_END();
}
