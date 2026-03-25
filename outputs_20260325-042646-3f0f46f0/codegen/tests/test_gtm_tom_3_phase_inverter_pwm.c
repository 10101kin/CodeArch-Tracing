#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "Ifx_Types.h"

// Structured configuration values (from requirements)
static const float32 INITIAL_DUTY_PERCENT_U = 25.0f;
static const float32 INITIAL_DUTY_PERCENT_V = 50.0f;
static const float32 INITIAL_DUTY_PERCENT_W = 75.0f;
static const float32 UPDATE_POLICY_INCREMENT_PERCENT = 10.0f;

void setUp(void)
{
    // Reset all mocks before each test
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) init: verify all expected iLLD calls happen
void test_initGtmTomPwm_CallsExpectedDriverAPIs(void)
{
    // Act
    initGtmTomPwm();

    // Assert: GTM and CMU enable
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    // Assert: Pin mapping and GPIO safe-state for unused complements
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0);

    // Assert: Unified PWM driver configuration and start
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// 2) init: verify configuration values applied (initial duty array, sync start)
void test_initGtmTomPwm_SetsExpectedConfigValues(void)
{
    const float32 tol = 1.0f; // percentage tolerance

    // Act
    initGtmTomPwm();

    // The init sequence must apply initial duties via unified immediate update API
    // Verify exactly one update call occurred during init
    TEST_ASSERT_EQUAL_UINT32(1u, IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());

    // Read the last array argument values passed to updateChannelsDutyImmediate
    float32 dutyU = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 dutyV = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 dutyW = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(tol, INITIAL_DUTY_PERCENT_U, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(tol, INITIAL_DUTY_PERCENT_V, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(tol, INITIAL_DUTY_PERCENT_W, dutyW);
}

// 3) update: single-step update applies +10% immediately to all three phases
void test_updateGtmTomPwmDutyCycles_SingleCall_UpdatesValues(void)
{
    const float32 tol = 1.0f; // percentage tolerance

    initGtmTomPwm();

    // Act: one update step (caller controls timing; each call applies one step)
    updateGtmTomPwmDutyCycles();

    // Expected = initial + step
    float32 expU = INITIAL_DUTY_PERCENT_U + UPDATE_POLICY_INCREMENT_PERCENT; // 35
    float32 expV = INITIAL_DUTY_PERCENT_V + UPDATE_POLICY_INCREMENT_PERCENT; // 60
    float32 expW = INITIAL_DUTY_PERCENT_W + UPDATE_POLICY_INCREMENT_PERCENT; // 85

    float32 dutyU = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 dutyV = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 dutyW = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(tol, expU, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(tol, expV, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(tol, expW, dutyW);
}

// 4) update: two consecutive calls increase each channel by exactly one step each time
void test_updateGtmTomPwmDutyCycles_MultipleCalls_ProgressesCorrectly(void)
{
    const float32 tol = 1.0f; // percentage tolerance

    initGtmTomPwm();

    // 1st update
    updateGtmTomPwmDutyCycles();
    float32 u_after1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);

    // 2nd update
    updateGtmTomPwmDutyCycles();
    float32 u_after2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);

    // Step size verification via delta
    float32 delta = u_after2 - u_after1;
    TEST_ASSERT_FLOAT_WITHIN(tol, UPDATE_POLICY_INCREMENT_PERCENT, delta);

    // Arg history count: 1 (init) + 2 updates = 3 total calls to update API
    TEST_ASSERT_EQUAL_UINT32(3u, IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());
}

// 5) update: boundary behavior — wrap-around and 100% clamp
void test_updateGtmTomPwmDutyCycles_BoundaryWrapAround(void)
{
    const float32 tol = 1.0f; // percentage tolerance

    initGtmTomPwm();

    // After 3 updates: W = 75 + 3*10 = 105 -> wraps to 5
    for (int i = 0; i < 3; ++i) { updateGtmTomPwmDutyCycles(); }
    float32 dutyW_after3 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);
    float32 expW_after3 = INITIAL_DUTY_PERCENT_W + 3.0f * UPDATE_POLICY_INCREMENT_PERCENT; // 105
    while (expW_after3 >= 100.0f) { expW_after3 -= 100.0f; } // wrap into 0–100 → 5
    TEST_ASSERT_FLOAT_WITHIN(tol, expW_after3, dutyW_after3);

    // After total 5 updates: V = 50 + 5*10 = 100 → must be clamped just below 100
    for (int i = 0; i < 2; ++i) { updateGtmTomPwmDutyCycles(); }
    float32 dutyV_after5 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    TEST_ASSERT_TRUE(dutyV_after5 < 100.0f); // clamp 100% to period-1 tick

    // After total 8 updates: U = 25 + 8*10 = 105 → wraps to 5
    for (int i = 0; i < 3; ++i) { updateGtmTomPwmDutyCycles(); }
    float32 dutyU_after8 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 expU_after8 = INITIAL_DUTY_PERCENT_U + 8.0f * UPDATE_POLICY_INCREMENT_PERCENT; // 105
    while (expU_after8 >= 100.0f) { expU_after8 -= 100.0f; } // wrap into 0–100 → 5
    TEST_ASSERT_FLOAT_WITHIN(tol, expU_after8, dutyU_after8);
}

// 6) update: ensures no re-initialization APIs are called during runtime updates
void test_updateGtmTomPwmDutyCycles_DoesNotReInit(void)
{
    initGtmTomPwm();

    // Record init-phase call counts
    uint32 cnt_enable_before          = IfxGtm_Mock_GetCallCount_enable();
    uint32 cnt_cmu_enable_before      = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 cnt_pinmap_before          = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    uint32 cnt_gpio_mode_before       = IfxPort_Mock_GetCallCount_setPinModeOutput();
    uint32 cnt_gpio_state_before      = IfxPort_Mock_GetCallCount_setPinState();
    uint32 cnt_pwm_initcfg_before     = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 cnt_pwm_initch_before      = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 cnt_pwm_init_before        = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 cnt_pwm_start_before       = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 cnt_update_before          = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    // Act: runtime updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert: no re-initialization occurred
    TEST_ASSERT_EQUAL_UINT32(cnt_enable_before,      IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cnt_cmu_enable_before,  IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(cnt_pinmap_before,      IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL_UINT32(cnt_gpio_mode_before,   IfxPort_Mock_GetCallCount_setPinModeOutput());
    TEST_ASSERT_EQUAL_UINT32(cnt_gpio_state_before,  IfxPort_Mock_GetCallCount_setPinState());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initcfg_before, IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initch_before,  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_init_before,    IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_start_before,   IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());

    // Only the duty update API should have increased by exactly 3 calls
    uint32 cnt_update_after = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_EQUAL_UINT32(cnt_update_before + 3u, cnt_update_after);
}
