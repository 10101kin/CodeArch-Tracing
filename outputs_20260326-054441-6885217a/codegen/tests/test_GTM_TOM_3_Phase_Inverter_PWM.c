#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

// Mocked iLLD headers
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "Ifx_Types.h"

// Structured configuration values (from requirements)
#define INITIAL_DUTY_CYCLE_PERCENT_U   (25.0f)
#define INITIAL_DUTY_CYCLE_PERCENT_V   (50.0f)
#define INITIAL_DUTY_CYCLE_PERCENT_W   (75.0f)
#define TIMING_PWM_FREQUENCY_HZ        (20000.0f)
#define TIMING_DEAD_TIME_SECONDS       (5e-07f)
#define TIMING_MIN_PULSE_SECONDS       (1e-06f)

// Tolerances for float comparisons
#define DUTY_TOLERANCE_PERCENT         (1.0f)
#define FREQ_TOLERANCE_HZ              (1.0f)
#define STEP_TOLERANCE_PERCENT         (1.0f)

void setUp(void)
{
    // Reset all mock state before each test
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) INIT: Verify all expected driver APIs are invoked
void test_GTM_TOM_3_Phase_Inverter_PWM_init_CallsExpectedDriverAPIs(void)
{
    // Act
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Assert — each expected iLLD call must be made at least once
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateFrequencyImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDeadTimeImmediate() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate() > 0);

    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs() > 0);
}

// 2) INIT: Verify configuration values (frequency, pin mapping mode/driver, initial duties)
void test_GTM_TOM_3_Phase_Inverter_PWM_init_SetsExpectedConfigValues(void)
{
    // Act
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // A) Frequency programmed to 20 kHz immediately
    float32 f_immediate = IfxGtm_Pwm_Mock_GetLastArg_updateFrequencyImmediate();
    TEST_ASSERT_FLOAT_WITHIN(FREQ_TOLERANCE_HZ, TIMING_PWM_FREQUENCY_HZ, f_immediate);

    // B) Pin mapping output mode and pad driver as required
    TEST_ASSERT_EQUAL(pushPull, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode());
    TEST_ASSERT_EQUAL(cmosAutomotiveSpeed1, IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver());

    // C) Initial duty vector (U=25%, V=50%, W=75%) applied via a synchronous update
    float32 dutyU = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 dutyV = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 dutyW = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_CYCLE_PERCENT_U, dutyU);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_CYCLE_PERCENT_V, dutyV);
    TEST_ASSERT_FLOAT_WITHIN(DUTY_TOLERANCE_PERCENT, INITIAL_DUTY_CYCLE_PERCENT_W, dutyW);
}

// 3) UPDATE: Single call applies one step to all three phases synchronously
void test_GTM_TOM_3_Phase_Inverter_PWM_updateUVW_SingleCall_UpdatesValues(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();
    uint32 beforeCount = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();

    float32 u0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w0 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    // Act — one update step
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW();

    // Assert — call count increased by 1
    uint32 afterCount = IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate();
    TEST_ASSERT_EQUAL_UINT32(beforeCount + 1u, afterCount);

    // Assert — all channels progressed by the same step (wrap allowed)
    float32 u1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
    float32 v1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(1);
    float32 w1 = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(2);

    float32 du = u1 - u0;
    float32 dv = v1 - v0;
    float32 dw = w1 - w0;

    // Direction consistency: either all increased or all wrapped (decreased)
    TEST_ASSERT_TRUE(((du >= 0.0f) && (dv >= 0.0f) && (dw >= 0.0f)) ||
                     ((du <= 0.0f) && (dv <= 0.0f) && (dw <= 0.0f)));

    // Step magnitude consistent across phases
    TEST_ASSERT_FLOAT_WITHIN(STEP_TOLERANCE_PERCENT, du, dv);
    TEST_ASSERT_FLOAT_WITHIN(STEP_TOLERANCE_PERCENT, du, dw);
}

// 4) UPDATE: Multiple calls progress with a constant step per call
void test_GTM_TOM_3_Phase_Inverter_PWM_updateUVW_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Act — two update steps
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW(); // history index 1 (after init at index 0)
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW(); // history index 2

    // Assert — use call history to verify constant step progression on U, and step consistency across U/V/W
    // History indices: 0 = init's initial duty update, 1 = first update, 2 = second update
    float32 u_init = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 0u);
    float32 v_init = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 1u);
    float32 w_init = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(0u, 2u);

    float32 u_1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 0u);
    float32 v_1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 1u);
    float32 w_1 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(1u, 2u);

    float32 u_2 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(2u, 0u);
    float32 v_2 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(2u, 1u);
    float32 w_2 = IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(2u, 2u);

    float32 du1 = u_1 - u_init;
    float32 du2 = u_2 - u_1;

    // Constant step per successive call on U
    TEST_ASSERT_FLOAT_WITHIN(STEP_TOLERANCE_PERCENT, du1, du2);

    // Step consistency across phases (for the first update step)
    float32 dv1 = v_1 - v_init;
    float32 dw1 = w_1 - w_init;
    TEST_ASSERT_FLOAT_WITHIN(STEP_TOLERANCE_PERCENT, du1, dv1);
    TEST_ASSERT_FLOAT_WITHIN(STEP_TOLERANCE_PERCENT, du1, dw1);
}

// 5) UPDATE: Boundary wrap-around when reaching maximum duty -> wraps to minimum duty
void test_GTM_TOM_3_Phase_Inverter_PWM_updateUVW_BoundaryWrapAround(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    float32 prev = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0); // track phase U
    bool wrapped = false;

    // Act — iterate updates until a wrap is observed (value decreases), or until safety cap
    const int maxIterations = 400; // bounded to keep test fast; algorithm step is fixed but unknown
    for (int i = 0; i < maxIterations; ++i)
    {
        GTM_TOM_3_Phase_Inverter_PWM_updateUVW();
        float32 curr = IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(0);
        if (curr < prev) { wrapped = true; break; }
        prev = curr;
    }

    // Assert — wrap must occur eventually
    TEST_ASSERT_TRUE(wrapped);
}

// 6) UPDATE: Must not re-initialize or re-configure hardware paths
void test_GTM_TOM_3_Phase_Inverter_PWM_updateUVW_DoesNotReInit(void)
{
    // Arrange
    GTM_TOM_3_Phase_Inverter_PWM_init();

    // Snapshot init-time call counts
    uint32 cc_initConfig          = IfxGtm_Pwm_Mock_GetCallCount_initConfig();
    uint32 cc_initChannelConfig   = IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig();
    uint32 cc_init                = IfxGtm_Pwm_Mock_GetCallCount_init();
    uint32 cc_startSynced         = IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels();
    uint32 cc_startOutputs        = IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs();
    uint32 cc_pinMap              = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();
    uint32 cc_gtmEnable           = IfxGtm_Mock_GetCallCount_enable();
    uint32 cc_cmuEnableClocks     = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();

    // Act — perform several runtime updates
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW();
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW();
    GTM_TOM_3_Phase_Inverter_PWM_updateUVW();

    // Assert — none of the init-phase APIs are re-called
    TEST_ASSERT_EQUAL_UINT32(cc_initConfig,        IfxGtm_Pwm_Mock_GetCallCount_initConfig());
    TEST_ASSERT_EQUAL_UINT32(cc_initChannelConfig, IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig());
    TEST_ASSERT_EQUAL_UINT32(cc_init,              IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL_UINT32(cc_startSynced,       IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels());
    TEST_ASSERT_EQUAL_UINT32(cc_startOutputs,      IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs());
    TEST_ASSERT_EQUAL_UINT32(cc_pinMap,            IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
    TEST_ASSERT_EQUAL_UINT32(cc_gtmEnable,         IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL_UINT32(cc_cmuEnableClocks,   IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
}
