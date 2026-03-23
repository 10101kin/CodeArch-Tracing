#include "unity.h"
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "Ifx_Types.h"

// Structured configuration values (from specification)
static const float32 kPwmFreqHz       = 20000.0f;   // TIMING_PWM_FREQUENCY_HZ
static const float32 kMinPulseUs      = 1.0f;       // TIMING_MIN_PULSE_US
static const float32 kDeadtimeUs      = 0.5f;       // TIMING_DEADTIME_US (call-count verified only)
static const uint32  kCenterAligned   = 1u;         // TIMING_CENTER_ALIGNED = True

// Helper computations derived strictly from structured values above
static float32 getPeriodUs(void) {
    return 1000000.0f / kPwmFreqHz; // 1e6 us per second / Hz => period in microseconds
}

static float32 getMinDuty(void) {
    // minDuty = min_pulse_ticks / period_ticks = (min_pulse_us / period_us)
    return kMinPulseUs / getPeriodUs();
}

static float32 getMaxDuty(void) {
    return 1.0f - getMinDuty();
}

static const float32 kDutyTol = 0.0005f;  // tolerance for float comparisons on fractions

void setUp(void) {
    // Reset all mocks before each test (void, no args per convention)
    IfxEgtm_Pwm_Mock_Reset();
    IfxEgtm_Cmu_Mock_Reset();
    IfxEgtm_PinMap_Mock_Reset();
    IfxEgtm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Init must call all expected TC4xx (eGTM) APIs — verify via call counts > 0
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void) {
    initEgtmAtom3phInv();

    // Core module and CMU
    TEST_ASSERT_TRUE(IfxEgtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_setGclkDivider() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    // Unified eGTM PWM config and init
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_init() > 0);

    // Pin mapping and polarity
    TEST_ASSERT_TRUE(IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity() > 0);

    // Deadtime, initial duty, and start
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs() > 0);
}

// 2) Init must set correct configuration values (frequency, alignment, sync start) and safe initial duties within [min,max]
void test_initEgtmAtom3phInv_SetsExpectedConfigValues(void) {
    initEgtmAtom3phInv();

    // Verify base PWM frequency passed to unified driver
    float32 freq = IfxEgtm_Pwm_Mock_GetLastArg_init_frequency();
    TEST_ASSERT_FLOAT_WITHIN(0.5f, kPwmFreqHz, freq);

    // Verify center-aligned mode and synchronized start enabled
    uint32 alignment = IfxEgtm_Pwm_Mock_GetLastArg_init_alignment();
    TEST_ASSERT_EQUAL_UINT32(kCenterAligned, alignment);

    uint32 syncStart = IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart();
    TEST_ASSERT_TRUE(syncStart != 0u);

    // Number of synchronized channels must be configured (should be > 0)
    uint32 numCh = IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels();
    TEST_ASSERT_TRUE(numCh > 0u);

    // Initial duty array applied during init must respect min/max pulse policy
    float32 minDuty = getMinDuty();
    float32 maxDuty = getMaxDuty();
    float32 d0 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(0);
    float32 d1 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(1);
    float32 d2 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(2);

    TEST_ASSERT_TRUE(d0 >= (minDuty - kDutyTol));
    TEST_ASSERT_TRUE(d0 <= (maxDuty + kDutyTol));
    TEST_ASSERT_TRUE(d1 >= (minDuty - kDutyTol));
    TEST_ASSERT_TRUE(d1 <= (maxDuty + kDutyTol));
    TEST_ASSERT_TRUE(d2 >= (minDuty - kDutyTol));
    TEST_ASSERT_TRUE(d2 <= (maxDuty + kDutyTol));
}

// 3) Single update: applies one ramp step and clamps within [minDuty, maxDuty]; also exercises getChannelState
void test_updateEgtmAtom3phInvDuty_SingleCall_UpdatesValues(void) {
    initEgtmAtom3phInv();

    // Ensure getChannelState can be called inside update path
    IfxEgtm_Pwm_Mock_SetReturn_getChannelState(1u);

    uint32 preCalls = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    updateEgtmAtom3phInvDuty();

    // The immediate update must have been issued once more
    uint32 postCalls = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_EQUAL_UINT32(preCalls + 1u, postCalls);

    // getChannelState should be consulted at runtime
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_getChannelState() > 0u);

    float32 minDuty = getMinDuty();
    float32 maxDuty = getMaxDuty();

    float32 u = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    // All channels are updated synchronously and within bounds
    TEST_ASSERT_TRUE(u >= (minDuty - kDutyTol) && u <= (maxDuty + kDutyTol));
    TEST_ASSERT_TRUE(v >= (minDuty - kDutyTol) && v <= (maxDuty + kDutyTol));
    TEST_ASSERT_TRUE(w >= (minDuty - kDutyTol) && w <= (maxDuty + kDutyTol));

    // The ramp applies the same increment to all channels — values should match closely
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, u, v);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, u, w);
}

// 4) Multiple updates: verify monotonic progression with consistent step across channels
void test_updateEgtmAtom3phInvDuty_MultipleCalls_ProgressesCorrectly(void) {
    initEgtmAtom3phInv();

    // Two successive updates
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    // Verify we captured two immediate update calls
    TEST_ASSERT_EQUAL_UINT32(2u, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());

    // Channel U values across the two calls
    float32 u1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 0u);
    float32 u2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 0u);
    float32 v1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 1u);
    float32 v2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 1u);
    float32 w1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 2u);
    float32 w2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 2u);

    float32 du = u2 - u1;
    float32 dv = v2 - v1;
    float32 dw = w2 - w1;

    // Step is positive (ramp up) and equal across channels
    TEST_ASSERT_TRUE(du > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, du, dv);
    TEST_ASSERT_FLOAT_WITHIN(kDutyTol, du, dw);
}

// 5) Boundary/wrap-around: after enough updates, duty wraps back near min or decreases versus previous
void test_updateEgtmAtom3phInvDuty_BoundaryWrapAround(void) {
    initEgtmAtom3phInv();

    float32 minDuty = getMinDuty();
    float32 maxDuty = getMaxDuty();

    float32 prev = -1.0f;
    uint32  maxIters = 1000u; // search bound to observe a wrap
    uint32  iter;
    uint8   wrapped = 0u;

    for (iter = 0u; iter < maxIters; ++iter) {
        updateEgtmAtom3phInvDuty();
        float32 curr = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0u);

        // Detect wrap: current drops below previous (beyond tolerance) or lands near minDuty
        if (prev >= 0.0f) {
            if (curr < (prev - kDutyTol)) { wrapped = 1u; break; }
        }
        if ((curr >= (minDuty - kDutyTol)) && (curr <= (minDuty + kDutyTol)) && (iter > 0u)) {
            wrapped = 1u;
            break;
        }
        prev = curr;
    }

    TEST_ASSERT_TRUE(wrapped != 0u);

    // Still must respect clamp range after wrap
    float32 lastU = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0u);
    TEST_ASSERT_TRUE(lastU >= (minDuty - kDutyTol));
    TEST_ASSERT_TRUE(lastU <= (maxDuty + kDutyTol));
}

// 6) Update must not re-run any init/config APIs
void test_updateEgtmAtom3phInvDuty_DoesNotReInit(void) {
    initEgtmAtom3phInv();

    // Snapshot init-related call counts
    uint32 c_initCfg        = IfxEgtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 c_initChCfg      = IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 c_init           = IfxEgtm_Pwm_Mock_GetCallCount_init();
    uint32 c_startSync      = IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 c_startOutputs   = IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs();
    uint32 c_setPol         = IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity();
    uint32 c_deadtime       = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime();
    uint32 c_updateDutyInit = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty();
    uint32 c_modEnable      = IfxEgtm_Mock_GetCallCount_enable();
    uint32 c_cmuSel         = IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput();
    uint32 c_cmuDiv         = IfxEgtm_Cmu_Mock_GetCallCount_setGclkDivider();
    uint32 c_cmuEn          = IfxEgtm_Cmu_Mock_GetCallCount_enableClocks();

    uint32 c_immediatePre   = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    // Perform several runtime updates
    const uint32 updates = 3u;
    for (uint32 i = 0u; i < updates; ++i) {
        updateEgtmAtom3phInvDuty();
    }

    // No re-init should occur
    TEST_ASSERT_EQUAL_UINT32(c_initCfg,        IfxEgtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(c_initChCfg,      IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(c_init,           IfxEgtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(c_startSync,      IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels());
    TEST_ASSERT_EQUAL_UINT32(c_startOutputs,   IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs());
    TEST_ASSERT_EQUAL_UINT32(c_setPol,         IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity());
    TEST_ASSERT_EQUAL_UINT32(c_deadtime,       IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime());
    TEST_ASSERT_EQUAL_UINT32(c_updateDutyInit, IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty());
    TEST_ASSERT_EQUAL_UINT32(c_modEnable,      IfxEgtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(c_cmuSel,         IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput());
    TEST_ASSERT_EQUAL_UINT32(c_cmuDiv,         IfxEgtm_Cmu_Mock_GetCallCount_setGclkDivider());
    TEST_ASSERT_EQUAL_UINT32(c_cmuEn,          IfxEgtm_Cmu_Mock_GetCallCount_enableClocks());

    // Immediate duty updates must increase exactly by the number of runtime calls
    uint32 c_immediatePost = IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_EQUAL_UINT32(c_immediatePre + updates, c_immediatePost);
}
