#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

// Test constants derived from requirements/design
static const float32 TOL_PERCENT = 1.0f;           // tolerance for duty percentage comparisons
static const float32 EXPECTED_PWM_FREQ_HZ = 20000.0f;  // TIMING_PWM_FREQUENCY_HZ
static const uint32  EXPECTED_NUM_CHANNELS = 3U;       // 3 synchronized channels (TOM1 CH2/4/6)
static const float32 INITIAL_DUTY_U = 25.0f;           // U phase initial duty [%]
static const float32 INITIAL_DUTY_V = 50.0f;           // V phase initial duty [%]
static const float32 INITIAL_DUTY_W = 75.0f;           // W phase initial duty [%]
static const float32 DUTY_STEP_PERCENT = 10.0f;        // TIMING_DUTY_STEP_PERCENT

void setUp(void)
{
    // Reset all related driver mocks before each test
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) INIT: Verify expected driver APIs are invoked
void test_initGtmTom3PhaseInverterPwm_CallsExpectedDriverAPIs(void)
{
    initGtmTom3PhaseInverterPwm();

    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);                     // IfxGtm_enable
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);           // IfxGtm_Cmu_enableClocks
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);          // IfxGtm_PinMap_setTomTout (pin routing)

    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);                   // IfxGtm_Pwm_init
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);    // IfxGtm_Pwm_startSyncedChannels
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0); // initial duty push
}

// 2) INIT: Verify configuration values and initial duties applied
void test_initGtmTom3PhaseInverterPwm_SetsExpectedConfigValues(void)
{
    initGtmTom3PhaseInverterPwm();

    // Verify init parameters captured by the PWM mock
    float32 freq = IfxGtm_Pwm_Mock_GetLastArg_init_frequency();
    TEST_ASSERT_FLOAT_WITHIN(1.0f, EXPECTED_PWM_FREQ_HZ, freq);

    uint32 numCh = IfxGtm_Pwm_Mock_GetLastArg_init_numChannels();
    TEST_ASSERT_EQUAL_UINT32(EXPECTED_NUM_CHANNELS, numCh);

    // Verify initial duties were applied via the immediate multi-channel update
    float32 u0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_U, u0);
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_V, v0);
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_W, w0);
}

// 3) UPDATE (single call): +10% step applied immediately to all channels
void test_updateGtmTom3PhaseDuty_SingleCall_UpdatesValues(void)
{
    initGtmTom3PhaseInverterPwm();

    uint32 before = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    updateGtmTom3PhaseDuty();
    uint32 after = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_TRUE(after > before);  // update must push new duties immediately

    float32 u1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_U + DUTY_STEP_PERCENT, u1); // 25 -> 35
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_V + DUTY_STEP_PERCENT, v1); // 50 -> 60
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_W + DUTY_STEP_PERCENT, w1); // 75 -> 85
}

// 4) UPDATE (two calls): progression equals initial + 2*step
void test_updateGtmTom3PhaseDuty_MultipleCalls_ProgressesCorrectly(void)
{
    initGtmTom3PhaseInverterPwm();

    updateGtmTom3PhaseDuty(); // +10
    updateGtmTom3PhaseDuty(); // +10 (total +20)

    float32 u2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w2 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_U + 2.0f * DUTY_STEP_PERCENT, u2); // 25 -> 45
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_V + 2.0f * DUTY_STEP_PERCENT, v2); // 50 -> 70
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, INITIAL_DUTY_W + 2.0f * DUTY_STEP_PERCENT, w2); // 75 -> 95
}

// 5) UPDATE: boundary wrap-around at 100% -> 0%
void test_updateGtmTom3PhaseDuty_BoundaryWrapAround(void)
{
    initGtmTom3PhaseInverterPwm();

    // For W channel (starts at 75%), three updates hit/exceed 100%: 75 -> 85 -> 95 -> 105 -> wrap to 0
    updateGtmTom3PhaseDuty();
    updateGtmTom3PhaseDuty();
    updateGtmTom3PhaseDuty();

    float32 u = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0); // 25 + 30 = 55
    float32 v = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1); // 50 + 30 = 80
    float32 w = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2); // wrap to 0

    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, 55.0f, u);
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, 80.0f, v);
    TEST_ASSERT_FLOAT_WITHIN(TOL_PERCENT, 0.0f,  w);
}

// 6) UPDATE: ensure no re-initialization occurs during updates
void test_updateGtmTom3PhaseDuty_DoesNotReInit(void)
{
    initGtmTom3PhaseInverterPwm();

    uint32 init_calls_before   = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 gtm_en_before       = IfxGtm_Mock_GetCallCount_enable();
    uint32 cmu_clk_before      = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 start_before        = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 pinmap_before       = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();

    // Perform several updates
    for (int i = 0; i < 5; ++i) {
        updateGtmTom3PhaseDuty();
    }

    // None of the init-phase APIs should be called again
    TEST_ASSERT_EQUAL_UINT32(init_calls_before, IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(gtm_en_before,     IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cmu_clk_before,    IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(start_before,      IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());
    TEST_ASSERT_EQUAL_UINT32(pinmap_before,     IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
}
