#include "unity.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

// Structured configuration values (provided by requirements)
#define EGTM_CLUSTER                    (0U)
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)
#define TIMING_DEADTIME_US              (1U)
#define CLOCK_GTM_CMU_CLK_MHZ           (100U)

static inline uint32 compute_deadtime_ticks(uint32 deadtime_us, uint32 clk_mhz)
{
    return (deadtime_us * clk_mhz); // 1 us at 100 MHz => 100 ticks
}

void setUp(void)
{
    // Reset all mocks
    IfxPort_Mock_Reset();
    IfxEgtm_Pwm_Mock_Reset();
    IfxEgtm_Mock_Reset();
    IfxEgtm_Cmu_Mock_Reset();
    IfxEgtm_Dtm_Mock_Reset();
    IfxEgtm_PinMap_Mock_Reset();
    IfxEgtm_Trigger_Mock_Reset();
    IfxAdc_Tmadc_Mock_Reset();
}

void tearDown(void) {}

// 1) INIT: Verify expected TC4xx driver APIs are called (call counts > 0)
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Act
    initEgtmAtom3phInv();

    // Assert - TC4xx API usage and essential calls
    TEST_ASSERT_TRUE(IfxEgtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_init() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setClockSource() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setRelrise() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setRelfall() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc() > 0);

    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModule() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_runModule() > 0);

    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startSyncedGroups() > 0);
}

// 2) INIT: Verify configuration values passed to drivers (frequency, deadtime, trigger routing cluster)
void test_initEgtmAtom3phInv_SetsExpectedConfigValues(void)
{
    // Act
    initEgtmAtom3phInv();

    // PWM base frequency on initConfig/init
    TEST_ASSERT_FLOAT_WITHIN(1.0f, TIMING_PWM_FREQUENCY_HZ, IfxEgtm_Pwm_Mock_GetLastArg_initConfig_frequency());
    TEST_ASSERT_FLOAT_WITHIN(1.0f, TIMING_PWM_FREQUENCY_HZ, IfxEgtm_Pwm_Mock_GetLastArg_init_frequency());

    // DTM deadtime ticks computed from 1 us at 100 MHz => 100 ticks
    const uint32 expectedDeadtimeTicks = compute_deadtime_ticks(TIMING_DEADTIME_US, CLOCK_GTM_CMU_CLK_MHZ);
    TEST_ASSERT_EQUAL_UINT32(expectedDeadtimeTicks, IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise());
    TEST_ASSERT_EQUAL_UINT32(expectedDeadtimeTicks, IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall());

    // Trigger routing to ADC: validate eGTM cluster index
    TEST_ASSERT_EQUAL(EGTM_CLUSTER, IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_egtmCluster());

    // CMU clocks: ensure some clocks were enabled (mask non-zero)
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask() != 0U);
}

// 3) UPDATE: Single call applies provided duties immediately and toggles heartbeat
void test_updateEgtmAtom3phInvDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    initEgtmAtom3phInv();
    float32 requestDuty[3] = {0.25f, 0.50f, 0.75f};

    // Act
    updateEgtmAtom3phInvDuty(&requestDuty[0]);

    // Assert - correct driver API called
    TEST_ASSERT_EQUAL_UINT32(1U, IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate());
    TEST_ASSERT_EQUAL_UINT32(1U, IfxPort_Mock_GetCallCount_togglePin());

    // Value verification - duties forwarded to driver
    float32 ch0 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 ch1 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 ch2 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, requestDuty[0], ch0);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, requestDuty[1], ch1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, requestDuty[2], ch2);
}

// 4) UPDATE: Multiple calls use each provided array immediately and in order
void test_updateEgtmAtom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    initEgtmAtom3phInv();
    float32 req1[3] = {0.10f, 0.20f, 0.30f};
    float32 req2[3] = {0.60f, 0.70f, 0.80f};

    // Act
    updateEgtmAtom3phInvDuty(&req1[0]); // 1st call
    updateEgtmAtom3phInvDuty(&req2[0]); // 2nd call

    // Assert - heartbeat toggled on each call
    TEST_ASSERT_EQUAL_UINT32(2U, IfxPort_Mock_GetCallCount_togglePin());

    // Argument history verification
    TEST_ASSERT_EQUAL_UINT32(2U, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());

    float32 c0_1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0U, 0U);
    float32 c1_1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0U, 1U);
    float32 c2_1 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0U, 2U);

    float32 c0_2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1U, 0U);
    float32 c1_2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1U, 1U);
    float32 c2_2 = IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1U, 2U);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, req1[0], c0_1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, req1[1], c1_1);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, req1[2], c2_1);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, req2[0], c0_2);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, req2[1], c1_2);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, req2[2], c2_2);
}

// 5) UPDATE: Boundary/clamp behavior to [0.0, 1.0]
void test_updateEgtmAtom3phInvDuty_BoundaryWrapAround(void)
{
    // Arrange
    initEgtmAtom3phInv();
    float32 req[3] = {-0.50f, 0.00f, 1.50f};

    // Act
    updateEgtmAtom3phInvDuty(&req[0]);

    // Assert - captured duties are clamped into [0.0, 1.0]
    float32 ch0 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 ch1 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 ch2 = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ch0); // clamped up to 0.0
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, ch1); // unchanged (already 0.0)
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 1.0f, ch2); // clamped down to 1.0
}

// 6) UPDATE: Ensure no re-initialization occurs during updates
void test_updateEgtmAtom3phInvDuty_DoesNotReInit(void)
{
    // Arrange
    initEgtmAtom3phInv();

    uint32 initCfg_before   = IfxEgtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 init_before      = IfxEgtm_Pwm_Mock_GetCallCount_init();
    uint32 chCfg_before     = IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 start_before     = IfxEgtm_Pwm_Mock_GetCallCount_startSyncedGroups();

    float32 reqA[3] = {0.20f, 0.40f, 0.60f};
    float32 reqB[3] = {0.30f, 0.50f, 0.70f};

    // Act
    updateEgtmAtom3phInvDuty(&reqA[0]);
    updateEgtmAtom3phInvDuty(&reqB[0]);

    // Assert - init path not called again
    TEST_ASSERT_EQUAL_UINT32(initCfg_before, IfxEgtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(init_before,    IfxEgtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(chCfg_before,   IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(start_before,   IfxEgtm_Pwm_Mock_GetCallCount_startSyncedGroups());

    // Update path called twice; heartbeat toggled twice
    TEST_ASSERT_EQUAL_UINT32(2U, IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate());
    TEST_ASSERT_EQUAL_UINT32(2U, IfxPort_Mock_GetCallCount_togglePin());
}
