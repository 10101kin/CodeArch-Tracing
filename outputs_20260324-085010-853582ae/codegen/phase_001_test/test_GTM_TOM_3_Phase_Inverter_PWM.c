#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

// Helper to compute positive step across potential wrap-around
static float32 compute_increment_with_wrap(uint32 before, uint32 after, uint32 period)
{
    float32 b = (float32)before;
    float32 a = (float32)after;
    float32 p = (float32)period;
    float32 d = a - b;
    if (d < 0.0f)
    {
        d += p;
    }
    return d;
}

void setUp(void)
{
    // Reset all driver mocks
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Tom_PwmHl_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();

    // Ensure init() calls in production succeed
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
}

void tearDown(void) {}

// 1) INIT: verify all expected driver API calls (only those with available mocks)
void test_initGtmTomPwm_CallsExpectedDriverAPIs(void)
{
    initGtmTomPwm();

    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);

    // Pin routing + pad config
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);

    // Port pad mode (push-pull)
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);

    // PWMHL
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_init() > 0);
}

// 2) INIT: verify configuration values passed to drivers and initial ON-time duties
void test_initGtmTomPwm_SetsExpectedConfigValues(void)
{
    initGtmTomPwm();

    // Frequency configuration on both Timer and PWMHL init paths
    {
        float32 timerFreq = IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency();
        float32 pwmhlFreq = IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_frequency();
        TEST_ASSERT_EQUAL_FLOAT((float32)TIMING_FREQUENCY_HZ, timerFreq);
        TEST_ASSERT_EQUAL_FLOAT((float32)TIMING_FREQUENCY_HZ, pwmhlFreq);
    }

    // PWMHL number of channel pairs = 3
    {
        uint32 numChPairs = (uint32)IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_numChannels();
        TEST_ASSERT_EQUAL_UINT32(3u, numChPairs);
    }

    // Pin map output mode and pad driver
    {
        uint32 outMode = (uint32)IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode();
        uint32 padDrv  = (uint32)IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver();
        TEST_ASSERT_EQUAL((uint32)PAD_OUTPUT_MODE, outMode);
        TEST_ASSERT_EQUAL((uint32)PAD_PAD_DRIVER, padDrv);
    }

    // Also verify the last Port mode set was push-pull
    {
        uint32 portMode = (uint32)IfxPort_Mock_GetLastArg_setPinModeOutput_mode();
        TEST_ASSERT_EQUAL((uint32)PAD_OUTPUT_MODE, portMode);
    }

    // Verify initial ON-times correspond to 25% / 50% / 75% of the configured period
    {
        // Access production driver state (extern provided by the production header)
        // Expect timer period to be non-zero after init
        uint32 periodTicks = (uint32)g_pwm3PhaseOutput.pwm.timer->base.period;
        TEST_ASSERT_TRUE(periodTicks > 0u);

        float32 periodF = (float32)periodTicks;
        float32 dutyU = ((float32)g_pwm3PhaseOutput.pwmOnTimes[0]) / periodF;
        float32 dutyV = ((float32)g_pwm3PhaseOutput.pwmOnTimes[1]) / periodF;
        float32 dutyW = ((float32)g_pwm3PhaseOutput.pwmOnTimes[2]) / periodF;

        TEST_ASSERT_FLOAT_WITHIN(0.01f, (float32)INITIAL_DUTY_CYCLES_PHASE_U, dutyU);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, (float32)INITIAL_DUTY_CYCLES_PHASE_V, dutyV);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, (float32)INITIAL_DUTY_CYCLES_PHASE_W, dutyW);
    }
}

// 3) UPDATE (periodic ramp): single call applies one coherent step to all channels
void test_IfxGtm_Tom_PwmHl_setOnTime_SingleCall_UpdatesValues(void)
{
    initGtmTomPwm();

    uint32 period = (uint32)g_pwm3PhaseOutput.pwm.timer->base.period;
    TEST_ASSERT_TRUE(period > 0u);

    uint32 u0 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[0];
    uint32 v0 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[1];
    uint32 w0 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[2];

    // Act
    IfxGtm_Tom_PwmHl_setOnTime();

    uint32 u1 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[0];
    uint32 v1 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[1];
    uint32 w1 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[2];

    // Compute effective increments accounting for wrap-around
    float32 du = compute_increment_with_wrap(u0, u1, period);
    float32 dv = compute_increment_with_wrap(v0, v1, period);
    float32 dw = compute_increment_with_wrap(w0, w1, period);

    // All three channels should step by the same positive increment
    TEST_ASSERT_TRUE(du > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, du, dv);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, du, dw);
}

// 4) UPDATE (periodic ramp): multiple calls progress by a consistent step size
void test_IfxGtm_Tom_PwmHl_setOnTime_MultipleCalls_ProgressesCorrectly(void)
{
    initGtmTomPwm();

    uint32 period = (uint32)g_pwm3PhaseOutput.pwm.timer->base.period;
    TEST_ASSERT_TRUE(period > 0u);

    uint32 u0 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[0];

    IfxGtm_Tom_PwmHl_setOnTime(); // 1st step
    uint32 u1 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[0];

    IfxGtm_Tom_PwmHl_setOnTime(); // 2nd step
    uint32 u2 = (uint32)g_pwm3PhaseOutput.pwmOnTimes[0];

    float32 d1 = compute_increment_with_wrap(u0, u1, period);
    float32 d2 = compute_increment_with_wrap(u1, u2, period);

    // Step size should be consistent
    TEST_ASSERT_TRUE(d1 > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, d1, d2);
}

// 5) UPDATE boundary: values wrap around to minimum after reaching/exceeding max
void test_IfxGtm_Tom_PwmHl_setOnTime_BoundaryWrapAround(void)
{
    initGtmTomPwm();

    uint32 period = (uint32)g_pwm3PhaseOutput.pwm.timer->base.period;
    TEST_ASSERT_TRUE(period > 0u);

    // Drive updates until any channel wraps (detect decrease without normalization)
    uint32 prev = (uint32)g_pwm3PhaseOutput.pwmOnTimes[2]; // start from W (highest initial duty)
    bool wrapped = false;

    // Take the first step to establish a non-zero increment
    IfxGtm_Tom_PwmHl_setOnTime();

    for (int i = 0; i < 200 && !wrapped; ++i)
    {
        IfxGtm_Tom_PwmHl_setOnTime();
        uint32 cur = (uint32)g_pwm3PhaseOutput.pwmOnTimes[2];
        if (cur < prev)
        {
            wrapped = true;
        }
        prev = cur;
    }

    TEST_ASSERT_TRUE(wrapped);

    // After a wrap, duty should be back near the minimum band. It must be below the
    // smallest initial duty (25%), which uses a value from the structured config.
    float32 dutyAfterWrap = ((float32)g_pwm3PhaseOutput.pwmOnTimes[2]) / (float32)period;
    TEST_ASSERT_TRUE(dutyAfterWrap <= (float32)INITIAL_DUTY_CYCLES_PHASE_U);
}

// 6) UPDATE path must not re-initialize any drivers
void test_IfxGtm_Tom_PwmHl_setOnTime_DoesNotReInit(void)
{
    initGtmTomPwm();

    // Snapshot init-related call counts immediately after init
    uint32 t_initCfg_before  = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 t_init_before     = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 p_initCfg_before  = IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig();
    uint32 p_init_before     = IfxGtm_Tom_PwmHl_Mock_GetCallCount_init();
    uint32 pm_set_before     = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    uint32 port_mode_before  = IfxPort_Mock_GetCallCount_setPinModeOutput();

    // Perform several updates
    for (int i = 0; i < 5; ++i)
    {
        IfxGtm_Tom_PwmHl_setOnTime();
    }

    // Verify none of the init-phase APIs were re-called
    TEST_ASSERT_EQUAL_UINT32(t_initCfg_before, IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(t_init_before,    IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(p_initCfg_before, IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(p_init_before,    IfxGtm_Tom_PwmHl_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(pm_set_before,    IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL_UINT32(port_mode_before, IfxPort_Mock_GetCallCount_setPinModeOutput());
}
