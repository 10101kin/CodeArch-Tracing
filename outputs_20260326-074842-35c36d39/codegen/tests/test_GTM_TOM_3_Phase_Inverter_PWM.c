#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"           // Mocks
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "Ifx_Types.h"

// Structured configuration values from requirements
#ifndef INITIAL_DUTY_PERCENT_U
#define INITIAL_DUTY_PERCENT_U 25.0f
#endif
#ifndef INITIAL_DUTY_PERCENT_V
#define INITIAL_DUTY_PERCENT_V 50.0f
#endif
#ifndef INITIAL_DUTY_PERCENT_W
#define INITIAL_DUTY_PERCENT_W 75.0f
#endif
#ifndef TIMING_PWM_FREQUENCY_HZ
#define TIMING_PWM_FREQUENCY_HZ 20000
#endif
#ifndef TIMING_DEADTIME_US
#define TIMING_DEADTIME_US 0.5f
#endif
#ifndef TIMING_MIN_PULSE_US
#define TIMING_MIN_PULSE_US 1.0f
#endif
#ifndef MASTER_TIMEBASE_CENTER_ALIGNED
#define MASTER_TIMEBASE_CENTER_ALIGNED 1
#endif
#ifndef MASTER_TIMEBASE_CLOCK_SOURCE
// GCLK requested
#define MASTER_TIMEBASE_CLOCK_SOURCE 1
#endif
#ifndef SYNCHRONIZED_UPDATE_USE_DISABLE_APPLY_UPDATE
#define SYNCHRONIZED_UPDATE_USE_DISABLE_APPLY_UPDATE 1
#endif

// Tolerances
#define DUTY_PERCENT_TOL 1.0f
#define SMALL_TOL 0.001f

// Helper: choose which PWM update API was actually used by production code
typedef enum {
    UPDATE_API_NONE = 0,
    UPDATE_API_DUTY,
    UPDATE_API_PULSE_IMMEDIATE
} UpdateApi;

static UpdateApi pick_api_called_on_init(void)
{
    uint32 cntDuty = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty();
    uint32 cntPulse = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate();
    if (cntDuty > 0U) {
        return UPDATE_API_DUTY;
    } else if (cntPulse > 0U) {
        return UPDATE_API_PULSE_IMMEDIATE;
    }
    return UPDATE_API_NONE;
}

static void read_last_duties(UpdateApi api, float32 *u, float32 *v, float32 *w)
{
    switch (api) {
        case UPDATE_API_DUTY:
            *u = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_requestDuty(0);
            *v = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_requestDuty(1);
            *w = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_requestDuty(2);
            break;
        case UPDATE_API_PULSE_IMMEDIATE:
            *u = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestDuty(0);
            *v = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestDuty(1);
            *w = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestDuty(2);
            break;
        default:
            *u = *v = *w = 0.0f;
            break;
    }
}

static UpdateApi detect_update_api_used_after_call(uint32 beforeDutyCnt, uint32 beforePulseCnt)
{
    uint32 afterDutyCnt = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty();
    uint32 afterPulseCnt = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate();

    if (afterDutyCnt == beforeDutyCnt + 1U) {
        return UPDATE_API_DUTY;
    }
    if (afterPulseCnt == beforePulseCnt + 1U) {
        return UPDATE_API_PULSE_IMMEDIATE;
    }
    // If production batches multiple updates per call (unexpected), prefer the one that changed
    if (afterDutyCnt > beforeDutyCnt) {
        return UPDATE_API_DUTY;
    }
    if (afterPulseCnt > beforePulseCnt) {
        return UPDATE_API_PULSE_IMMEDIATE;
    }
    return UPDATE_API_NONE;
}

static float32 get_history_value(UpdateApi api, uint32 callIdx, uint32 elemIdx)
{
    switch (api) {
        case UPDATE_API_DUTY:
            return IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty_requestDuty(callIdx, elemIdx);
        case UPDATE_API_PULSE_IMMEDIATE:
            return IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_requestDuty(callIdx, elemIdx);
        default:
            return 0.0f;
    }
}

static uint32 get_history_count(UpdateApi api)
{
    switch (api) {
        case UPDATE_API_DUTY:
            return IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_requestDuty();
        case UPDATE_API_PULSE_IMMEDIATE:
            return IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestDuty();
        default:
            return 0U;
    }
}

void setUp(void)
{
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Init API verification
void test_GTM_TOM_3PhaseInverterPWM_init_CallsExpectedDriverAPIs(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Module + CMU clocks
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0U);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_selectClkInput() > 0U);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0U);

    // Unified PWM driver configuration + start
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0U);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0U);
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0U); // pin routing configured
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs() > 0U);
}

// 2) Init value/config verification (pins, clock source, initial duties 25/50/75)
void test_GTM_TOM_3PhaseInverterPWM_init_SetsExpectedConfigValues(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Clock selection uses GCLK as global source
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal());

    // Pin configuration: push-pull + pad driver IfxPort_PadDriver_cmosAutomotiveSpeed1
    TEST_ASSERT_EQUAL(IfxPort_OutputMode_pushPull, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode());
    TEST_ASSERT_EQUAL(IfxPort_PadDriver_cmosAutomotiveSpeed1, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver());

    // Initial duty staging: expect 25/50/75 percent on the three phases
    UpdateApi initApi = pick_api_called_on_init();
    TEST_ASSERT_MESSAGE(initApi != UPDATE_API_NONE, "PWM initial duty staging API was not called during init");

    float32 du = 0.0f, dv = 0.0f, dw = 0.0f;
    read_last_duties(initApi, &du, &dv, &dw);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_PERCENT_TOL, INITIAL_DUTY_PERCENT_U, du);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_PERCENT_TOL, INITIAL_DUTY_PERCENT_V, dv);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_PERCENT_TOL, INITIAL_DUTY_PERCENT_W, dw);
}

// 3) Single update call applies one step immediately and equally to all phases
void test_GTM_TOM_3PhaseInverterPWM_updateDuties_SingleCall_UpdatesValues(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Capture pre-update values from both APIs (whichever was used in init)
    UpdateApi initApi = pick_api_called_on_init();
    float32 preU_duty = 0.0f, preV_duty = 0.0f, preW_duty = 0.0f;
    if (initApi != UPDATE_API_NONE) {
        read_last_duties(initApi, &preU_duty, &preV_duty, &preW_duty);
    } else {
        // Fallback to documented initial values
        preU_duty = INITIAL_DUTY_PERCENT_U;
        preV_duty = INITIAL_DUTY_PERCENT_V;
        preW_duty = INITIAL_DUTY_PERCENT_W;
    }

    // Baseline call counts for update APIs
    uint32 beforeDutyCnt = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty();
    uint32 beforePulseCnt = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate();

    // Act
    GTM_TOM_3PhaseInverterPWM_updateDuties();

    // Detect which API performed the update, then read new values
    UpdateApi updApi = detect_update_api_used_after_call(beforeDutyCnt, beforePulseCnt);
    TEST_ASSERT_MESSAGE(updApi != UPDATE_API_NONE, "No PWM update API was invoked by updateDuties");

    float32 postU = 0.0f, postV = 0.0f, postW = 0.0f;
    read_last_duties(updApi, &postU, &postV, &postW);

    // Assert: one immediate step applied and equal across all three phases
    TEST_ASSERT_TRUE(postU > preU_duty);
    TEST_ASSERT_TRUE(postV > preV_duty);
    TEST_ASSERT_TRUE(postW > preW_duty);

    float32 dU = postU - preU_duty;
    float32 dV = postV - preV_duty;
    float32 dW = postW - preW_duty;

    TEST_ASSERT_FLOAT_WITHIN(0.01f, dU, dV);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, dU, dW);
}

// 4) Multiple updates progress by consistent step without drift; verify per-call history
void test_GTM_TOM_3PhaseInverterPWM_updateDuties_MultipleCalls_ProgressesCorrectly(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Baseline history counts before any explicit update() calls
    uint32 baseDutyHist = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_requestDuty();
    uint32 basePulseHist = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestDuty();

    // Two consecutive updates
    GTM_TOM_3PhaseInverterPWM_updateDuties();
    GTM_TOM_3PhaseInverterPWM_updateDuties();

    // Determine which API was used for updates (look at history growth)
    uint32 dutyHist = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_requestDuty();
    uint32 pulseHist = IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestDuty();

    UpdateApi updApi = UPDATE_API_NONE;
    uint32 baseHist = 0U, finalHist = 0U;
    if (dutyHist >= baseDutyHist + 2U) {
        updApi = UPDATE_API_DUTY;
        baseHist = baseDutyHist;
        finalHist = dutyHist;
    } else if (pulseHist >= basePulseHist + 2U) {
        updApi = UPDATE_API_PULSE_IMMEDIATE;
        baseHist = basePulseHist;
        finalHist = pulseHist;
    }
    TEST_ASSERT_MESSAGE(updApi != UPDATE_API_NONE, "PWM update API history did not increase as expected");

    // Analyze the last two update calls (indices baseHist and baseHist+1)
    float32 u0 = get_history_value(updApi, baseHist + 0U, 0U);
    float32 v0 = get_history_value(updApi, baseHist + 0U, 1U);
    float32 w0 = get_history_value(updApi, baseHist + 0U, 2U);

    float32 u1 = get_history_value(updApi, baseHist + 1U, 0U);
    float32 v1 = get_history_value(updApi, baseHist + 1U, 1U);
    float32 w1 = get_history_value(updApi, baseHist + 1U, 2U);

    float32 stepU = u1 - u0;
    float32 stepV = v1 - v0;
    float32 stepW = w1 - w0;

    TEST_ASSERT_TRUE(stepU > 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, stepU, stepV);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, stepU, stepW);

    // If there was at least one staging call before updates, verify absolute progression from that baseline
    if (baseHist > 0U) {
        float32 uPrev = get_history_value(updApi, baseHist - 1U, 0U);
        float32 expectedAfterTwo = uPrev + 2.0f * stepU;
        TEST_ASSERT_FLOAT_WITHIN(2.0f, expectedAfterTwo, u1);
    }

    (void)finalHist; // silence unused if compiled with strict warnings
}

// 5) Boundary wrap-around: detect a rollover where duty resets to the lower threshold
void test_GTM_TOM_3PhaseInverterPWM_updateDuties_BoundaryWrapAround(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Detect which API will be used for updates by performing one probe update
    uint32 beforeDuty = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty();
    uint32 beforePulse = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate();
    GTM_TOM_3PhaseInverterPWM_updateDuties();
    UpdateApi api = detect_update_api_used_after_call(beforeDuty, beforePulse);
    TEST_ASSERT_MESSAGE(api != UPDATE_API_NONE, "PWM update API not detected for wrap-around test");

    // Read current value after the probe
    float32 prevU = 0.0f, dummyV = 0.0f, dummyW = 0.0f;
    read_last_duties(api, &prevU, &dummyV, &dummyW);

    // Iterate updates until we observe a rollover (new < prev)
    bool wrapped = false;
    for (int i = 0; i < 600 && !wrapped; ++i) {
        GTM_TOM_3PhaseInverterPWM_updateDuties();
        float32 currU = 0.0f;
        read_last_duties(api, &currU, &dummyV, &dummyW);
        if (currU + SMALL_TOL < prevU) {
            wrapped = true; // wrap-around detected
            break;
        }
        prevU = currU;
    }

    TEST_ASSERT_TRUE(wrapped);
}

// 6) Update must not re-run any init sequence APIs
void test_GTM_TOM_3PhaseInverterPWM_updateDuties_DoesNotReInit(void)
{
    GTM_TOM_3PhaseInverterPWM_init();

    // Record init-phase call counts
    uint32 c_enable        = IfxGtm_Mock_GetCallCount_enable();
    uint32 c_selClk        = IfxGtm_Cmu_Mock_GetCallCount_selectClkInput();
    uint32 c_enClocks      = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    uint32 c_initCfg       = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 c_init          = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 c_pinMap        = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    uint32 c_startOutputs  = IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs();

    // Perform several updates
    for (int i = 0; i < 5; ++i) {
        GTM_TOM_3PhaseInverterPWM_updateDuties();
    }

    // Assert none of the init-time APIs were called again
    TEST_ASSERT_EQUAL_UINT32(c_enable,       IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(c_selClk,       IfxGtm_Cmu_Mock_GetCallCount_selectClkInput());
    TEST_ASSERT_EQUAL_UINT32(c_enClocks,     IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL_UINT32(c_initCfg,      IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(c_init,         IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(c_pinMap,       IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL_UINT32(c_startOutputs, IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs());
}
