#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQUENCY_HZ              (20000)
#define UT_ALIGNMENT_CENTER_ALIGNED      (1)
#define UT_DEADTIME_US                   (0.5f)
#define UT_MINPULSE_US                   (1.0f)
#define UT_FXCLK0_FREQ_MHZ               (100)
#define UT_PERIOD_TICKS                  (5000)
#define UT_DEADTIME_TICKS                (50)
#define UT_MINPULSE_TICKS                (100)
#define UT_NUM_CHANNEL_PAIRS             (3)
#define UT_NUM_TOM_PINS                  (6)

/* Initial duty percentages and corresponding ticks */
#define UT_INIT_DUTY_PERCENT_U           (25)
#define UT_INIT_DUTY_PERCENT_V           (50)
#define UT_INIT_DUTY_PERCENT_W           (75)
#define UT_INIT_DUTY_TICKS_U             ((UT_PERIOD_TICKS * UT_INIT_DUTY_PERCENT_U) / 100)
#define UT_INIT_DUTY_TICKS_V             ((UT_PERIOD_TICKS * UT_INIT_DUTY_PERCENT_V) / 100)
#define UT_INIT_DUTY_TICKS_W             ((UT_PERIOD_TICKS * UT_INIT_DUTY_PERCENT_W) / 100)

/* Extern mock control variables provided by the mock header */
extern uint32 mock_IfxGtm_PinMap_setTomTout_callCount;
extern uint32 mock_IfxGtm_enable_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_initConfig_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_applyUpdate_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_run_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_init_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_disableUpdate_callCount;
extern uint32 mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount;
extern uint8  mock_IfxGtm_Tom_Timer_init_returnValue;
extern uint32 mock_IfxGtm_Tom_PwmHl_init_callCount;
extern uint32 mock_IfxGtm_Tom_PwmHl_initConfig_callCount;
extern uint32 mock_IfxGtm_Tom_PwmHl_setMode_callCount;
extern uint32 mock_IfxGtm_Tom_PwmHl_setOnTime_callCount;
extern uint8  mock_IfxGtm_Tom_PwmHl_init_returnValue;
extern uint8  mock_IfxGtm_Tom_PwmHl_setMode_returnValue;
extern uint32 mock_IfxGtm_Cmu_enableClocks_callCount;
extern uint32 mock_IfxGtm_Tom_PwmHl_init_lastNumChannels;
extern uint32 mock_IfxGtm_Tom_PwmHl_init_lastFrequency;
extern uint32 mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels;
extern uint32 mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency;
extern uint32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[];
extern uint32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[];
extern uint32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[];
extern uint32 mock_togglePin_callCount;

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ========================= GROUP 1 - initGtmTom3phInv: initialization / enable guard ========================= */
static void arrange_successful_init_returns(void)
{
    mock_IfxGtm_Tom_Timer_init_returnValue = 1u;      /* TRUE */
    mock_IfxGtm_Tom_PwmHl_init_returnValue  = 1u;     /* TRUE */
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1u;   /* TRUE */
}

void test_TC_G1_001_init_enables_gtm_and_clocks_once(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_enable_getCallCount(), "GTM enable must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called once during init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer run must be called once to start PWM base counter");
}

void test_TC_G1_002_init_is_idempotent_no_extra_calls_on_second_invocation(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    /* Capture baseline after first init */
    uint32 base_enable       = mock_IfxGtm_enable_getCallCount();
    uint32 base_clk          = mock_IfxGtm_Cmu_enableClocks_getCallCount();
    uint32 base_ti_cfg       = mock_IfxGtm_Tom_Timer_initConfig_getCallCount();
    uint32 base_ti_init      = mock_IfxGtm_Tom_Timer_init_getCallCount();
    uint32 base_ph_cfg       = mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount();
    uint32 base_ph_init      = mock_IfxGtm_Tom_PwmHl_init_getCallCount();
    uint32 base_run          = mock_IfxGtm_Tom_Timer_run_getCallCount();

    /* Second init should not reconfigure hardware */
    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_enable,  mock_IfxGtm_enable_getCallCount(), "Second init must not call IfxGtm_enable again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_clk,     mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Second init must not call CMU enableClocks again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_ti_cfg,  mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Second init must not call Timer initConfig again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_ti_init, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Second init must not call Timer init again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_ph_cfg,  mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "Second init must not call PwmHl initConfig again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_ph_init, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "Second init must not call PwmHl init again");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_run,     mock_IfxGtm_Tom_Timer_run_getCallCount(), "Second init must not call Timer run again");
}

/* ========================= GROUP 2 - initGtmTom3phInv: configuration values ========================= */
void test_TC_G2_001_init_config_values_match_requirements(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    /* PwmHl config captures */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNEL_PAIRS, mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels, "initConfig must set 3 channel pairs");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ,  mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency,  "initConfig must set frequency to 20 kHz");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNEL_PAIRS, mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "PwmHl init must use 3 channel pairs");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ,  mock_IfxGtm_Tom_PwmHl_init_lastFrequency,  "PwmHl init must use frequency 20 kHz");

    /* Mode and timer frequency update */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "PwmHl setMode must be called once (center-aligned)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_updateInputFrequency_getCallCount(), "Timer input frequency must be updated once");
}

void test_TC_G2_002_init_sets_initial_duty_and_deadtime_correctly(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Initial setOnTime must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_DUTY_TICKS_U, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0], "Phase U on-time must be 25% of period");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_DUTY_TICKS_V, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1], "Phase V on-time must be 50% of period");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_INIT_DUTY_TICKS_W, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2], "Phase W on-time must be 75% of period");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[0],  "Rising dead-time must be configured to 0.5us");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[1],  "Rising dead-time must be configured to 0.5us");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[2],  "Rising dead-time must be configured to 0.5us");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[0], "Falling dead-time must be configured to 0.5us");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[1], "Falling dead-time must be configured to 0.5us");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[2], "Falling dead-time must be configured to 0.5us");
}

/* ========================= GROUP 3 - initGtmTom3phInv: runtime update logic (init sequence) ========================= */
void test_TC_G3_001_init_performs_single_shadow_transfer_sequence(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "Init must disable updates once before initial write");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "Init must write initial duties once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "Init must apply update once to latch values");
}

void test_TC_G3_002_init_configures_pin_mapping_for_six_outputs(void)
{
    arrange_successful_init_returns();

    initGtmTom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_TOM_PINS, mock_IfxGtm_PinMap_setTomTout_getCallCount(), "Pin map must be configured for 6 TOM outputs (3 complementary pairs)");
}

/* ========================= GROUP 4 - updateGtmTom3phInvDuty: initialization / enable guard ========================= */
void test_TC_G4_001_update_without_init_performs_no_hw_writes(void)
{
    /* Do NOT call init here */
    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "Update without init must not disable updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "Update without init must not set on-time");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "Update without init must not apply update");

    /* Ensure no lazy init side-effects */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(),   "Update must not call Timer initConfig implicitly");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxGtm_Tom_Timer_init_getCallCount(),         "Update must not call Timer init implicitly");
}

void test_TC_G4_002_update_after_init_calls_update_sequence_once(void)
{
    arrange_successful_init_returns();
    initGtmTom3phInv();

    /* Baseline counts after init (init already performed a shadow transfer once) */
    uint32 base_disable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32 base_set     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32 base_apply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_disable + 1u, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "Update must disable updates exactly once per call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_set     + 1u, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "Update must write duties exactly once per call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(base_apply   + 1u, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "Update must apply update exactly once per call");
}

/* ========================= GROUP 5 - updateGtmTom3phInvDuty: configuration values ========================= */
void test_TC_G5_001_update_keeps_frequency_and_channel_config_constant(void)
{
    arrange_successful_init_returns();
    initGtmTom3phInv();

    uint32 cfg_num_pairs_before = mock_IfxGtm_Tom_PwmHl_init_lastNumChannels;
    uint32 cfg_freq_before      = mock_IfxGtm_Tom_PwmHl_init_lastFrequency;

    updateGtmTom3phInvDuty();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(cfg_num_pairs_before, mock_IfxGtm_Tom_PwmHl_init_lastNumChannels, "Update must not change number of channel pairs");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(cfg_freq_before,      mock_IfxGtm_Tom_PwmHl_init_lastFrequency,   "Update must not change configured PWM frequency");
}

void test_TC_G5_002_update_enforces_min_and_max_pulse_limits(void)
{
    arrange_successful_init_returns();
    initGtmTom3phInv();

    /* Perform multiple updates and verify duty bounds on every write */
    for (int iter = 0; iter < 50; ++iter)
    {
        updateGtmTom3phInvDuty();
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] >= UT_MINPULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0] <= (UT_PERIOD_TICKS - UT_MINPULSE_TICKS)), "Phase U duty must remain within min/max pulse limits");
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] >= UT_MINPULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1] <= (UT_PERIOD_TICKS - UT_MINPULSE_TICKS)), "Phase V duty must remain within min/max pulse limits");
        TEST_ASSERT_TRUE_MESSAGE((mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2] >= UT_MINPULSE_TICKS) && (mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2] <= (UT_PERIOD_TICKS - UT_MINPULSE_TICKS)), "Phase W duty must remain within min/max pulse limits");
    }
}

/* ========================= GROUP 6 - updateGtmTom3phInvDuty: runtime update logic ========================= */
void test_TC_G6_001_update_increments_duty_when_below_boundary(void)
{
    arrange_successful_init_returns();
    initGtmTom3phInv();

    uint32 prevU = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0];
    uint32 prevV = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1];
    uint32 prevW = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];

    /* Perform up to 100 updates to observe a positive increment without wrap */
    bool increasedU = false, increasedV = false, increasedW = false;
    for (int i = 0; i < 100; ++i)
    {
        updateGtmTom3phInvDuty();
        uint32 curU = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[0];
        uint32 curV = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[1];
        uint32 curW = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];
        if (!increasedU && (curU > prevU)) increasedU = true;
        if (!increasedV && (curV > prevV)) increasedV = true;
        if (!increasedW && (curW > prevW)) increasedW = true;
        prevU = curU; prevV = curV; prevW = curW;
        if (increasedU && increasedV && increasedW) break;
    }

    TEST_ASSERT_TRUE_MESSAGE(increasedU, "Phase U duty must increment when below wrap boundary");
    TEST_ASSERT_TRUE_MESSAGE(increasedV, "Phase V duty must increment when below wrap boundary");
    TEST_ASSERT_TRUE_MESSAGE(increasedW, "Phase W duty must increment when below wrap boundary");
}

void test_TC_G6_002_update_wraps_duty_when_exceeding_max_and_uses_single_shadow_transfer_per_call(void)
{
    arrange_successful_init_returns();
    initGtmTom3phInv();

    /* Baseline call counts after init */
    uint32 base_disable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32 base_set     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32 base_apply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    /* Drive W channel past the max threshold to force wrap to min threshold */
    uint32 prevW = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];
    bool wrappedW = false;
    const int max_iterations = 6000; /* Large enough to guarantee wrap regardless of step size */
    for (int i = 0; i < max_iterations; ++i)
    {
        updateGtmTom3phInvDuty();
        uint32 curW = mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[2];
        if (curW < prevW)
        {
            wrappedW = true;
            TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_MINPULSE_TICKS, curW, "Phase W duty must wrap to minimum pulse threshold on overflow");
            break;
        }
        prevW = curW;
    }

    TEST_ASSERT_TRUE_MESSAGE(wrappedW, "Phase W duty must eventually wrap when exceeding maximum threshold");

    /* Verify exactly one shadow-transfer sequence per update call across the loop */
    uint32 updates_performed = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount() - base_set;
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(updates_performed, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount() - base_disable, "disableUpdate must be called exactly once per update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(updates_performed, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount()   - base_apply,   "applyUpdate must be called exactly once per update");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_gtm_and_clocks_once);
    RUN_TEST(test_TC_G1_002_init_is_idempotent_no_extra_calls_on_second_invocation);

    RUN_TEST(test_TC_G2_001_init_config_values_match_requirements);
    RUN_TEST(test_TC_G2_002_init_sets_initial_duty_and_deadtime_correctly);

    RUN_TEST(test_TC_G3_001_init_performs_single_shadow_transfer_sequence);
    RUN_TEST(test_TC_G3_002_init_configures_pin_mapping_for_six_outputs);

    RUN_TEST(test_TC_G4_001_update_without_init_performs_no_hw_writes);
    RUN_TEST(test_TC_G4_002_update_after_init_calls_update_sequence_once);

    RUN_TEST(test_TC_G5_001_update_keeps_frequency_and_channel_config_constant);
    RUN_TEST(test_TC_G5_002_update_enforces_min_and_max_pulse_limits);

    RUN_TEST(test_TC_G6_001_update_increments_duty_when_below_boundary);
    RUN_TEST(test_TC_G6_002_update_wraps_duty_when_exceeding_max_and_uses_single_shadow_transfer_per_call);

    return UNITY_END();
}
