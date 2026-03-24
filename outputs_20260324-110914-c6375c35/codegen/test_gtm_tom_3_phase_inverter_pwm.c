#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "Ifx_Types.h"

// Structured configuration values (from requirements)
#define INITIAL_DUTY_PERCENT_U   (25.0f)
#define INITIAL_DUTY_PERCENT_V   (50.0f)
#define INITIAL_DUTY_PERCENT_W   (75.0f)
#define TIMING_PWM_FREQUENCY_HZ  (20000.0f)
#define TIMING_UPDATE_INTERVAL_MS (500U)

static const float32 kDutyTol = 1.0f;  // tolerance for percent comparisons

void setUp(void)
{
    // Reset all driver mocks
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Init must call all expected iLLD APIs
void test_GTM_TOM_3_Phase_Inverter_PWM_init_CallsExpectedDriverAPIs(void)
{
    // Act
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Assert: Each expected iLLD call was made (GetCallCount > 0)
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// 2) Init should set initial duties and apply them atomically across 6 channels (U,U,V,V,W,W)
void test_GTM_TOM_3_Phase_Inverter_PWM_init_SetsExpectedConfigValues(void)
{
    // Act
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Assert: Initial six-channel duty array reflects U=25, V=50, W=75 (each duplicated for complementary pairs)
    float32 ch0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0); // U+
    float32 ch1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1); // U-
    float32 ch2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2); // V+
    float32 ch3 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(3); // V-
    float32 ch4 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4); // W+
    float32 ch5 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5); // W-

    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_U, ch0);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_U, ch1);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_V, ch2);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_V, ch3);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_W, ch4);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_W, ch5);
}

// 3) Single update applies one positive step to all three phases atomically (pairs remain equal)
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();
    uint32 initCalls = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate();
    TEST_ASSERT_TRUE(initCalls >= 1);

    // Baseline (after init)
    float32 u0 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 0);
    float32 v0 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 2);
    float32 w0 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 4);

    // Act - one step
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();

    // Capture latest (after 1st update)
    uint32 cnt = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate();
    TEST_ASSERT_TRUE(cnt >= 2);
    float32 u1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(cnt - 1, 0);
    float32 v1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(cnt - 1, 2);
    float32 w1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(cnt - 1, 4);

    // Derived step from actual driver call history (no hardcoded step)
    float32 stepMeasured = u1 - u0;

    // Verify numeric correctness vs known initial values for U and V
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_U + stepMeasured, u1);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_V + stepMeasured, v1);

    // Verify all complementary pairs remain equal on the last call
    float32 u1_a = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 u1_b = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 v1_a = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);
    float32 v1_b = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(3);
    float32 w1_a = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 w1_b = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);

    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, u1_a, u1_b);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, v1_a, v1_b);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, w1_a, w1_b);
}

// 4) Multiple updates progress by a consistent step each call
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Act - two update steps
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();

    // History: [0]=init, [1]=after 1st update, [2]=after 2nd update
    uint32 histCount = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate();
    TEST_ASSERT_TRUE(histCount >= 3);

    float32 u0 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 0);
    float32 u1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1, 0);
    float32 u2 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(2, 0);

    float32 v0 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 2);
    float32 v1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1, 2);
    float32 v2 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(2, 2);

    float32 stepMeasured = u1 - u0;

    // After 2 updates: expected = initial + 2*step
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_U + 2.0f * stepMeasured, u2);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_V + 2.0f * stepMeasured, v2);

    // Progress per step is consistent
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, stepMeasured, u2 - u1);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, stepMeasured, v2 - v1);
}

// 5) Boundary: wrap-around when a phase exceeds 100% (W starts at 75% so it should wrap first)
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_BoundaryWrapAround(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Track W phase (index 4) through successive updates until it decreases (wrap detected)
    float32 prevW = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0, 4);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, INITIAL_DUTY_PERCENT_W, prevW);

    float32 wrappedW = prevW;
    int wrappedAt = -1;

    // Sufficient iterations to force wrap for typical step sizes
    for (int i = 0; i < 30; ++i)
    {
        GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
        uint32 cnt = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate();
        float32 curW = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(cnt - 1, 4);
        if (curW < prevW)
        {
            wrappedW = curW;
            wrappedAt = i;
            break;
        }
        prevW = curW;
    }

    TEST_ASSERT_TRUE(wrappedAt >= 0); // wrap detected

    // After wrap, duty should be back near the low range (below initial U=25%)
    TEST_ASSERT_TRUE(wrappedW < INITIAL_DUTY_PERCENT_U);

    // Complementary pair still equal at the wrap point
    float32 w_last_a = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(4);
    float32 w_last_b = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(5);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, w_last_a, w_last_b);
}

// 6) Update must not re-run any init phase APIs
void test_GTM_TOM_3_Phase_Inverter_PWM_stepDuty_DoesNotReInit(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Snapshot init-phase call counts
    uint32 t_initCfg_before   = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 t_init_before      = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 t_mask_before      = IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask();
    uint32 t_apply_before     = IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate();

    uint32 p_initCfg_before   = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 p_chCfg_before     = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 p_init_before      = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 p_start_before     = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 p_update_before    = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    // Act - several updates
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();
    GTM_TOM_3_Phase_Inverter_PWM_stepDuty();

    // Assert: no re-init
    TEST_ASSERT_EQUAL_UINT32(t_initCfg_before, IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(t_init_before,    IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(t_mask_before,    IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask());
    TEST_ASSERT_EQUAL_UINT32(t_apply_before,   IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate());

    TEST_ASSERT_EQUAL_UINT32(p_initCfg_before, IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(p_chCfg_before,   IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(p_init_before,    IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(p_start_before,   IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());

    // Only the immediate duty update API should advance (by 3 more calls)
    TEST_ASSERT_EQUAL_UINT32(p_update_before + 3U, IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate());
}
