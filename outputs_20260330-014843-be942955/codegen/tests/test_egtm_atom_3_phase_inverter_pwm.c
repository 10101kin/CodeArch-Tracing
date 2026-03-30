#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQ_HZ                   (20000)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEAD_TIME_RISING_US           (1.0f)
#define UT_DEAD_TIME_FALLING_US          (1.0f)
#define UT_ISR_PRIORITY                  (20)

/* Extern declaration for ISR defined in production .c */
extern void interruptEgtmAtom(void);

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init: enable guard and driver call ordering
 **********************/
void test_TC_G1_001_init_enables_egtm_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* Force enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000U; /* 300 MHz */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - init sequence */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");

    /* Assert - enable guard and CMU setup */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read during enable sequence");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider must be configured during enable sequence");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK/DTM divider must be configured during enable sequence");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Required EGTM clocks must be enabled during enable sequence");

    /* LED pin configured as output after PWM init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output after PWM init");
}

void test_TC_G1_002_init_skips_cmu_setup_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* Skip enable path */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - init sequence still occurs */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once even if already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once even if already enabled");

    /* Assert - CMU/enable path skipped */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must NOT be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK/DTM divider must NOT be configured when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "EGTM clocks must NOT be re-enabled when already enabled");

    /* LED pin configured as output after PWM init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured as output after PWM init");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3 (U,V,W)");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_applies_1us_deadtime_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - dead-time configured to 1.0 us rising/falling for each of 3 channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[0], "DT rising[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[1], "DT rising[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_RISING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[2], "DT rising[2] must be 1.0 us");

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[0], "DT falling[0] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[1], "DT falling[1] must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_FALLING_US, mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[2], "DT falling[2] must be 1.0 us");
}

/**********************
 * GROUP 3 - runtime duty update (single and few steps)
 **********************/
void test_TC_G3_001_first_update_increments_duty_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - HAL called once, duties incremented by step in percent */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after first update must be 35% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after first update must be 60% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after first update must be 85% (percent units)");
}

void test_TC_G3_002_three_updates_wraps_w_phase_and_updates_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1st: 35, 60, 85 */
    updateEgtmAtom3phInvDuty(); /* 2nd: 45, 70, 95 */
    updateEgtmAtom3phInvDuty(); /* 3rd: 55, 80, 10 (wrap on W) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3 updates must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3 updates must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 3 updates must wrap to 10% per wrap logic");
}

/**********************
 * GROUP 4 - ISR / interrupt behavior
 **********************/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert - prefer mock_togglePin_callCount per rules */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per interrupt call");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle count must accumulate across multiple calls");
}

/**********************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration consistency and atomicity
 **********************/
void test_TC_G5_001_update_is_atomic_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 35,60,85 */
    updateEgtmAtom3phInvDuty(); /* 45,70,95 */

    /* Assert - exactly two HAL calls, last duties reflect second update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Each update must result in exactly one HAL call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 2 updates must be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 2 updates must be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 2 updates must be 95%");
}

void test_TC_G5_002_update_preserves_three_channel_configuration(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - init configuration remains 3 channels at 20 kHz (from init spy) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Init must configure exactly 3 logical channels");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Init must configure PWM frequency to 20 kHz");
}

/**********************
 * GROUP 6 - updateEgtmAtom3phInvDuty: wrap-around logic
 **********************/
void test_TC_G6_001_wrap_logic_independent_per_channel_after_eight_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - perform 8 updates */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 updates: U=10, V=40, W=60 (percent) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called once per update across 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 8 updates must wrap to 10%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 8 updates must be 40%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 8 updates must be 60%");
}

void test_TC_G6_002_wrap_logic_mixed_state_after_seven_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act - perform 7 updates */
    for (int i = 0; i < 7; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 7 updates: U=95 (no wrap yet), V=30 (wrapped earlier), W=40 (wrapped earlier) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called once per update across 7 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 7 updates must be 95%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 7 updates must be 30% due to wrap");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty after 7 updates must be 40% due to wrap");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_egtm_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_setup_when_already_enabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_applies_1us_deadtime_per_channel);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_duty_by_step);
    RUN_TEST(test_TC_G3_002_three_updates_wraps_w_phase_and_updates_once_per_call);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_is_atomic_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_G5_002_update_preserves_three_channel_configuration);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_wrap_logic_independent_per_channel_after_eight_updates);
    RUN_TEST(test_TC_G6_002_wrap_logic_mixed_state_after_seven_updates);

    return UNITY_END();
}
