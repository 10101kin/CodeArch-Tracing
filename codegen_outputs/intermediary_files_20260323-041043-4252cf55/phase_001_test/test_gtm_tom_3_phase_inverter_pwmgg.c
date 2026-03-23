#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwmgg.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "Ifx_Types.h"

// Structured configuration values derived from requirements
static const float32 TIMING_PWM_FREQUENCY_HZ_VAL = 20000.0f;     // TIMING_PWM_FREQUENCY_HZ
static const boolean TIMING_SYNCHRONOUS_UPDATE_VAL = TRUE;       // TIMING_SYNCHRONOUS_UPDATE
static const uint32  TIMING_DUTY_UPDATE_PERIOD_MS_VAL = 500;     // TIMING_DUTY_UPDATE_PERIOD_MS
static const float32 TIMING_DUTY_STEP_PERCENT_VAL = 10.0f;       // TIMING_DUTY_STEP_PERCENT
static const float32 TIMING_DEADTIME_NS_VAL = 0.0f;              // TIMING_DEADTIME_NS
static const float32 TIMING_MIN_PULSE_NS_VAL = 0.0f;             // TIMING_MIN_PULSE_NS
static const float32 DUTY_MIN_PERCENT_VAL = 0.0f;                // wrap low threshold (0%)
static const float32 DUTY_MAX_PERCENT_VAL = 100.0f;              // wrap high threshold (100%)

// Initial duties from user requirements
static const float32 DUTY_U_INIT_PERCENT = 25.0f;
static const float32 DUTY_V_INIT_PERCENT = 50.0f;
static const float32 DUTY_W_INIT_PERCENT = 75.0f;

// Comparison tolerance for percentage checks
static const float32 DUTY_TOL = 1.0f;

void setUp(void)
{
    // Reset all mocks (void functions, no arguments)
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Init/config behavior and driver API calls
void test_updateGtmTomPwmDutyCycles_CallsExpectedDriverAPIs(void)
{
    // Act: first call performs initialization/config + applies initial duties
    updateGtmTomPwmDutyCycles();

    // Assert: verify all expected iLLD calls happened at least once
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// 2) Init/config value verification (initial duties, synchronous start)
void test_updateGtmTomPwmDutyCycles_SetsExpectedConfigValues(void)
{
    // Act: first call performs initialization/config and applies initial duties
    updateGtmTomPwmDutyCycles();

    // Assert initial duty array values passed to the driver
    float32 u0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(0);
    float32 v0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(1);
    float32 w0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, DUTY_U_INIT_PERCENT, u0);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, DUTY_V_INIT_PERCENT, v0);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, DUTY_W_INIT_PERCENT, w0);

    // Synchronous update enabled → startSyncedChannels must be called during init
    if (TIMING_SYNCHRONOUS_UPDATE_VAL)
    {
        TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
    }
}

// 3) Single update step (+10%) after init
void test_updateGtmTomPwmDutyCycles_SingleCall_UpdatesValues(void)
{
    // Arrange: first call initializes and applies initial duties
    updateGtmTomPwmDutyCycles();

    // Act: second call performs one runtime update (+10%) with wrap behavior
    updateGtmTomPwmDutyCycles();

    // Assert: last update array equals initial + step
    const float32 expU = DUTY_U_INIT_PERCENT + TIMING_DUTY_STEP_PERCENT_VAL;  // 35%
    const float32 expV = DUTY_V_INIT_PERCENT + TIMING_DUTY_STEP_PERCENT_VAL;  // 60%
    const float32 expW = DUTY_W_INIT_PERCENT + TIMING_DUTY_STEP_PERCENT_VAL;  // 85%

    float32 u = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(0);
    float32 v = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(1);
    float32 w = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expU, u);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expV, v);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expW, w);

    // And the update API must have been called at least twice total (init + one update)
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty() >= 2);
}

// 4) Multiple updates progress correctly (2 steps: +20%)
void test_updateGtmTomPwmDutyCycles_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange: init
    updateGtmTomPwmDutyCycles();

    // Act: apply two update steps
    updateGtmTomPwmDutyCycles();  // step 1
    updateGtmTomPwmDutyCycles();  // step 2

    // Expected after two steps: initial + 2*step
    const float32 expU = DUTY_U_INIT_PERCENT + 2.0f * TIMING_DUTY_STEP_PERCENT_VAL; // 45%
    const float32 expV = DUTY_V_INIT_PERCENT + 2.0f * TIMING_DUTY_STEP_PERCENT_VAL; // 70%
    const float32 expW = DUTY_W_INIT_PERCENT + 2.0f * TIMING_DUTY_STEP_PERCENT_VAL; // 95%

    float32 u2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(0);
    float32 v2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(1);
    float32 w2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expU, u2);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expV, v2);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expW, w2);
}

// 5) Boundary/wrap-around: when >= 100% wrap to 0%
void test_updateGtmTomPwmDutyCycles_BoundaryWrapAround(void)
{
    // Arrange: init
    updateGtmTomPwmDutyCycles();

    // For W channel starting at 75%, with +10% step: need 3 updates to reach >= 100%
    const uint32 updatesToWrapW = 3U; // ceil((100-75)/10) = 3

    for (uint32 i = 0; i < updatesToWrapW; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    // Expected after 3 updates: U=55, V=80, W wraps to 0
    const float32 expU = DUTY_U_INIT_PERCENT + 3.0f * TIMING_DUTY_STEP_PERCENT_VAL; // 55
    const float32 expV = DUTY_V_INIT_PERCENT + 3.0f * TIMING_DUTY_STEP_PERCENT_VAL; // 80
    const float32 expW = DUTY_MIN_PERCENT_VAL;                                       // 0 after wrap

    float32 u = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(0);
    float32 v = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(1);
    float32 w = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expU, u);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expV, v);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOL, expW, w);

    // Ensure all duties remain within [0, 100]
    TEST_ASSERT_TRUE(u >= DUTY_MIN_PERCENT_VAL && u <= DUTY_MAX_PERCENT_VAL);
    TEST_ASSERT_TRUE(v >= DUTY_MIN_PERCENT_VAL && v <= DUTY_MAX_PERCENT_VAL);
    TEST_ASSERT_TRUE(w >= DUTY_MIN_PERCENT_VAL && w <= DUTY_MAX_PERCENT_VAL);
}

// 6) Runtime updates must not re-initialize
void test_updateGtmTomPwmDutyCycles_DoesNotReInit(void)
{
    // Arrange: perform the init/config pass
    updateGtmTomPwmDutyCycles();

    // Capture init-phase call counts right after init
    const uint32 enableCnt_init           = IfxGtm_Mock_GetCallCount_enable();
    const uint32 cmuEnableClkCnt_init     = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    const uint32 pwmInitCfgCnt_init       = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    const uint32 pwmInitChCfgCnt_init     = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    const uint32 pinMapSetCnt_init        = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    const uint32 pwmInitCnt_init          = IfxGtm_Pwm_Mock_GetCallCount_init();
    const uint32 pwmStartSyncedCnt_init   = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    const uint32 pwmUpdateCnt_before      = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty();

    // Act: perform several runtime updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert: init-phase calls must not increase after updates
    TEST_ASSERT_EQUAL(enableCnt_init,         IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL(cmuEnableClkCnt_init,   IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL(pwmInitCfgCnt_init,     IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL(pwmInitChCfgCnt_init,   IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL(pinMapSetCnt_init,      IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL(pwmInitCnt_init,        IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL(pwmStartSyncedCnt_init, IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());

    // But updateChannelsDuty must continue to be called
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty() > pwmUpdateCnt_before);
}
