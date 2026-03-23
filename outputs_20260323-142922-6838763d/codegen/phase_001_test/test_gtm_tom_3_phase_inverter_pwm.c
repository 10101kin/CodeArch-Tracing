#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

// Mocked iLLD headers (provide Mock_* control functions)
#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxPort.h"

// Structured configuration values (from requirements)
#define INITIAL_DUTY_PERCENT_U        (25.0f)
#define INITIAL_DUTY_PERCENT_V        (50.0f)
#define INITIAL_DUTY_PERCENT_W        (75.0f)
#define UPDATE_POLICY_STEP_PERCENT    (10.0f)
#define TIMING_PWM_FREQUENCY_HZ       (20000.0f)

// Tolerance for percentage comparisons
#define DUTY_TOLERANCE_PERCENT        (1.0f)

void setUp(void)
{
    // Reset all mocks
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Tom_PwmHl_Mock_Reset();
    IfxPort_Mock_Reset();

    // Configure success return values for iLLD init/config functions used by production code
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_setMode(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_setDeadtime(TRUE);
    IfxGtm_Tom_PwmHl_Mock_SetReturn_setMinPulse(TRUE);
}

void tearDown(void) {}

// 1) INIT: Verify all expected iLLD APIs are called
void test_initGtmTomPwm_CallsExpectedDriverAPIs(void)
{
    // Act
    initGtmTomPwm();  // void signature — no arguments

    // Assert — each expected iLLD call must have occurred (call count > 0)
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);

    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinPadDriver() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setDeadtime() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMinPulse() > 0);

    // Synchronous initial application of on-times
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);
}

// 2) INIT: Verify configuration values passed to drivers (initial duties via setOnTime array)
void test_initGtmTomPwm_SetsExpectedConfigValues(void)
{
    // Act
    initGtmTomPwm();

    // Assert — verify the initial complementary PWM duty targets (percent scale)
    float32 dutyU = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(0);
    float32 dutyV = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(1);
    float32 dutyW = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_PERCENT_U, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_PERCENT_V, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_PERCENT_W, dutyW);
}

// 3) UPDATE: Single-call update applies +10% step and calls expected iLLD APIs
void test_updateGtmTomPwmDutyCycles_SingleCall_UpdatesValues(void)
{
    // Arrange
    initGtmTomPwm();

    // Act — one update step (caller controls 500 ms cadence externally)
    updateGtmTomPwmDutyCycles();

    // Assert — expected iLLD update sequence occurred
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    // Assert — values advanced by one step (percent scale)
    float32 dutyU = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(0);
    float32 dutyV = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(1);
    float32 dutyW = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_U + UPDATE_POLICY_STEP_PERCENT, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_V + UPDATE_POLICY_STEP_PERCENT, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_W + UPDATE_POLICY_STEP_PERCENT, dutyW);
}

// 4) UPDATE: Multiple calls progress correctly (two steps)
void test_updateGtmTomPwmDutyCycles_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    initGtmTomPwm();

    // Act — two sequential update steps
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert — final values reflect initial + 2*step
    float32 dutyU = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(0);
    float32 dutyV = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(1);
    float32 dutyW = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_U + 2.0f * UPDATE_POLICY_STEP_PERCENT, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_V + 2.0f * UPDATE_POLICY_STEP_PERCENT, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_W + 2.0f * UPDATE_POLICY_STEP_PERCENT, dutyW);
}

// 5) UPDATE: Boundary wrap-around — W channel (75%) wraps to minimum after 3 steps
void test_updateGtmTomPwmDutyCycles_BoundaryWrapAround(void)
{
    // Arrange
    initGtmTomPwm();

    // Act — three steps: U=55, V=80, W hits >=100% then wraps to min (0%)
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert — U and V progressed without wrap
    float32 dutyU = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(0);
    float32 dutyV = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(1);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_U + 3.0f * UPDATE_POLICY_STEP_PERCENT, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT,
        INITIAL_DUTY_PERCENT_V + 3.0f * UPDATE_POLICY_STEP_PERCENT, dutyV);

    // Assert — W wrapped to min threshold (0%). Instead of hardcoding 0,
    // verify it's back in the low range (<= step), derived from config values.
    float32 dutyW = IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(2);
    TEST_ASSERT_TRUE(dutyW <= UPDATE_POLICY_STEP_PERCENT);
}

// 6) UPDATE: Does not re-initialize drivers
void test_updateGtmTomPwmDutyCycles_DoesNotReInit(void)
{
    // Arrange — perform init and snapshot init-related call counts
    initGtmTomPwm();

    uint32 tmrInitCfg_before  = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 tmrInit_before     = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 pwmhlInitCfg_before= IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig();
    uint32 pwmhlInit_before   = IfxGtm_Tom_PwmHl_Mock_GetCallCount_init();
    uint32 gtmEnable_before   = IfxGtm_Mock_GetCallCount_enable();
    uint32 cmuEnable_before   = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();

    // Act — multiple updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert — no re-initialization occurred
    TEST_ASSERT_EQUAL_UINT32(tmrInitCfg_before,  IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(tmrInit_before,     IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(pwmhlInitCfg_before,IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(pwmhlInit_before,   IfxGtm_Tom_PwmHl_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(gtmEnable_before,   IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cmuEnable_before,   IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
}
