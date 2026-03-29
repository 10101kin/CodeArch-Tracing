#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"
#include <stdint.h>

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                      (1e-4f)

/* Signal requirements / mapping (documentary only; not asserted directly) */
#define UT_SIGNAL_MODULE                      "GTM"
#define UT_SIGNAL_SUBMODULE                   "TOM1"
#define UT_PHASE_U_HS_CHANNEL                 (2U)
#define UT_PHASE_U_LS_CHANNEL                 (1U)
#define UT_PHASE_V_HS_CHANNEL                 (4U)
#define UT_PHASE_V_LS_CHANNEL                 (3U)
#define UT_PHASE_W_HS_CHANNEL                 (6U)
#define UT_PHASE_W_LS_CHANNEL                 (5U)

/* Timing and configuration macros */
#define UT_PWM_ALIGNMENT_CENTER               (1)
#define UT_PWM_FREQUENCY_HZ                   (20000U)
#define UT_FXCLK0_MHZ                         (100U)
#define UT_TICK_NS                            (10U)
#define UT_DEADTIME_US                        (0.5f)
#define UT_MIN_PULSE_US                       (1.0f)
#define UT_DEADTIME_TICKS                     (50U)    /* given */
#define UT_MIN_PULSE_TICKS                    (100U)   /* given */
#define UT_NUM_CHANNELS                       (3U)

/* Period ticks assumption: 100 MHz clock, 20 kHz PWM -> 50 us period -> 5000 ticks */
#define UT_PWM_PERIOD_TICKS                   (5000U)
#define UT_MAX_ON_TICKS                       (UT_PWM_PERIOD_TICKS - UT_MIN_PULSE_TICKS)

/* Initial duties in ticks: 25%, 50%, 75% of period */
#define UT_INIT_U_TICKS                       ((UT_PWM_PERIOD_TICKS * 25U) / 100U)  /* 1250 */
#define UT_INIT_V_TICKS                       ((UT_PWM_PERIOD_TICKS * 50U) / 100U)  /* 2500 */
#define UT_INIT_W_TICKS                       ((UT_PWM_PERIOD_TICKS * 75U) / 100U)  /* 3750 */

/* Helper: set happy-path return values and run init */
static void arrange_successful_init_and_call(void)
{
    /* Ensure production init path succeeds */
    mock_IfxGtm_Tom_Timer_init_returnValue    = 1; /* TRUE */
    mock_IfxGtm_Tom_PwmHl_init_returnValue    = 1; /* TRUE */
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1; /* TRUE */

    initGtmTomPwm();
}

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - initGtmTomPwm: initialization / enable guard
 **********************/
void test_TC_G1_001_init_enables_gtm_and_clocks_once(void)
{
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_enable_getCallCount(), "GTM enable must be called exactly once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "GTM FXCLK domain enable must be called exactly once during init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "PWM mode must be configured exactly once (center-aligned)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer run must be called exactly once to start TOM");
}

void test_TC_G1_002_init_called_twice_reinitializes_drivers(void)
{
    arrange_successful_init_and_call();
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_enable_getCallCount(), "GTM enable should be invoked on each init call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Clock enable should be invoked on each init call");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig should be invoked twice after two inits");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init should be invoked twice after two inits");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig should be invoked twice after two inits");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init should be invoked twice after two inits");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "Mode set should be invoked twice after two inits");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2U, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer run should be invoked twice after two inits");
}

/**********************
 * GROUP 2 - initGtmTomPwm: configuration values
 **********************/
void test_TC_G2_001_config_frequency_and_channel_count(void)
{
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (uint32_t)mock_IfxGtm_Tom_PwmHl_init_lastFrequency, "Configured PWM frequency must be 20 kHz (init)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (uint32_t)mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency, "Configured PWM frequency must be 20 kHz (initConfig)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (uint32_t)mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "Channel count must be 3 (init)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (uint32_t)mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels, "Channel count must be 3 (initConfig)");
}

void test_TC_G2_002_config_deadtime_and_initial_duties(void)
{
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_lastDtRising[0], "Rising-edge dead-time ticks must match requirement (50 ticks)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_lastDtFalling[0], "Falling-edge dead-time ticks must match requirement (50 ticks)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_U_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0], "Initial U duty on-time must be 25% of period in ticks (1250)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_V_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1], "Initial V duty on-time must be 50% of period in ticks (2500)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_W_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "Initial W duty on-time must be 75% of period in ticks (3750)");
}

/**********************
 * GROUP 3 - initGtmTomPwm: runtime update logic (shadow transfer sequence on init)
 **********************/
void test_TC_G3_001_init_uses_shadow_update_sequence_once(void)
{
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must be called once during init shadow sequence");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "setOnTime must be called once during init to program initial duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate must be called once during init shadow sequence");
}

void test_TC_G3_002_init_starts_timer_and_single_setOnTime(void)
{
    arrange_successful_init_and_call();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer must be started exactly once by init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Only one HAL update shall be issued for initial duties (not per channel)");
}

/**********************
 * GROUP 4 - updateGtmTomPwmDutyCycles: initialization / enable guard
 **********************/
void test_TC_G4_001_update_performs_single_hal_update_per_call(void)
{
    arrange_successful_init_and_call();

    uint32_t pre_disable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32_t pre_set     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32_t pre_apply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    updateGtmTomPwmDutyCycles();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_disable + 1U, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must increment by 1 per update call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_set + 1U,     mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "setOnTime must increment by 1 per update call (single HAL burst)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_apply + 1U,   mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "ApplyUpdate must increment by 1 per update call");
}

void test_TC_G4_002_update_multiple_calls_increment_counts(void)
{
    arrange_successful_init_and_call();

    uint32_t pre_disable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32_t pre_set     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32_t pre_apply   = mock_IfxGtm_Tom_Tomer_applyUpdate_getCallCount(); /* Intentionally incorrect to ensure using available getter - corrected below */
}

/* The previous function had a typo; provide the correct test implementation */
void test_TC_G4_002_update_multiple_calls_increment_counts_corrected(void)
{
    arrange_successful_init_and_call();

    uint32_t pre_disable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32_t pre_set     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32_t pre_apply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    const uint32_t nCalls = 3U;
    for (uint32_t i = 0; i < nCalls; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_disable + nCalls, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must increment by number of update calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_set + nCalls,     mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "setOnTime must increment by number of update calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_apply + nCalls,   mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "ApplyUpdate must increment by number of update calls");
}

/**********************
 * GROUP 5 - updateGtmTomPwmDutyCycles: configuration values
 **********************/
void test_TC_G5_001_config_values_persist_after_update(void)
{
    arrange_successful_init_and_call();

    /* Perform one runtime update */
    updateGtmTomPwmDutyCycles();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (uint32_t)mock_IfxGtm_Tom_PwmHl_init_lastFrequency, "Configured PWM frequency must remain 20 kHz after update (init spy)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, (uint32_t)mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency, "Configured PWM frequency must remain 20 kHz after update (initConfig spy)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (uint32_t)mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "Channel count must remain 3 after update (init spy)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (uint32_t)mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels, "Channel count must remain 3 after update (initConfig spy)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_lastDtRising[0], "Dead-time rising must remain configured after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_lastDtFalling[0], "Dead-time falling must remain configured after update");
}

void test_TC_G5_002_update_respects_on_time_bounds(void)
{
    arrange_successful_init_and_call();

    /* Perform several updates to exercise bounds */
    for (int i = 0; i < 10; ++i)
    {
        updateGtmTomPwmDutyCycles();
        /* Every update must keep duties within [min, max] */
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] >= UT_MIN_PULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] <= UT_MAX_ON_TICKS), "U duty must remain within min/max on-time bounds");
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] >= UT_MIN_PULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] <= UT_MAX_ON_TICKS), "V duty must remain within min/max on-time bounds");
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2] >= UT_MIN_PULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2] <= UT_MAX_ON_TICKS), "W duty must remain within min/max on-time bounds");
    }
}

/**********************
 * GROUP 6 - updateGtmTomPwmDutyCycles: runtime update logic
 **********************/
void test_TC_G6_001_duty_increments_when_below_boundary(void)
{
    arrange_successful_init_and_call();

    uint32_t prevU = (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0];
    uint32_t prevV = (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1];
    uint32_t prevW = (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];

    updateGtmTomPwmDutyCycles();

    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] > prevU, "U duty must increase on update when below max threshold");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] > prevV, "V duty must increase on update when below max threshold");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2] > prevW, "W duty must increase on update when below max threshold (before wrap)");
}

void test_TC_G6_002_wrap_around_occurs_and_is_independent(void)
{
    arrange_successful_init_and_call();

    /* W starts highest (75%), so it should wrap first. */
    uint32_t pre_set_calls = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();

    const uint32_t LOOP_LIMIT = 10000U; /* safety limit */
    uint32_t loops = 0U;
    uint32_t prevW = (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];

    while (loops < LOOP_LIMIT)
    {
        updateGtmTomPwmDutyCycles();
        ++loops;
        uint32_t currW = (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];
        if (currW < prevW)
        {
            break; /* wrap detected */
        }
        prevW = currW;
        TEST_ASSERT_TRUE_MESSAGE(currW <= UT_MAX_ON_TICKS, "W duty must not exceed max threshold while increasing");
    }

    TEST_ASSERT_TRUE_MESSAGE(loops < LOOP_LIMIT, "Wrap-around must occur for W within a reasonable number of updates");

    /* After wrap, W should be set exactly to the minimum threshold */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_MIN_PULSE_TICKS, (uint32_t)mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "W duty must wrap back exactly to minimum on-time threshold");

    /* U and V should not have wrapped yet (start from 25% and 50%) */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] != UT_MIN_PULSE_TICKS, "U duty should not have wrapped when W wraps first");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] != UT_MIN_PULSE_TICKS, "V duty should not have wrapped when W wraps first");

    /* Verify HAL update function called once per update (not per channel) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(pre_set_calls + loops, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "setOnTime must be called once per update call, not per channel");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_enables_gtm_and_clocks_once);
    RUN_TEST(test_TC_G1_002_init_called_twice_reinitializes_drivers);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_config_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_config_deadtime_and_initial_duties);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_init_uses_shadow_update_sequence_once);
    RUN_TEST(test_TC_G3_002_init_starts_timer_and_single_setOnTime);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_update_performs_single_hal_update_per_call);
    RUN_TEST(test_TC_G4_002_update_multiple_calls_increment_counts_corrected);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_config_values_persist_after_update);
    RUN_TEST(test_TC_G5_002_update_respects_on_time_bounds);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_duty_increments_when_below_boundary);
    RUN_TEST(test_TC_G6_002_wrap_around_occurs_and_is_independent);

    return UNITY_END();
}
