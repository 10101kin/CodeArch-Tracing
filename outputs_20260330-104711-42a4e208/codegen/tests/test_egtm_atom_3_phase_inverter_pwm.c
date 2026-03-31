#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and driver callback defined in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)

/* Channel order (logical index) per requirement: CH0->V, CH1->U, CH2->W */
#define UT_IDX_V                         (0)
#define UT_IDX_U                         (1)
#define UT_IDX_W                         (2)

void setUp(void) { 
    mock_egtm_atom_3_phase_inverter_pwm_reset(); 
}

void tearDown(void) {}

/* =============================
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 * ============================= */
void test_TC_G1_001_init_calls_target_APIs_and_init_once_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* Force enable-guard path */
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 300000000U; /* 300 MHz */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),       "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),          "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency should be read once in enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU clocks must be enabled when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),        "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin should be configured as output once after PWM init");
}

void test_TC_G1_002_init_when_already_enabled_does_not_enable_again(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* Skip enable-guard path */
    mock_IfxEgtm_Cmu_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),  "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),     "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig should still be called");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "init should still be called once");
}

/* =============================
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 * ============================= */
void test_TC_G2_001_init_sets_num_channels_to_3_and_frequency_20k(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz at init");
}

void test_TC_G2_002_no_initial_duty_update_during_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No immediate duty update should occur during init");
}

/* =============================
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (duty stepper validation)
 * ============================= */
void test_TC_G3_001_update_increments_each_channel_by_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert (CH0->V, CH1->U, CH2->W) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_V], "V duty should step by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_U], "U duty should step by +10% on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_W], "W duty should step by +10% on first update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once per update()");
}

void test_TC_G3_002_wraps_channel_W_on_third_update_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act: three updates → W: 75->85->95->wrap to 10 */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_V], "V after 3 updates must be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_U], "U after 3 updates must be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_W], "W must wrap to 10% on 3rd update");
}

/* =============================
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior (ISR toggles LED)
 * ============================= */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

/* =============================
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values
 * ============================= */
void test_TC_G5_001_init_frequency_is_20k_before_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be configured to 20 kHz");
}

void test_TC_G5_002_update_does_not_change_configured_frequency_spy(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert (init spy remains the same) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency configuration must remain 20 kHz after update");
}

/* =============================
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 * ============================= */
void test_TC_G6_001_wraps_channel_V_on_fifth_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act: five updates → V: 50->60->70->80->90->wrap to 10 */
    for (int i = 0; i < 5; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_U], "U after 5 updates must be 75%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_V], "V must wrap to 10% on 5th update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_W], "W after 5 updates must be 30%");
}

void test_TC_G6_002_wraps_channel_U_on_eighth_update_and_callcount_matches(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act: eight updates → U wraps on 8th; track call count */
    for (int i = 0; i < 8; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert (V: 40, U: wrap->10, W: 60) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_V], "V after 8 updates must be 40%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_U], "U must wrap to 10% on 8th update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[UT_IDX_W], "W after 8 updates must be 60%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update() invocation");
}

/* =============================
 * GROUP 7 - interruptEgtmAtom: configuration values (side-effect absence on PWM config)
 * ============================= */
void test_TC_G7_001_isr_does_not_call_pwm_update(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM duty update");
}

void test_TC_G7_002_isr_does_not_call_pwm_init(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_getCallCount(), "ISR must not call PWM init");
}

/* =============================
 * GROUP 8 - interruptEgtmAtom: ISR / interrupt behavior
 * ============================= */
void test_TC_G8_001_isr_multiple_toggles_accumulate(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i) {
        interruptEgtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "ISR should toggle LED five times after five calls");
}

void test_TC_G8_002_isr_does_not_change_port_mode_configuration(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR should not reconfigure port mode");
}

/* =============================
 * GROUP 9 - IfxEgtm_periodEventFunction: configuration values (no side effects)
 * ============================= */
void test_TC_G9_001_period_callback_has_no_led_toggle_side_effect(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_does_not_call_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

/* =============================
 * GROUP 10 - IfxEgtm_periodEventFunction: ISR / interrupt behavior (remain empty)
 * ============================= */
void test_TC_G10_001_multiple_period_callbacks_do_not_toggle_led(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
}

void test_TC_G10_002_period_callback_does_not_call_pwm_init(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_init_getCallCount(), "Period callback must not call PWM init");
}

/* =============================
 * GROUP 11 - initEgtmAtom3phInv: initialization / enable guard (API usage emphasis)
 * ============================= */
void test_TC_G11_001_init_calls_initConfig_and_init_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "init must be called exactly once");
}

void test_TC_G11_002_init_enables_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called when module is disabled");
}

/* =============================
 * GROUP 12 - updateEgtmAtom3phInvDuty: initialization / enable guard (call-rate emphasis)
 * ============================= */
void test_TC_G12_001_update_after_init_calls_hal_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Single update call must invoke HAL exactly once");
}

void test_TC_G12_002_multiple_updates_call_hal_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 4; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Four update() invocations must call HAL four times");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_calls_target_APIs_and_init_once_disabled);
    RUN_TEST(test_TC_G1_002_init_when_already_enabled_does_not_enable_again);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_num_channels_to_3_and_frequency_20k);
    RUN_TEST(test_TC_G2_002_no_initial_duty_update_during_init);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_update_increments_each_channel_by_step);
    RUN_TEST(test_TC_G3_002_wraps_channel_W_on_third_update_only);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_over_multiple_calls);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_init_frequency_is_20k_before_update);
    RUN_TEST(test_TC_G5_002_update_does_not_change_configured_frequency_spy);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_wraps_channel_V_on_fifth_update);
    RUN_TEST(test_TC_G6_002_wraps_channel_U_on_eighth_update_and_callcount_matches);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_isr_does_not_call_pwm_update);
    RUN_TEST(test_TC_G7_002_isr_does_not_call_pwm_init);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_isr_multiple_toggles_accumulate);
    RUN_TEST(test_TC_G8_002_isr_does_not_change_port_mode_configuration);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_period_callback_has_no_led_toggle_side_effect);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_call_pwm_update);

    /* GROUP 10 */
    RUN_TEST(test_TC_G10_001_multiple_period_callbacks_do_not_toggle_led);
    RUN_TEST(test_TC_G10_002_period_callback_does_not_call_pwm_init);

    /* GROUP 11 */
    RUN_TEST(test_TC_G11_001_init_calls_initConfig_and_init_once_each);
    RUN_TEST(test_TC_G11_002_init_enables_clocks_when_disabled);

    /* GROUP 12 */
    RUN_TEST(test_TC_G12_001_update_after_init_calls_hal_once);
    RUN_TEST(test_TC_G12_002_multiple_updates_call_hal_once_per_call);

    return UNITY_END();
}
