#include "unity.h"
#include "Ifx_Types.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

// TC4xx target iLLD mock headers
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

void setUp(void)
{
    // Reset all driver mocks before each test
    IfxEgtm_Mock_Reset();
    IfxEgtm_Cmu_Mock_Reset();
    IfxEgtm_Dtm_Mock_Reset();
    IfxEgtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Verify init calls the expected TC4xx driver APIs (migration guard)
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Act
    initEgtmAtom3phInv();

    // Assert: TC4xx eGTM enable + CMU clocks + DTM clock source configured
    TEST_ASSERT_TRUE(IfxEgtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_enableClocks() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setClockSource() > 0);

    // Assert: Unified PWM driver config/init/start (sync-start enabled)
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);

    // Assert: LED GPIO configured and driven to known state
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0);
}

// 2) Validate key configuration values observable via mocks after init
//    - CMU clock mask (non-zero: FXCLK_0 + CLK0 required)
//    - DTM clock source configured (non-zero)
//    - No duty update issued during init (syncStart only; syncUpdate occurs on runtime)
void test_initEgtmAtom3phInv_SetsExpectedConfigValues(void)
{
    // Act
    initEgtmAtom3phInv();

    // Value checks via available capture helpers
    uint32 clkMask = IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask();
    TEST_ASSERT_TRUE(clkMask != 0u);

    uint32 dtmClkSrc = IfxEgtm_Dtm_Mock_GetLastArg_setClockSource();
    TEST_ASSERT_TRUE(dtmClkSrc != 0u);

    // Sync-start must be performed at init as per PWM_SYNCSTART = True
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);

    // No duty array pushed during init (syncUpdate occurs on runtime updates)
    TEST_ASSERT_EQUAL_UINT32(0u, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty());
}

// 3) Single-call runtime update: duties are applied immediately and match driver state
void test_updateEgtmAtom3phInvDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    initEgtmAtom3phInv();

    // Act
    updateEgtmAtom3phInvDuty();

    // Assert: TC4xx unified driver immediate update API called
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);

    // Assert: Values passed to driver match production's duty array (coherent multi-channel update)
    float32 chU = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(0);
    float32 chV = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(1);
    float32 chW = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(2);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, g_egtmAtom3phInv.dutyCycles[0], chU);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, g_egtmAtom3phInv.dutyCycles[1], chV);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, g_egtmAtom3phInv.dutyCycles[2], chW);
}

// 4) Multiple updates: verify equal step progression across all three channels (independent updates, syncUpdate)
void test_updateEgtmAtom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    initEgtmAtom3phInv();

    // Act: two updates → two immediate multi-channel updates captured by the mock
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    // Assert: at least two captured arrays
    uint32 histCount = IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty();
    TEST_ASSERT_TRUE(histCount >= 2u);

    // Compare per-channel deltas between consecutive updates
    float32 u1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(0u, 0u);
    float32 u2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(1u, 0u);
    float32 v1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(0u, 1u);
    float32 v2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(1u, 1u);
    float32 w1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(0u, 2u);
    float32 w2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(1u, 2u);

    float32 du = u2 - u1;
    float32 dv = v2 - v1;
    float32 dw = w2 - w1;

    // All channels advance by the same step, positive direction
    TEST_ASSERT_TRUE(du > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, du, dv);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, du, dw);
}

// 5) Boundary/Wrap-around: values ramp and then wrap to low range (near zero) coherently
void test_updateEgtmAtom3phInvDuty_BoundaryWrapAround(void)
{
    // Arrange
    initEgtmAtom3phInv();

    // Establish a positive step from first two calls
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    uint32 hc = IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty();
    TEST_ASSERT_TRUE(hc >= 2u);

    float32 prevPrevU = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(hc - 2u, 0u);
    float32 prevU     = IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(hc - 1u, 0u);
    float32 step      = prevU - prevPrevU;
    TEST_ASSERT_TRUE(step > 0.0f);

    bool wrappedU = false, wrappedV = false, wrappedW = false;

    // Iterate sufficiently to observe wrap-around (100% boundary not needed as a constant)
    for (uint32 i = 0u; i < 300u && !(wrappedU && wrappedV && wrappedW); ++i)
    {
        float32 lastU = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(0u);
        float32 lastV = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(1u);
        float32 lastW = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(2u);

        updateEgtmAtom3phInvDuty();

        float32 nowU = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(0u);
        float32 nowV = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(1u);
        float32 nowW = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(2u);

        if (!wrappedU && (nowU < lastU)) { wrappedU = true; TEST_ASSERT_TRUE(nowU <= (step + 1.0f)); }
        if (!wrappedV && (nowV < lastV)) { wrappedV = true; TEST_ASSERT_TRUE(nowV <= (step + 1.0f)); }
        if (!wrappedW && (nowW < lastW)) { wrappedW = true; TEST_ASSERT_TRUE(nowW <= (step + 1.0f)); }
    }

    TEST_ASSERT_TRUE(wrappedU);
    TEST_ASSERT_TRUE(wrappedV);
    TEST_ASSERT_TRUE(wrappedW);
}

// 6) No re-initialization during runtime updates; only the update API is invoked repeatedly
void test_updateEgtmAtom3phInvDuty_DoesNotReInit(void)
{
    // Arrange
    initEgtmAtom3phInv();

    uint32 initInitCount   = IfxEgtm_Pwm_Mock_GetCallCount_init();
    uint32 initEnableCount = IfxEgtm_Mock_GetCallCount_enable();
    uint32 initClkCount    = IfxEgtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 initDtmCount    = IfxEgtm_Dtm_Mock_GetCallCount_setClockSource();

    // Act: perform multiple runtime updates
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    // Assert: no re-init or re-enable paths taken
    TEST_ASSERT_EQUAL_UINT32(initInitCount,   IfxEgtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(initEnableCount, IfxEgtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(initClkCount,    IfxEgtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(initDtmCount,    IfxEgtm_Dtm_Mock_GetCallCount_setClockSource());

    // Assert: runtime update API invoked on each call
    TEST_ASSERT_EQUAL_UINT32(3u, IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate());
    TEST_ASSERT_EQUAL_UINT32(3u, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty());
}
