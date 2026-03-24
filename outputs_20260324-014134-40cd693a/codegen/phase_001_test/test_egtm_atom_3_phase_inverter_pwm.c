#include "unity.h"
#include "egtm_atom_3_phase_inverter_pwm.h"  // Production module header

// Mocked iLLD drivers
#include "IfxEgtm_Atom_Pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "Ifx_Types.h"

void setUp(void)
{
    // Reset all mocks
    IfxEgtm_Atom_Pwm_Mock_Reset();
    IfxEgtm_Mock_Reset();
    IfxEgtm_Cmu_Mock_Reset();
    IfxEgtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();
    IfxCpu_Irq_Mock_Reset();

    // Arrange: eGTM FXCLK functional clock present per design
    IfxEgtm_Cmu_Mock_SetReturn_isFxClockEnabled(TRUE);
}

void tearDown(void) {}

// 1) INIT: Verify all required TC4xx driver calls are made
void test_IfxEgtm_Atom_Pwm_init_CallsExpectedDriverAPIs(void)
{
    // Act
    IfxEgtm_Atom_Pwm_init();

    // Assert: TC4xx eGTM + ATOM PWM API surface used
    TEST_ASSERT_TRUE(IfxEgtm_Mock_GetCallCount_enable() > 0);                      // IfxEgtm_enable
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_enableClocks() > 0);            // IfxEgtm_Cmu_enableClocks
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_isFxClockEnabled() > 0);        // IfxEgtm_Cmu_isFxClockEnabled
    TEST_ASSERT_TRUE(IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout() > 0);          // IfxEgtm_PinMap_setAtomTout (pin map)

    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig() > 0);         // IfxEgtm_Atom_Pwm_initConfig
    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_initChannelConfig() > 0);  // IfxEgtm_Atom_Pwm_initChannelConfig
    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_init() > 0);               // IfxEgtm_Atom_Pwm_init
    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDeadTime() > 0); // Dead-time config
    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);    // Synchronous start

    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);            // LED as push-pull output
    TEST_ASSERT_TRUE(IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler() > 0);  // Period IRQ on CPU0
}

// 2) INIT: Verify configuration values (priority, pin modes, mapping count) from requirements
void test_IfxEgtm_Atom_Pwm_init_SetsExpectedConfigValues(void)
{
    // Act
    IfxEgtm_Atom_Pwm_init();

    // A) Period IRQ priority = 20 (CPU0 provider is configured within production; verify prio)
    TEST_ASSERT_EQUAL_UINT32(20u, IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber());

    // B) LED P13.0 configured as push-pull output (index = 0 for P13.0)
    TEST_ASSERT_EQUAL(IfxPort_OutputMode_pushPull, IfxPort_Mock_GetLastArg_setPinModeOutput_mode());
    TEST_ASSERT_EQUAL_UINT32(0u, IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex());

    // C) ATOM TOUT mapping configured for push-pull outputs (complementary pairs)
    //    3 phases (U,V,W) x 2 pins (HS/LS) => at least 6 TOUT mappings
    TEST_ASSERT_TRUE(IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout() >= 6u);
    TEST_ASSERT_EQUAL(IfxPort_OutputMode_pushPull, IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode());

    // D) Synchronous start required by design
    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
}

// Helper: clamp helper for expected comparisons (based on [0.0 .. 1.0] requirement)
static float32 clamp01(float32 v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// 3) UPDATE: Single call applies clamping [0.0 .. 1.0] and pushes values
void test_IfxEgtm_Atom_Pwm_setDutyCycle_SingleCall_UpdatesValues(void)
{
    // Init first (stateful guard)
    IfxEgtm_Atom_Pwm_init();

    // Arrange: three-phase requested duty values with out-of-range entries
    // Use only values from structured config range endpoints [0.0 .. 1.0]
    g_egtmAtom3phInv.dutyCycles[0] = -0.10f;  // will clamp to 0.0
    g_egtmAtom3phInv.dutyCycles[1] = 1.00f;   // in-range
    g_egtmAtom3phInv.dutyCycles[2] = 2.00f;   // will clamp to 1.0

    // Act
    IfxEgtm_Atom_Pwm_setDutyCycle();

    // Assert: update API called once
    TEST_ASSERT_EQUAL_UINT32(1u, IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty());

    // Assert: values are clamped to [0.0 .. 1.0]
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[2]);
}

// 4) UPDATE: Multiple calls accept new inputs and continue to clamp correctly (no progression/ramp)
void test_IfxEgtm_Atom_Pwm_setDutyCycle_MultipleCalls_ProgressesCorrectly(void)
{
    IfxEgtm_Atom_Pwm_init();

    // First update with safe endpoints
    g_egtmAtom3phInv.dutyCycles[0] = 0.0f;
    g_egtmAtom3phInv.dutyCycles[1] = 0.0f;
    g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    IfxEgtm_Atom_Pwm_setDutyCycle();

    TEST_ASSERT_EQUAL_UINT32(1u, IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty());
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[2]);

    // Second update with a new set that includes boundary maximums and minimums
    g_egtmAtom3phInv.dutyCycles[0] = 1.0f;  // max boundary
    g_egtmAtom3phInv.dutyCycles[1] = 0.0f;  // min boundary
    g_egtmAtom3phInv.dutyCycles[2] = 1.0f;  // max boundary
    IfxEgtm_Atom_Pwm_setDutyCycle();

    TEST_ASSERT_EQUAL_UINT32(2u, IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty());
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[2]);
}

// 5) UPDATE: Boundary/edge clamping verification at extremes
void test_IfxEgtm_Atom_Pwm_setDutyCycle_BoundaryWrapAround(void)
{
    IfxEgtm_Atom_Pwm_init();

    // Values beyond both ends must clamp to the valid range [0.0 .. 1.0]
    g_egtmAtom3phInv.dutyCycles[0] = -1.0f;  // below min
    g_egtmAtom3phInv.dutyCycles[1] = 2.0f;   // above max
    g_egtmAtom3phInv.dutyCycles[2] = 1.0f;   // exactly max (no change)

    IfxEgtm_Atom_Pwm_setDutyCycle();

    TEST_ASSERT_TRUE(IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty() > 0);

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, g_egtmAtom3phInv.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, g_egtmAtom3phInv.dutyCycles[2]);
}

// 6) UPDATE: Ensure runtime updates do not re-initialize PWM driver
void test_IfxEgtm_Atom_Pwm_setDutyCycle_DoesNotReInit(void)
{
    IfxEgtm_Atom_Pwm_init();

    // Capture init-related call counts immediately after init
    uint32 initCfgCalls_before = IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig();
    uint32 initCalls_before    = IfxEgtm_Atom_Pwm_Mock_GetCallCount_init();
    uint32 startSync_before    = IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels();

    // Perform multiple runtime updates with boundary-valid values
    g_egtmAtom3phInv.dutyCycles[0] = 0.0f; g_egtmAtom3phInv.dutyCycles[1] = 0.0f; g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    IfxEgtm_Atom_Pwm_setDutyCycle();
    g_egtmAtom3phInv.dutyCycles[0] = 1.0f; g_egtmAtom3phInv.dutyCycles[1] = 1.0f; g_egtmAtom3phInv.dutyCycles[2] = 1.0f;
    IfxEgtm_Atom_Pwm_setDutyCycle();
    g_egtmAtom3phInv.dutyCycles[0] = 0.0f; g_egtmAtom3phInv.dutyCycles[1] = 1.0f; g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    IfxEgtm_Atom_Pwm_setDutyCycle();

    // Assert: no re-init, no re-start of synced channels during updates
    TEST_ASSERT_EQUAL_UINT32(initCfgCalls_before, IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(initCalls_before,    IfxEgtm_Atom_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(startSync_before,    IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels());

    // Assert: exactly three update calls occurred
    TEST_ASSERT_EQUAL_UINT32(3u, IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty());
}
