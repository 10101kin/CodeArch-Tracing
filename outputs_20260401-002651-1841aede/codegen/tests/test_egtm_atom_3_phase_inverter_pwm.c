#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for functions not exposed by the production header */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY                 (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_DEADTIME_RISE_US              (1.0f)
#define UT_DEADTIME_FALL_US              (1.0f)
#define UT_ISR_PRIORITY                  (20u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* Available extern spy variables from the mock */
extern unsigned int mock_IfxPort_togglePin_callCount;
extern unsigned int mock_IfxPort_setPinModeOutput_callCount;
extern unsigned int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern unsigned int mock_IfxEgtm_Pwm_init_callCount;
extern unsigned int mock_IfxEgtm_Pwm_initConfig_callCount;
extern unsigned int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern unsigned int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern unsigned int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern unsigned int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern unsigned int mock_IfxEgtm_enable_callCount;
extern unsigned int mock_IfxEgtm_isEnabled_callCount;
extern unsigned int mock_togglePin_callCount;
extern float mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern float mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[];
extern float mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[];
extern unsigned int mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float mock_IfxEgtm_Pwm_init_lastFrequency;
extern unsigned int mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern unsigned char mock_IfxEgtm_isEnabled_returnValue;

/* ================================ */
/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
/* ================================ */
void test_TC_G1_001_init_disabled_enables_and_configures_cmu_pwm_led(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* EGTM disabled initially */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* arbitrary module freq */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(),    "IfxEgtm_enable must be called when not enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK frequency must be configured once when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 frequency must be configured once when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "Required EGTM clocks must be enabled once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),      "IfxEgtm_Pwm_init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once after PWM init");
}

void test_TC_G1_002_init_already_enabled_skips_enable_and_cmu(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;  /* EGTM already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(),    "IfxEgtm_enable must NOT be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU configuration must be skipped when EGTM already enabled (guarded)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(),   "GCLK configuration must be skipped when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(),    "CLK0 configuration must be skipped when EGTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "Clock enable must be skipped when EGTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),      "IfxEgtm_Pwm_init must still be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once");
}

/* ================================ */
/* GROUP 2 - initEgtmAtom3phInv: configuration values */
/* ================================ */
void test_TC_G2_001_init_applies_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of channels must match 3-phase configuration");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
}

void test_TC_G2_002_init_uses_tc4xx_api_surface(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "TC4xx IfxEgtm_Pwm_initConfig must be used (not TC3xx IfxGtm_*)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(),      "TC4xx IfxEgtm_Pwm_init must be used (not TC3xx IfxGtm_*)");
}

/* ================================ */
/* GROUP 3 - initEgtmAtom3phInv: runtime update logic */
/* ================================ */
void test_TC_G3_001_update_after_init_calls_single_hal_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be a single HAL call (multi-channel atomic)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step");
}

void test_TC_G3_002_two_sequential_updates_increment_call_count_and_duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates must result in two HAL calls (not per channel)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty after 2 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty after 2 updates");
}

/* ================================ */
/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior */
/* ================================ */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G4_002_isr_toggles_led_cumulatively(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3u, mock_togglePin_callCount, "ISR toggle must accumulate over multiple calls");
}

/* ================================ */
/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
/* ================================ */
void test_TC_G5_001_update_duty_array_size_and_order(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Driver must be initialized for 3 logical channels (U,V,W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Index 0 must map to Phase U");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Index 1 must map to Phase V");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Index 2 must map to Phase W");
}

void test_TC_G5_002_update_duty_units_are_percent_and_wrap_rule_applies(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 5 updates */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Expected after 5 updates: U=75, V=10 (wrap at step 5), W=30 (wrapped at step 3) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty in percent after 5 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty wraps to step value (10%%) at boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty after wrap progression");
}

/* ================================ */
/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
/* ================================ */
void test_TC_G6_001_wrap_occurs_independently_for_each_phase(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 3 updates → only Phase W wraps */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W wraps to 10%% at 3rd step");
}

void test_TC_G6_002_full_cycle_multiple_wraps_and_call_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act: perform 10 updates */
    for (int i = 0; i < 10; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert: after 10 steps → (30, 60, 80) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL call per update (10 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after 10 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after 10 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after 10 steps");
}

/* ================================ */
/* GROUP 7 - interruptEgtmAtom: ISR / interrupt behavior */
/* ================================ */
void test_TC_G7_001_isr_does_not_call_pwm_hal(void)
{
    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must NOT call PWM update HAL");
}

void test_TC_G7_002_isr_multiple_calls_accumulate_toggle_only(void)
{
    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "Two ISR invocations must toggle LED twice");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not affect PWM update call count");
}

/* ================================ */
/* GROUP 8 - IfxEgtm_periodEventFunction: configuration values */
/* ================================ */
void test_TC_G8_001_period_callback_is_empty_no_side_effects(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must NOT toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must NOT update PWM duties");
}

void test_TC_G8_002_period_callback_multiple_calls_still_no_side_effects(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Multiple period callbacks must NOT toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must NOT update PWM duties");
}

/* ================================ */
/* GROUP 9 - IfxEgtm_periodEventFunction: ISR / interrupt behavior */
/* ================================ */
void test_TC_G9_001_period_callback_does_not_toggle_led(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G9_002_period_callback_does_not_invoke_pwm_update(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not call PWM update HAL");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_disabled_enables_and_configures_cmu_pwm_led);
    RUN_TEST(test_TC_G1_002_init_already_enabled_skips_enable_and_cmu);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_applies_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_uses_tc4xx_api_surface);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_after_init_calls_single_hal_once);
    RUN_TEST(test_TC_G3_002_two_sequential_updates_increment_call_count_and_duties);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggles_led_cumulatively);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_duty_array_size_and_order);
    RUN_TEST(test_TC_G5_002_update_duty_units_are_percent_and_wrap_rule_applies);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_wrap_occurs_independently_for_each_phase);
    RUN_TEST(test_TC_G6_002_full_cycle_multiple_wraps_and_call_count);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_isr_does_not_call_pwm_hal);
    RUN_TEST(test_TC_G7_002_isr_multiple_calls_accumulate_toggle_only);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_period_callback_is_empty_no_side_effects);
    RUN_TEST(test_TC_G8_002_period_callback_multiple_calls_still_no_side_effects);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_period_callback_does_not_toggle_led);
    RUN_TEST(test_TC_G9_002_period_callback_does_not_invoke_pwm_update);

    return UNITY_END();
}
