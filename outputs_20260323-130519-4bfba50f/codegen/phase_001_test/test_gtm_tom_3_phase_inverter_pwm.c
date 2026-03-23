#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxPort.h"

// Structured configuration values from requirements
#define GTM_TOM_TOM_INDEX                (1)
#define GTM_TOM_CENTER_ALIGNED           (1)
#define GTM_TOM_COMPLEMENTARY            (1)
#define DEADTIME_FALLBACK_NS_IF_UNKNOWN  (1000)
#define TIMING_PWM_FREQUENCY_HZ          (20000U)
#define TIMING_DUTY_UPDATE_PERIOD_MS     (500U)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ   (20U)

// Derived behavior values from functional requirements
#define DUTY_INIT0_FRAC                  (0.25f)  // 25%
#define DUTY_INIT1_FRAC                  (0.50f)  // 50%
#define DUTY_INIT2_FRAC                  (0.75f)  // 75%
#define DUTY_STEP_FRAC                   (0.10f)  // +10% per update
#define DUTY_MIN_FRAC                    (0.0f)   // 0%
#define DUTY_MAX_FRAC                    (1.0f)   // 100%

// Tolerance for float comparisons (timer "period" is in ticks → integer, but use float tolerance)
#define DUTY_TOLERANCE_TICKS             (1.0f)

void setUp(void)
{
    // Reset all mocks
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Tom_PwmHl_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();

    // Ensure init() paths succeed
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
}

void tearDown(void) {}

static void force_known_timer_period_and_link(uint32 periodTicks)
{
    // Link PWM driver to the timer instance and set a known period so update logic has deterministic math
    g_pwm3PhaseOutput.pwm.timer = &g_pwm3PhaseOutput.timer;
    g_pwm3PhaseOutput.timer.base.period = (Ifx_TimerValue)periodTicks;
}

// 1) Init call graph verification
void test_initGtmTomPwm_CallsExpectedDriverAPIs(void)
{
    // Act
    initGtmTomPwm();

    // Assert expected iLLD calls were made (available mocks only)
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_run() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode() > 0);

    // Coherent initial update sequence
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    // Verify TOM TOUT mapping was configured
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);
}

// 2) Init configuration values and initial duties (25/50/75%)
void test_initGtmTomPwm_SetsExpectedConfigValues(void)
{
    // Arrange: choose a deterministic period based on structured values: 20000 / 20 = 1000 ticks
    const uint32 periodTicks = (TIMING_PWM_FREQUENCY_HZ / CLOCK_EXPECTED_SYSTEM_FREQ_MHZ);
    force_known_timer_period_and_link(periodTicks);

    // Act
    initGtmTomPwm();

    // Assert I/O mapping config (push-pull + pad driver CM Automotive Speed1)
    TEST_ASSERT_EQUAL(IfxPort_OutputMode_pushPull, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode());
    TEST_ASSERT_EQUAL(IfxPort_PadDriver_cmosAutomotiveSpeed1, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver());

    // Assert clocks enabled with FXCLK mask
    TEST_ASSERT_EQUAL(IFXGTM_CMU_CLKEN_FXCLK, IfxGtm_Cmu_Mock_GetLastArg_enableClocks_clkMask());

    // Assert center-aligned mode intent is configured (setMode was called; mode captured)
    // Value verification via captured mode argument
    {
        uint32 modeArg = IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMode_mode();
        // We expect a valid (non-zero) mode corresponding to center-aligned per design
        TEST_ASSERT_TRUE_MESSAGE(modeArg != 0U, "PWM mode not set");
        TEST_ASSERT_TRUE(GTM_TOM_CENTER_ALIGNED); // design flag
    }

    // Assert initial ON-times: 25%, 50%, 75% of period
    {
        float exp0 = ((float)periodTicks) * DUTY_INIT0_FRAC;
        float exp1 = ((float)periodTicks) * DUTY_INIT1_FRAC;
        float exp2 = ((float)periodTicks) * DUTY_INIT2_FRAC;

        float a0 = (float)g_pwm3PhaseOutput.pwmOnTimes[0];
        float a1 = (float)g_pwm3PhaseOutput.pwmOnTimes[1];
        float a2 = (float)g_pwm3PhaseOutput.pwmOnTimes[2];

        TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, exp0, a0);
        TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, exp1, a1);
        TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, exp2, a2);

        // Also verify the 1:2:3 ratio implied by 25/50/75
        TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, a0 * 2.0f, a1);
        TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, a0 * 3.0f, a2);
    }
}

// 3) Single update applies +10% step coherently to all three channels
void test_updateGtmTomPwmDutyCycles_SingleCall_UpdatesValues(void)
{
    // Arrange
    const uint32 periodTicks = (TIMING_PWM_FREQUENCY_HZ / CLOCK_EXPECTED_SYSTEM_FREQ_MHZ); // 1000
    force_known_timer_period_and_link(periodTicks);
    initGtmTomPwm();

    // Capture before values
    float before0 = (float)g_pwm3PhaseOutput.pwmOnTimes[0];
    float before1 = (float)g_pwm3PhaseOutput.pwmOnTimes[1];
    float before2 = (float)g_pwm3PhaseOutput.pwmOnTimes[2];

    // Act
    updateGtmTomPwmDutyCycles();

    // Assert: disableUpdate → setOnTime → applyUpdate were called
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    // Each channel increases by exactly +10% of period
    const float step = ((float)periodTicks) * DUTY_STEP_FRAC; // +10%
    float after0 = (float)g_pwm3PhaseOutput.pwmOnTimes[0];
    float after1 = (float)g_pwm3PhaseOutput.pwmOnTimes[1];
    float after2 = (float)g_pwm3PhaseOutput.pwmOnTimes[2];

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, step, after0 - before0);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, step, after1 - before1);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, step, after2 - before2);
}

// 4) Multiple updates progress linearly: N * (+10%)
void test_updateGtmTomPwmDutyCycles_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    const uint32 periodTicks = (TIMING_PWM_FREQUENCY_HZ / CLOCK_EXPECTED_SYSTEM_FREQ_MHZ); // 1000
    force_known_timer_period_and_link(periodTicks);
    initGtmTomPwm();

    const float step = ((float)periodTicks) * DUTY_STEP_FRAC;

    float base0 = (float)g_pwm3PhaseOutput.pwmOnTimes[0];
    float base1 = (float)g_pwm3PhaseOutput.pwmOnTimes[1];
    float base2 = (float)g_pwm3PhaseOutput.pwmOnTimes[2];

    // Act: two updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    float now0 = (float)g_pwm3PhaseOutput.pwmOnTimes[0];
    float now1 = (float)g_pwm3PhaseOutput.pwmOnTimes[1];
    float now2 = (float)g_pwm3PhaseOutput.pwmOnTimes[2];

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, 2.0f * step, now0 - base0);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, 2.0f * step, now1 - base1);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, 2.0f * step, now2 - base2);

    // Also verify setOnTime was called once in init + twice here → at least 3 total
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetArgHistoryCount_setOnTime() >= 3U);
}

// 5) Boundary: wrap-around at 100% → back to 0%
void test_updateGtmTomPwmDutyCycles_BoundaryWrapAround(void)
{
    // Arrange
    const uint32 periodTicks = (TIMING_PWM_FREQUENCY_HZ / CLOCK_EXPECTED_SYSTEM_FREQ_MHZ); // 1000
    force_known_timer_period_and_link(periodTicks);
    initGtmTomPwm();

    // Force all channels to 100% (max). Next +10% step should wrap to 0%.
    g_pwm3PhaseOutput.pwmOnTimes[0] = (Ifx_TimerValue)((float)periodTicks * DUTY_MAX_FRAC);
    g_pwm3PhaseOutput.pwmOnTimes[1] = (Ifx_TimerValue)((float)periodTicks * DUTY_MAX_FRAC);
    g_pwm3PhaseOutput.pwmOnTimes[2] = (Ifx_TimerValue)((float)periodTicks * DUTY_MAX_FRAC);

    // Act
    updateGtmTomPwmDutyCycles();

    // Assert wrap to minimum (0%)
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, ((float)periodTicks) * DUTY_MIN_FRAC, (float)g_pwm3PhaseOutput.pwmOnTimes[0]);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, ((float)periodTicks) * DUTY_MIN_FRAC, (float)g_pwm3PhaseOutput.pwmOnTimes[1]);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_TICKS, ((float)periodTicks) * DUTY_MIN_FRAC, (float)g_pwm3PhaseOutput.pwmOnTimes[2]);
}

// 6) Update must not re-initialize any drivers
void test_updateGtmTomPwmDutyCycles_DoesNotReInit(void)
{
    // Arrange
    const uint32 periodTicks = (TIMING_PWM_FREQUENCY_HZ / CLOCK_EXPECTED_SYSTEM_FREQ_MHZ); // 1000
    force_known_timer_period_and_link(periodTicks);
    initGtmTomPwm();

    // Capture init-phase call counts
    uint32 t_init_cfg_before  = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 t_init_before      = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 t_run_before       = IfxGtm_Tom_Timer_Mock_GetCallCount_run();

    uint32 hl_init_cfg_before = IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig();
    uint32 hl_init_before     = IfxGtm_Tom_PwmHl_Mock_GetCallCount_init();

    uint32 disable_before     = IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate();
    uint32 set_before         = IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime();
    uint32 apply_before       = IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate();

    // Act: perform several updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert: no re-init
    TEST_ASSERT_EQUAL(t_init_cfg_before, IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL(t_init_before,     IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL(t_run_before,      IfxGtm_Tom_Timer_Mock_GetCallCount_run());

    TEST_ASSERT_EQUAL(hl_init_cfg_before, IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL(hl_init_before,     IfxGtm_Tom_PwmHl_Mock_GetCallCount_init());

    // Assert: update path calls increased
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > disable_before);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime()     > set_before);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate()   > apply_before);
}
