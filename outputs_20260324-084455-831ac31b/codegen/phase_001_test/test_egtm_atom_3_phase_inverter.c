#include "unity.h"
#include "egtm_atom_3_phase_inverter.h"

#include "Ifx_Types.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

// Structured configuration values (from requirements)
#define PWM_OUTPUTS_DEADTIME_US             (1.0f)
#define TIMING_DTM_CLOCK_ASSUMPTION_MHZ     (100.0f)
#define PWM_OUTPUTS_INITIAL_DUTY_U          (25.0f)
#define PWM_OUTPUTS_INITIAL_DUTY_V          (50.0f)
#define PWM_OUTPUTS_INITIAL_DUTY_W          (75.0f)
#define ADC_TRIGGER_CHANNEL_CH3              (3u)
#define LED_PIN_INDEX_P03_9                 (9u)

static void resetAllMocks(void)
{
    IfxEgtm_Pwm_Mock_Reset();
    IfxEgtm_Dtm_Mock_Reset();
    IfxEgtm_PinMap_Mock_Reset();
    IfxEgtm_Mock_Reset();
    IfxEgtm_Cmu_Mock_Reset();
    IfxEgtm_Trigger_Mock_Reset();
    IfxAdc_Tmadc_Mock_Reset();
    IfxPort_Mock_Reset();
}

void setUp(void)
{
    resetAllMocks();
}

void tearDown(void) {}

// 1) Verify init calls all expected TC4xx driver APIs (call counts > 0)
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Arrange
    // Ensure trigger API returns success path for init routing
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);

    // Act
    initEgtmAtom3phInv();

    // Assert — TC4xx API usage only
    TEST_ASSERT_TRUE(IfxEgtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Cmu_Mock_GetCallCount_enableClocks() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_init() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setClockSource() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setRelrise() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setRelfall() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Function() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Select() > 0);
    TEST_ASSERT_TRUE(IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Polarity() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);

    TEST_ASSERT_TRUE(IfxEgtm_Trigger_Mock_GetCallCount_trigToAdc() > 0);

    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModule() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannel() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_runModule() > 0);

    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0);
}

// 2) Verify init programs correct configuration values (deadtime ticks, LED pin, ADC trigger channel)
void test_initEgtmAtom3phInv_SetsExpectedConfigValues(void)
{
    // Arrange
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);

    // Act
    initEgtmAtom3phInv();

    // Assert: Dead-time programming (1.0 us at 100 MHz => 100 ticks)
    const uint32 expectedDeadtimeTicks = (uint32)(PWM_OUTPUTS_DEADTIME_US * TIMING_DTM_CLOCK_ASSUMPTION_MHZ);
    uint32 lastRiseTicks = IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise();
    uint32 lastFallTicks = IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall();
    TEST_ASSERT_EQUAL_UINT32(expectedDeadtimeTicks, lastRiseTicks);
    TEST_ASSERT_EQUAL_UINT32(expectedDeadtimeTicks, lastFallTicks);

    // Assert: LED pin configured on P03.9 (pin index 9)
    TEST_ASSERT_EQUAL_UINT32(LED_PIN_INDEX_P03_9, IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex());
    TEST_ASSERT_EQUAL_UINT32(LED_PIN_INDEX_P03_9, IfxPort_Mock_GetLastArg_setPinState_pinIndex());

    // Assert: ADC trigger is sourced from ATOM0 C0 CH3
    TEST_ASSERT_EQUAL_UINT32(ADC_TRIGGER_CHANNEL_CH3, IfxEgtm_Trigger_Mock_GetLastArg_trigToAdc_Channel());
}

// 3) Single update call applies requested duties immediately
void test_updateEgtmAtom3phInvDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);
    initEgtmAtom3phInv();

    float32 requestDuty[3] = { PWM_OUTPUTS_INITIAL_DUTY_U, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_W };

    // Act
    updateEgtmAtom3phInvDuty(requestDuty);

    // Assert: Driver update was called and values match exactly
    TEST_ASSERT_TRUE(IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);

    float32 u = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w = IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_U, u);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_V, v);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_W, w);
}

// 4) Multiple update calls: verify per-call values via argument history (no unintended progression)
void test_updateEgtmAtom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);
    initEgtmAtom3phInv();

    float32 dutyFirst[3]  = { PWM_OUTPUTS_INITIAL_DUTY_U, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_W };
    float32 dutySecond[3] = { PWM_OUTPUTS_INITIAL_DUTY_W, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_U };

    // Act
    updateEgtmAtom3phInvDuty(dutyFirst);
    updateEgtmAtom3phInvDuty(dutySecond);

    // Assert: Two update calls captured
    TEST_ASSERT_EQUAL_UINT32(2u, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());

    // Call 0 values
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_U, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 0u));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_V, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 1u));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_W, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 2u));

    // Call 1 values
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_W, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 0u));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_V, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 1u));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_U, IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 2u));
}

// 5) Boundary/edge behavior: duties are applied as provided (no wrap/clamp performed by production update)
void test_updateEgtmAtom3phInvDuty_BoundaryValues_PassThroughWithoutModification(void)
{
    // Arrange
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);
    initEgtmAtom3phInv();

    // Using configured values directly to validate pass-through
    float32 requestDuty[3] = { PWM_OUTPUTS_INITIAL_DUTY_W, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_U };

    // Act
    updateEgtmAtom3phInvDuty(requestDuty);

    // Assert: The exact requested values are forwarded to the driver
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_W, IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_V, IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PWM_OUTPUTS_INITIAL_DUTY_U, IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2));
}

// 6) Update does not re-initialize PWM/eGTM/ADC (no re-init calls during runtime updates)
void test_updateEgtmAtom3phInvDuty_DoesNotReInit(void)
{
    // Arrange
    IfxEgtm_Trigger_Mock_SetReturn_trigToAdc(0u);
    initEgtmAtom3phInv();

    // Snapshot init-phase call counts
    uint32 cnt_enable_before               = IfxEgtm_Mock_GetCallCount_enable();
    uint32 cnt_cmu_enable_before           = IfxEgtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 cnt_pwm_initcfg_before          = IfxEgtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 cnt_pwm_initchcfg_before        = IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 cnt_pwm_init_before             = IfxEgtm_Pwm_Mock_GetCallCount_init();
    uint32 cnt_pwm_startSynced_before      = IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 cnt_adc_modcfg_before           = IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig();
    uint32 cnt_adc_mod_before              = IfxAdc_Tmadc_Mock_GetCallCount_initModule();
    uint32 cnt_adc_chcfg_before            = IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig();
    uint32 cnt_adc_ch_before               = IfxAdc_Tmadc_Mock_GetCallCount_initChannel();

    float32 dutyA[3] = { PWM_OUTPUTS_INITIAL_DUTY_U, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_W };
    float32 dutyB[3] = { PWM_OUTPUTS_INITIAL_DUTY_W, PWM_OUTPUTS_INITIAL_DUTY_V, PWM_OUTPUTS_INITIAL_DUTY_U };

    // Act — multiple runtime updates
    updateEgtmAtom3phInvDuty(dutyA);
    updateEgtmAtom3phInvDuty(dutyB);

    // Assert — no additional init-phase calls were made
    TEST_ASSERT_EQUAL_UINT32(cnt_enable_before,          IfxEgtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cnt_cmu_enable_before,      IfxEgtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initcfg_before,     IfxEgtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_initchcfg_before,   IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_init_before,        IfxEgtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(cnt_pwm_startSynced_before, IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels());
    TEST_ASSERT_EQUAL_UINT32(cnt_adc_modcfg_before,      IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_adc_mod_before,         IfxAdc_Tmadc_Mock_GetCallCount_initModule());
    TEST_ASSERT_EQUAL_UINT32(cnt_adc_chcfg_before,       IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(cnt_adc_ch_before,          IfxAdc_Tmadc_Mock_GetCallCount_initChannel());

    // And the runtime update API was called twice
    TEST_ASSERT_EQUAL_UINT32(2u, IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate());
}
