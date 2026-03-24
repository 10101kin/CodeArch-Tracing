#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"

// iLLD mock control headers
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

// Helper: convert on-time ticks to duty percent using current period
static float32 to_percent(float32 onTime, float32 period)
{
    if (period <= 0.0f)
    {
        return 0.0f;
    }
    return (onTime / period) * 100.0f;
}

void setUp(void)
{
    // Reset all mocks before each test
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Tom_PwmHl_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) INIT: Verify init calls all key driver APIs that have available call-count controls
void test_initGtmTom3phInv_CallsExpectedDriverAPIs(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);

    // Act
    initGtmTom3phInv();

    // Assert - verify expected driver calls (only those with available GetCallCount controls)
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);
}

// 2) INIT: Verify initial PWM configuration values (initial duties, frequency if exposed)
void test_initGtmTom3phInv_SetsExpectedConfigValues(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);

    // Act
    initGtmTom3phInv();

    // Assert initial duty percentages using the module's extern-visible state
    // Using structured configuration values: U=25%, V=50%, W=75%
    // Derive duty percent from on-time ticks and current timer period
    float32 period = (float32)g_pwm3PhaseOutput.timer.base.period;
    float32 du = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[0], period);
    float32 dv = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[1], period);
    float32 dw = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[2], period);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 25.0f, du); // PIN start U=25%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f, dv); // PIN start V=50%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f, dw); // PIN start W=75%

    // Optional: verify configured PWM frequency if the driver exposes it via timer base
    // This uses TIMING_PWM_FREQUENCY_HZ from structured values
    TEST_ASSERT_FLOAT_WITHIN(1.0f, (float32)TIMING_PWM_FREQUENCY_HZ, (float32)g_pwm3PhaseOutput.timer.base.frequency);
}

// 3) UPDATE: Single call advances duties by +10% (step) with synchronous update intent
void test_updateGtmTom3phInvDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
    initGtmTom3phInv();

    // Act: one update → add TIMING_DUTY_STEP_PERCENT to each phase
    updateGtmTom3phInvDuty();

    // Assert
    float32 period = (float32)g_pwm3PhaseOutput.timer.base.period;
    float32 du = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[0], period);
    float32 dv = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[1], period);
    float32 dw = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[2], period);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 25.0f + (float32)TIMING_DUTY_STEP_PERCENT, du); // 35%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f + (float32)TIMING_DUTY_STEP_PERCENT, dv); // 60%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f + (float32)TIMING_DUTY_STEP_PERCENT, dw); // 85%
}

// 4) UPDATE: Two calls progress duties by 2 * step
void test_updateGtmTom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
    initGtmTom3phInv();

    // Act
    updateGtmTom3phInvDuty(); // +10%
    updateGtmTom3phInvDuty(); // +10% again

    // Assert
    float32 period = (float32)g_pwm3PhaseOutput.timer.base.period;
    float32 du = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[0], period);
    float32 dv = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[1], period);
    float32 dw = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[2], period);

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 25.0f + 2.0f * (float32)TIMING_DUTY_STEP_PERCENT, du); // 45%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 50.0f + 2.0f * (float32)TIMING_DUTY_STEP_PERCENT, dv); // 70%
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 75.0f + 2.0f * (float32)TIMING_DUTY_STEP_PERCENT, dw); // 95%
}

// Helper: compute expected percent after N updates with wrap in [0, 100)
static float32 expected_percent_after_N(float32 initialPct, uint32 n)
{
    float32 pct = initialPct;
    for (uint32 i = 0; i < n; ++i)
    {
        pct += (float32)TIMING_DUTY_STEP_PERCENT;
        if (pct >= 100.0f) // wrap per requirement 0..100%
        {
            pct = 0.0f;
        }
    }
    return pct;
}

// 5) UPDATE: Boundary wrap-around behavior
void test_updateGtmTom3phInvDuty_BoundaryWrapAround(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
    initGtmTom3phInv();

    // Perform N updates such that all phases have wrapped at least once.
    // From 25/50/75 with step=10, N=8 ensures U wraps, V and W also wrap.
    const uint32 N = 8u;
    for (uint32 i = 0; i < N; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    // Assert against analytically computed expectations using config values
    float32 period = (float32)g_pwm3PhaseOutput.timer.base.period;
    float32 du = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[0], period);
    float32 dv = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[1], period);
    float32 dw = to_percent((float32)g_pwm3PhaseOutput.pwmOnTimes[2], period);

    float32 expU = expected_percent_after_N(25.0f, N); // → 0%
    float32 expV = expected_percent_after_N(50.0f, N); // → 20%
    float32 expW = expected_percent_after_N(75.0f, N); // → 50%

    TEST_ASSERT_FLOAT_WITHIN(1.0f, expU, du);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, expV, dv);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, expW, dw);

    // Additionally, verify wrapped values are within one step of 0% (low range)
    TEST_ASSERT_TRUE(du <= (float32)TIMING_DUTY_STEP_PERCENT);
}

// 6) UPDATE: Ensure update does not re-initialize drivers
void test_updateGtmTom3phInvDuty_DoesNotReInit(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
    initGtmTom3phInv();

    // Snapshot init-related call counts right after init
    uint32 cc_gtm_enable_init = IfxGtm_Mock_GetCallCount_enable();
    uint32 cc_cmu_enclk_init = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 cc_timer_init_init = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 cc_pwmhl_init_init = IfxGtm_Tom_PwmHl_Mock_GetCallCount_init();

    // Act: perform several updates
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    // Assert: no re-initialization occurred (counts unchanged)
    TEST_ASSERT_EQUAL(cc_gtm_enable_init, IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL(cc_cmu_enclk_init, IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL(cc_timer_init_init, IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL(cc_pwmhl_init_init, IfxGtm_Tom_PwmHl_Mock_GetCallCount_init());
}
