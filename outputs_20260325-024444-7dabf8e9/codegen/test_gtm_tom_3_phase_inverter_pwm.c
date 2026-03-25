#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

// Structured configuration values (from requirements)
#define INITIAL_DUTY_PERCENT_U                 (25.0f)
#define INITIAL_DUTY_PERCENT_V                 (50.0f)
#define INITIAL_DUTY_PERCENT_W                 (75.0f)
#define DUTY_STEP_BEHAVIOR_INCREMENT_PERCENT   (10.0f)
// Derived from: ATOMIC_UPDATE_SCOPE = "All six output channels within same TGC"
#define NUM_CHANNELS_IN_TGC                    (6u)

#define WITHIN_TOL(x, y, tol)                  (((x) > (y)) ? (((x) - (y)) <= (tol)) : (((y) - (x)) <= (tol)))
#define ABSF(x)                                (((x) < 0.0f) ? (-(x)) : (x))

void setUp(void)
{
    // Reset all mocks before each test (VOID only; no arguments allowed)
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();

    // Ensure timer init succeeds (link-time mock return configuration)
    IfxGtm_Tom_Timer_Mock_SetReturn_init(TRUE);
}

void tearDown(void) {}

// 1) Verify init calls expected TC3xx iLLD APIs (target API usage + module references)
void test_initGtmTom3phInv_CallsExpectedDriverAPIs(void)
{
    initGtmTom3phInv();

    // GTM enable + CMU clocking
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_selectClkInput() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    // TOM timebase config and init
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);

    // Pin routing via generic pin-map
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);

    // Unified PWM init over TOM (migrated to IfxGtm_Pwm)
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);

    // Polarity and hardware dead-time configuration
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate() > 0);

    // Atomic TGC-gated initial duty programming
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);

    // Start synchronized channels
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// 2) Verify init sets expected config values (num channels + initial duties)
void test_initGtmTom3phInv_SetsExpectedConfigValues(void)
{
    initGtmTom3phInv();

    // A) PWM unified driver configured for 6 synchronized channels on same TOM/TGC
    TEST_ASSERT_EQUAL_UINT32(NUM_CHANNELS_IN_TGC, IfxGtm_Pwm_Mock_GetLastArg_init_numChannels());

    // B) Initial coherent duty programming: U=25%, V=50%, W=75% and their complements.
    // We verify the most recently programmed duty (last call) is one of {25, 50, 75}.
    // These are the only distinct values across high/low complementary pairs.
    float32 lastDuty = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();
    int matched = (WITHIN_TOL(lastDuty, INITIAL_DUTY_PERCENT_U, 1.0f) ||
                   WITHIN_TOL(lastDuty, INITIAL_DUTY_PERCENT_V, 1.0f) ||
                   WITHIN_TOL(lastDuty, INITIAL_DUTY_PERCENT_W, 1.0f));
    TEST_ASSERT_TRUE_MESSAGE(matched, "Initial duty (last programmed) must be one of {25, 50, 75} percent");

    // C) Atomic gating mechanism used at init (disable → write → apply)
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > 0);
}

// 3) Single update step applies +10% (high-side) / -10% (low-side) with atomic gating
void test_updateGtmTom3phInvDuty_SingleCall_UpdatesValues(void)
{
    initGtmTom3phInv();

    // Capture baseline (from init's last programmed channel)
    float32 d0 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();
    uint32 idx0 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_index();
    uint32 dis0 = IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate();
    uint32 app0 = IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate();
    uint32 upd0 = IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate();

    // One runtime update step
    updateGtmTom3phInvDuty();

    float32 d1 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();
    uint32 idx1 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_index();

    // The same API should be called again, gated atomically
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate() > dis0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate() > app0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate() > upd0);

    // Step magnitude must be exactly 10% (direction depends on high/low side)
    float32 stepMag = ABSF(d1 - d0);
    TEST_ASSERT_TRUE_MESSAGE(WITHIN_TOL(stepMag, DUTY_STEP_BEHAVIOR_INCREMENT_PERCENT, 1.0f), "Duty step must be 10% per call");

    // Index should remain the same channel across consecutive last-calls
    TEST_ASSERT_EQUAL_UINT32(idx0, idx1);
}

// 4) Multiple updates progress by N*step immediately
void test_updateGtmTom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    initGtmTom3phInv();

    float32 d0 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();
    uint32 upd0 = IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate();

    // Apply two update steps
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    float32 d2 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();

    // Magnitude should be 2 * step (wrap not expected from any initial {25,50,75} within two steps)
    float32 stepMag2 = ABSF(d2 - d0);
    TEST_ASSERT_TRUE(WITHIN_TOL(stepMag2, 2.0f * DUTY_STEP_BEHAVIOR_INCREMENT_PERCENT, 1.0f));

    // Ensure duty writes occurred at least twice after the baseline
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate() >= (upd0 + 2u));
}

// 5) Boundary/wrap-around: after enough updates, value wraps into 0..100% range correctly
void test_updateGtmTom3phInvDuty_BoundaryWrapAround(void)
{
    initGtmTom3phInv();

    float32 d0 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();

    // Determine side/direction by observing the first step
    updateGtmTom3phInvDuty();
    float32 d1 = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();
    int isIncreasing = (d1 > d0) ? 1 : 0;  // High-side increases, low-side decreases

    // Compute how many additional steps are needed to force the HIGH-SIDE to wrap
    // Let high0 be the corresponding high-side initial duty of the observed channel.
    int d0i = (int)(d0 + 0.5f);
    int step = (int)(DUTY_STEP_BEHAVIOR_INCREMENT_PERCENT + 0.5f); // 10
    int high0 = isIncreasing ? d0i : (100 - d0i);
    int remainingTo100 = 100 - high0;
    int nToReachOrExceed100 = (remainingTo100 + step - 1) / step; // ceil div
    int extra = 1; // ensure we cross the boundary
    int N = nToReachOrExceed100 + extra;

    // Apply remaining N-1 updates (we already did one above)
    for (int i = 0; i < N - 1; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    float32 dN = IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty();

    // Compute expected wrapped value for the observed channel
    int highN = (high0 + (N * step)) % 100;              // wrapped high-side
    int expectedDutyInt = isIncreasing ? highN : (100 - highN);
    float32 expectedDuty = (float32)expectedDutyInt;

    // Duty must remain within 0..100 and match the wrapped expectation
    TEST_ASSERT_TRUE_MESSAGE((dN >= 0.0f) && (dN <= 100.0f), "Duty must remain within 0..100% after wrap");
    TEST_ASSERT_TRUE_MESSAGE(WITHIN_TOL(dN, expectedDuty, 1.5f), "Wrapped duty must match expected modulo-100 progression");
}

// 6) Update must not re-initialize PWM/timebase or pin mapping
void test_updateGtmTom3phInvDuty_DoesNotReInit(void)
{
    initGtmTom3phInv();

    // Snapshot init-phase API call counts
    uint32 t_initCfg_0   = IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig();
    uint32 t_init_0      = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    uint32 p_initCfg_0   = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 p_initChCfg_0 = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 p_init_0      = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 p_polar_0     = IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity();
    uint32 p_dt_0        = IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate();
    uint32 map_0         = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();

    // Perform several runtime updates
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    // Verify no re-initialization or re-routing occurred during updates
    TEST_ASSERT_EQUAL_UINT32(t_initCfg_0,   IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(t_init_0,      IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(p_initCfg_0,   IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(p_initChCfg_0, IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(p_init_0,      IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(p_polar_0,     IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity());
    TEST_ASSERT_EQUAL_UINT32(p_dt_0,        IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate());
    TEST_ASSERT_EQUAL_UINT32(map_0,         IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
}
