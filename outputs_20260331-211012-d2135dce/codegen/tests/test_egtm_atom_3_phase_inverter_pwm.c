#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_CMU_TARGET_FREQ_HZ            (100000000U)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
void test_TC_1_001_init_enables_module_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* Force enable guard to run */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_TARGET_FREQ_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "Peripheral must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency must be configured when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 frequency must be configured when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "PWM init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured once in init");
}

void test_TC_1_002_init_skips_enabling_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* Already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled must still be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "Peripheral enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "GCLK config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CLK0 config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig still must be called");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "PWM init must still be called");
}

/* GROUP 2 - initEgtmAtom3phInv: configuration values */
void test_TC_2_001_init_sets_pwm_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* keep init minimal */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3");
}

void test_TC_2_002_init_calls_init_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "init must be called once");
}

/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (post-init) */
void test_TC_3_001_update_after_init_increments_duties_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be applied with one HAL call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step");
}

void test_TC_3_002_update_after_init_wraps_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Drive W to boundary and wrap only W on the 3rd update */
    updateEgtmAtom3phInvDuty(); /* [35,60,85] */
    updateEgtmAtom3phInvDuty(); /* [45,70,95] */
    updateEgtmAtom3phInvDuty(); /* expect [55,80,10] because W wraps */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should be 55 after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 80 after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should wrap to 10 after hitting boundary");
}

/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior */
void test_TC_4_001_isr_toggles_led_once_per_call(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
void test_TC_5_001_update_calls_hal_once_per_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once per update()");
}

void test_TC_5_002_update_does_not_reinit_pwm_driver(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "PWM init must not be called again during update");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Logical channel count remains 3 after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency config remains 20 kHz after update");
}

/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
void test_TC_6_001_multiple_updates_follow_expected_sequence(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* [35,60,85] */
    updateEgtmAtom3phInvDuty(); /* [45,70,95] */
    updateEgtmAtom3phInvDuty(); /* [55,80,10] */
    updateEgtmAtom3phInvDuty(); /* [65,90,20] */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 65.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U sequence after 4 updates must be 65");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 90.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V sequence after 4 updates must be 90");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W sequence after 4 updates must be 20");
}

void test_TC_6_002_update_wraps_v_phase_at_boundary(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* [35,60,85] */
    updateEgtmAtom3phInvDuty(); /* [45,70,95] */
    updateEgtmAtom3phInvDuty(); /* [55,80,10] */
    updateEgtmAtom3phInvDuty(); /* [65,90,20] */
    updateEgtmAtom3phInvDuty(); /* expect [75,10,30] - V wraps */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 5 updates must be 75");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap to 10 at boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after 5 updates must be 30");
}

/* GROUP 7 - interruptEgtmAtom: initialization / enable guard */
void test_TC_7_001_isr_toggles_without_prior_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED even without prior init");
}

void test_TC_7_002_isr_toggles_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED after init as well");
}

/* GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior */
void test_TC_8_001_isr_calls_do_not_invoke_pwm_update(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update API");
}

void test_TC_8_002_isr_three_calls_increment_toggle_three(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "Three ISR calls must produce three LED toggles");
}

/* GROUP 9 - IfxEgtm_periodEventFunction: configuration values */
void test_TC_9_001_period_callback_has_no_effect_on_led(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_9_002_period_callback_does_not_invoke_pwm_calls(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM duty update API");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_getCallCount(), "Period callback must not (re)initialize PWM");
}

/* GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (empty callback) */
void test_TC_10_001_period_callback_is_idempotent(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void*)0);
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must have no side effects even on multiple calls");
}

void test_TC_10_002_period_callback_multiple_calls_keep_all_counts_zero(void)
{
    /* Act */
    IfxEgtm_periodEventFunction((void*)0);
    IfxEgtm_periodEventFunction((void*)0);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No PWM updates expected from callback only");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "No LED toggles expected from callback only");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_1_001_init_enables_module_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_1_002_init_skips_enabling_when_already_enabled);

    RUN_TEST(test_TC_2_001_init_sets_pwm_frequency_and_channel_count);
    RUN_TEST(test_TC_2_002_init_calls_init_once_each);

    RUN_TEST(test_TC_3_001_update_after_init_increments_duties_by_step);
    RUN_TEST(test_TC_3_002_update_after_init_wraps_independently);

    RUN_TEST(test_TC_4_001_isr_toggles_led_once_per_call);
    RUN_TEST(test_TC_4_002_isr_toggle_accumulates_across_calls);

    RUN_TEST(test_TC_5_001_update_calls_hal_once_per_update);
    RUN_TEST(test_TC_5_002_update_does_not_reinit_pwm_driver);

    RUN_TEST(test_TC_6_001_multiple_updates_follow_expected_sequence);
    RUN_TEST(test_TC_6_002_update_wraps_v_phase_at_boundary);

    RUN_TEST(test_TC_7_001_isr_toggles_without_prior_init);
    RUN_TEST(test_TC_7_002_isr_toggles_after_init);

    RUN_TEST(test_TC_8_001_isr_calls_do_not_invoke_pwm_update);
    RUN_TEST(test_TC_8_002_isr_three_calls_increment_toggle_three);

    RUN_TEST(test_TC_9_001_period_callback_has_no_effect_on_led);
    RUN_TEST(test_TC_9_002_period_callback_does_not_invoke_pwm_calls);

    RUN_TEST(test_TC_10_001_period_callback_is_idempotent);
    RUN_TEST(test_TC_10_002_period_callback_multiple_calls_keep_all_counts_zero);

    return UNITY_END();
}
