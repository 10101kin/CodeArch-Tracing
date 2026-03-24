#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "Ifx_Types.h"

// Structured configuration values (from requirements)
static const float32 INITIAL_DUTY_PERCENT_U = 25.0f;
static const float32 INITIAL_DUTY_PERCENT_V = 50.0f;
static const float32 INITIAL_DUTY_PERCENT_W = 75.0f;
static const float32 TIMING_PWM_FREQUENCY_HZ = 20000.0f;
static const uint32  TIMING_UPDATE_INTERVAL_MS = 500u; // Caller-controlled timing
// Step delta from user requirement: +10% per update call
static const float32 DUTY_STEP_PERCENT = 10.0f;

void setUp(void)
{
    // Reset all mocks before each test
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();

    // Default successful return for timer init path if production checks it
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
}

void tearDown(void) {}

// 1) Init should call all expected driver APIs
void test_GTM_TOM_3_Phase_Inverter_PWM_init_CallsExpectedDriverAPIs(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // GTM + CMU clocks
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    // Time base configuration using TOM timer
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    // Pin muxing for six PWM outputs
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);

    // Unified multi-channel PWM configuration
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);

    // Initial duties applied atomically and channels started synchronously
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// 2) Init should set expected config values and initial duties
void test_GTM_TOM_3_Phase_Inverter_PWM_init_SetsExpectedConfigValues(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Verify PWM frequency configuration (20 kHz center-aligned)
    TEST_ASSERT_FLOAT_WITHIN(1.0f, TIMING_PWM_FREQUENCY_HZ,
        IfxGtm_Pwm_Mock_GetLastArg_init_frequency());

    // Initial six-channel duty array should reflect complementary pairs:
    // U pair: indices 0,1; V pair: indices 2,3; W pair: indices 4,5
    float32 d0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 d1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 d2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);
    float32 d3 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(3);
    float32 d4 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 d5 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U, d0);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U, d1);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V, d2);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V, d3);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W, d4);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W, d5);
}

// 3) Single update step: duties increase by +10% and are applied atomically
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_SingleCall_UpdatesValues(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Call once: each phase duty should increase by +10%
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();

    float32 d0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 d1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 d2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);
    float32 d3 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(3);
    float32 d4 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 d5 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U + DUTY_STEP_PERCENT, d0);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U + DUTY_STEP_PERCENT, d1);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V + DUTY_STEP_PERCENT, d2);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V + DUTY_STEP_PERCENT, d3);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W + DUTY_STEP_PERCENT, d4);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W + DUTY_STEP_PERCENT, d5);

    // Verify the update API was called for this step
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);
}

// 4) Multiple updates: progression equals initial + N * step
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_MultipleCalls_ProgressesCorrectly(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    GTM_TOM_3_Phase_Inverter_PWM_stepDuty(); // 1st step
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty(); // 2nd step

    float32 d0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 d1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 d2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);
    float32 d3 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(3);
    float32 d4 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 d5 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);

    const float32 N = 2.0f;
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U + N * DUTY_STEP_PERCENT, d0);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_U + N * DUTY_STEP_PERCENT, d1);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V + N * DUTY_STEP_PERCENT, d2);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_V + N * DUTY_STEP_PERCENT, d3);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W + N * DUTY_STEP_PERCENT, d4);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, INITIAL_DUTY_PERCENT_W + N * DUTY_STEP_PERCENT, d5);
}

// 5) Boundary wrap-around: when a phase exceeds 100%, it wraps to 0%
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_BoundaryWrapAround(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Phase W starts at 75% and wraps after 3 steps of +10%: 75 -> 85 -> 95 -> 105 -> wrap to 0
    for (int i = 0; i < 3; ++i)
    {
        GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
    }

    float32 w0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 w1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, w0);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, w1);
}

// 6) Update must not re-run any init/config routines
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_DoesNotReInit(void)
{
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Capture init-time call counts
    uint32 cnt_gtm_enable            = IfxGtm_Mock_GetCallCount_enable();
    uint32 cnt_cmu_enableClocks      = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 cnt_timer_initCfg         = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 cnt_timer_init            = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 cnt_timer_addMask         = IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask();
    uint32 cnt_timer_applyUpdate     = IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate();
    uint32 cnt_pinmap_setTomTout     = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    uint32 cnt_pwm_initCfg           = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 cnt_pwm_initChCfg         = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 cnt_pwm_init              = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 cnt_pwm_startSynced       = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 cnt_pwm_update_before     = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    // Perform several runtime updates
    const uint32 updates = 5u;
    for (uint32 i = 0; i < updates; ++i)
    {
        GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
    }

    // No re-init or re-config calls should occur during updates
    TEST_ASSERT_EQUAL_UINT32(cnt_gtm_enable,        IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cnt_cmu_enableClocks,  IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(cnt_timer_initCfg,     IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_timer_init,        IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(cnt_timer_addMask,     IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask());
    TEST_ASSERT_EQUAL_UINT32(cnt_timer_applyUpdate, IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate());
    TEST_ASSERT_EQUAL_UINT32(cnt_pinmap_setTomTout, IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initCfg,       IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initChCfg,     IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_init,          IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_startSynced,   IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());

    // Only the duty update API should increase by 'updates'
    uint32 cnt_pwm_update_after = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_update_before + updates, cnt_pwm_update_after);
}
