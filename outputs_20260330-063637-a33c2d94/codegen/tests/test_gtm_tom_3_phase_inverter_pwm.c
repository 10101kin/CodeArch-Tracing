#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and optional period callback defined in production */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                         (1e-4f)
#define UT_NUM_CHANNELS                          (3U)
#define UT_PWM_FREQ_HZ                           (20000U)
#define UT_DEADTIME_TICKS                        (100U)
#define UT_PWM_PERIOD_TICKS                      (2500U)
#define UT_GTM_GCLK_HZ                           (100000000U)
#define UT_INIT_DUTY_U_PERCENT                   (25.0f)
#define UT_INIT_DUTY_V_PERCENT                   (50.0f)
#define UT_INIT_DUTY_W_PERCENT                   (75.0f)
#define UT_STEP_PERCENT                          (10.0f)
#define UT_EXPECTED_TOM_TOUT_PINS                (6U)

static void arrange_init_common(boolean gtmAlreadyEnabled)
{
    /* Configure mock return values for a successful init */
    mock_IfxGtm_isEnabled_returnValue = gtmAlreadyEnabled ? TRUE : FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = TRUE;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = TRUE;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = TRUE;
    mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue = TRUE;
}

void setUp(void)
{
    mock_gtm_tom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/* GROUP 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_init_enables_cmu_when_disabled(void)
{
    arrange_init_common(FALSE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_enable_getCallCount(), "GTM enable called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency queried once when enabling");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks enabled when GTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "PwmHl mode set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setDeadtime_getCallCount(), "PwmHl dead-time set once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_EXPECTED_TOM_TOUT_PINS, mock_IfxGtm_PinMap_setTomTout_getCallCount(), "All TOM TOUT pins routed");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxPort_setPinModeOutput_getCallCount(), "Diagnostic GPIO configured once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_enable_getCallCount(), "GTM enable not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency not queried when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks not re-enabled when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init called once");
}

/* GROUP 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_init_sets_numChannels_and_frequency(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)UT_NUM_CHANNELS, (int)mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "PwmHl numChannels must be 3 (U,V,W)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Tom_PwmHl_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_sets_deadtime_per_channel(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setDeadtime_getCallCount(), "Dead-time configured exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[0], "Dead-time rising[0] = 100 ticks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[1], "Dead-time rising[1] = 100 ticks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[2], "Dead-time rising[2] = 100 ticks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[0], "Dead-time falling[0] = 100 ticks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[1], "Dead-time falling[1] = 100 ticks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (unsigned int)mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[2], "Dead-time falling[2] = 100 ticks");
}

/* GROUP 3 - initGtmTom3phInv: runtime update logic (initial coherent update) */
void test_TC_G3_001_init_coherent_update_sequence_once(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate called once during initial write");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "setOnTime called once for all 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate called once to latch initial duties");
}

void test_TC_G3_002_init_applies_initial_duties_percent(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0], "Initial duty U must be 25% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1], "Initial duty V must be 50% (percent units)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "Initial duty W must be 75% (percent units)");
}

/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior */
void test_TC_G4_001_period_callback_has_no_side_effects(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    unsigned int before = mock_togglePin_callCount;
    IfxGtm_periodEventFunction(NULL);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_callCount, "Period event callback must be empty (no GPIO toggle)");
}

void test_TC_G4_002_isr_toggle_accumulates(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_togglePin_callCount, "No toggles before ISR calls");
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_togglePin_callCount, "ISR must toggle diagnostic pin on each invocation");
}

/* GROUP 5 - updateGtmTom3phInvDuty: configuration values */
void test_TC_G5_001_update_preserves_numChannels_and_frequency(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)UT_NUM_CHANNELS, (int)mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "Num channels remains 3 after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Tom_PwmHl_init_lastFrequency, "PWM frequency remains 20 kHz after update");
}

void test_TC_G5_002_update_calls_setOnTime_once_per_update(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    /* One setOnTime from init already */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Baseline: one setOnTime from init");

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Exactly one additional setOnTime per update (not per channel)");

    updateGtmTom3phInvDuty();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Exactly one additional setOnTime for second update");
}

/* GROUP 6 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G6_001_update_increments_duty_below_boundary(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    updateGtmTom3phInvDuty();

    /* Expected after one step of +10%: 35, 60, 85 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0], "U duty increments by step below boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1], "V duty increments by step below boundary");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "W duty increments by step below boundary");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate called once in init and once in update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate called once in init and once in update");
}

void test_TC_G6_002_update_wrap_rule_applies_independently(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    /* Perform three updates to force W to wrap: 75 -> 85 -> 95 -> (wrap) 10 */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0], "U after 3 steps: 25->55");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1], "V after 3 steps: 50->80");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "W wraps to step (10%) after exceeding 100%");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxGtm_Tom_Tom_Timer_getPeriod_getCallCount(), "Timer period read once per update (3 updates)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "setOnTime called once in init + once per update (total 4)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4U, mock_IfxGtm_Tom_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate called once in init + once per update (total 4)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(4U, mock_IfxGtm_Tom_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate called once in init + once per update (total 4)");
}

/* GROUP 7 - updateGtmTom3phInvDuty: ISR / interrupt behavior */
void test_TC_G7_001_isr_toggle_increments_by_one_over_updates(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    updateGtmTom3phInvDuty();

    unsigned int before = mock_togglePin_callCount; /* baseline may include update toggles */
    interruptGtmAtom();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_togglePin_callCount, "ISR increments toggle count by exactly one over baseline");
}

void test_TC_G7_002_isr_multiple_toggles_after_updates(void)
{
    arrange_init_common(TRUE);

    initGtmTom3phInv();

    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = UT_PWM_PERIOD_TICKS;
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    unsigned int before = mock_togglePin_callCount; /* may include 2 update-time toggles */

    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3U, mock_togglePin_callCount, "ISR toggles accumulate across multiple invocations after updates");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_cmu_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_numChannels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_sets_deadtime_per_channel);

    RUN_TEST(test_TC_G3_001_init_coherent_update_sequence_once);
    RUN_TEST(test_TC_G3_002_init_applies_initial_duties_percent);

    RUN_TEST(test_TC_G4_001_period_callback_has_no_side_effects);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates);

    RUN_TEST(test_TC_G5_001_update_preserves_numChannels_and_frequency);
    RUN_TEST(test_TC_G5_002_update_calls_setOnTime_once_per_update);

    RUN_TEST(test_TC_G6_001_update_increments_duty_below_boundary);
    RUN_TEST(test_TC_G6_002_update_wrap_rule_applies_independently);

    RUN_TEST(test_TC_G7_001_isr_toggle_increments_by_one_over_updates);
    RUN_TEST(test_TC_G7_002_isr_multiple_toggles_after_updates);

    return UNITY_END();
}
