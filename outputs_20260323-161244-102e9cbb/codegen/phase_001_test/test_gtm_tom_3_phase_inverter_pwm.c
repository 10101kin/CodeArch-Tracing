#include "unity.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

// iLLD mock headers (only use available mock controls)
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

// Structured configuration values (exact from requirements)
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U  (25.0f)
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V  (50.0f)
#define INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W  (75.0f)
#define TIMING_FREQUENCY_HZ                     (20000.0f)
#define TIMING_FXCLK_FREQUENCY_MHZ              (100.0f)
#define TIMING_PRESCALER                        (1U)
#define TIMING_TIMER_BASE_PERIOD_TICKS          (2500.0f)
#define CLOCK_REQUIRES_XTAL                     (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ          (300.0f)
#define CLOCK_GTM_FXCLK_MHZ                     (100.0f)

// Derived behavior constants from design description
// Update step: +10 percentage points each call; wrap at 100% -> 0%
#define DUTY_STEP_PERCENT                       (10.0f)
#define MAX_DUTY_PERCENT                        (100.0f)

// Tolerance for float comparisons (percent units)
#define PCT_TOLERANCE                           (0.001f)

void setUp(void)
{
    // Reset all mocks before each test (no arguments)
    IfxGtm_Mock_Reset();
    IfxGtm_Cmu_Mock_Reset();
    IfxGtm_Tom_Timer_Mock_Reset();
    IfxGtm_Pwm_Mock_Reset();
    IfxGtm_PinMap_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Init should call key driver APIs
void test_initGtmTomPwm_CallsExpectedDriverAPIs(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);

    // Act
    initGtmTomPwm();

    // Assert (GetCallCount > 0 for each expected call we can measure)
    TEST_ASSERT_TRUE(IfxGtm_Mock_GetCallCount_enable() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Cmu_Mock_GetCallCount_enableClocks() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetCallCount_init() > 0);
    TEST_ASSERT_TRUE(IfxGtm_PinMap_Mock_GetCallCount_setTomTout() > 0);
}

// 2) Init should set expected configuration values (frequency, alignment/sync as available)
void test_initGtmTomPwm_SetsExpectedConfigValues(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);

    // Act
    initGtmTomPwm();

    // Assert: verify timer and PWM init frequencies from mocks
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, TIMING_FREQUENCY_HZ,
        IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency());

    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, TIMING_FREQUENCY_HZ,
        IfxGtm_Pwm_Mock_GetLastArg_init_frequency());

    // Optional behavior flags captured by mocks (center-aligned + sync start)
    // We verify sync start is enabled (synchronized updates per design)
    TEST_ASSERT_TRUE(IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart());
    TEST_ASSERT_TRUE(IfxGtm_Pwm_Mock_GetLastArg_init_syncStart());

    // Functional behavior: initial duty values in module state (six channels)
    // High-side: U=25, V=50, W=75; Low-side: same numeric values (complement via polarity)
    // Access the production module's extern-visible driver state as declared in its header
    extern struct GTM_TOM_3_Phase_Inverter_PWM_State g_GTM_TOM_3_Phase_Inverter_PWM;

    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2]);

    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_U,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[3]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_V,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[4]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, INITIAL_HIGH_SIDE_DUTY_PERCENT_PHASE_W,
        g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[5]);
}

// 3) Single update applies one +10% step (with low sides mirroring high sides numerically)
void test_updateGtmTomPwmDutyCycles_SingleCall_UpdatesValues(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);
    initGtmTomPwm();

    extern struct GTM_TOM_3_Phase_Inverter_PWM_State g_GTM_TOM_3_Phase_Inverter_PWM;

    // Capture pre-update values
    float beforeU = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0];
    float beforeV = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1];
    float beforeW = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2];

    // Act
    updateGtmTomPwmDutyCycles();

    // Compute expected after 1 update
    float expU = beforeU + DUTY_STEP_PERCENT; if (expU > MAX_DUTY_PERCENT) expU = 0.0f;
    float expV = beforeV + DUTY_STEP_PERCENT; if (expV > MAX_DUTY_PERCENT) expV = 0.0f;
    float expW = beforeW + DUTY_STEP_PERCENT; if (expW > MAX_DUTY_PERCENT) expW = 0.0f;

    // Assert high-side duties progressed by +10% with wrap at 100%
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expU, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expV, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expW, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2]);

    // Low-sides mirror high-sides numerically (complement via polarity set in init)
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[3]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[4]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[5]);
}

// 4) Multiple updates progress linearly by N * 10%
void test_updateGtmTomPwmDutyCycles_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);
    initGtmTomPwm();

    extern struct GTM_TOM_3_Phase_Inverter_PWM_State g_GTM_TOM_3_Phase_Inverter_PWM;

    float baseU = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0];
    float baseV = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1];
    float baseW = g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2];

    // Act: 2 updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Compute expected after 2 steps with wrap
    float expU = baseU + 2.0f * DUTY_STEP_PERCENT; while (expU > MAX_DUTY_PERCENT) expU -= MAX_DUTY_PERCENT + 0.0f; if (expU > MAX_DUTY_PERCENT) expU = 0.0f;
    float expV = baseV + 2.0f * DUTY_STEP_PERCENT; while (expV > MAX_DUTY_PERCENT) expV -= MAX_DUTY_PERCENT + 0.0f; if (expV > MAX_DUTY_PERCENT) expV = 0.0f;
    float expW = baseW + 2.0f * DUTY_STEP_PERCENT; while (expW > MAX_DUTY_PERCENT) expW -= MAX_DUTY_PERCENT + 0.0f; if (expW > MAX_DUTY_PERCENT) expW = 0.0f;

    // Assert
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expU, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expV, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, expW, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2]);

    // Low-sides mirror high-sides numerically
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[0], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[3]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[1], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[4]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[5]);
}

// 5) Boundary wrap-around: when exceeding 100%, wrap to 0%
void test_updateGtmTomPwmDutyCycles_BoundaryWrapAround(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);
    initGtmTomPwm();

    extern struct GTM_TOM_3_Phase_Inverter_PWM_State g_GTM_TOM_3_Phase_Inverter_PWM;

    // Compute iterations needed to force wrap for the channel with highest initial duty (75%)
    // N = ceil((100 - 75) / 10) = 3 → 75 -> 85 -> 95 -> 105 -> wraps to 0
    unsigned int iterations = 3U;

    // Act
    for (unsigned int i = 0; i < iterations; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    // Assert: at least the W high-side should have wrapped to 0
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, 0.0f, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2]);
    TEST_ASSERT_FLOAT_WITHIN(PCT_TOLERANCE, g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[2], g_GTM_TOM_3_Phase_Inverter_PWM.dutyCycles[5]);
}

// 6) Update must not re-initialize drivers
void test_updateGtmTomPwmDutyCycles_DoesNotReInit(void)
{
    // Arrange
    IfxGtm_Tom_Timer_Mock_SetReturn_init(1);
    initGtmTomPwm();

    // Record call counts right after init
    unsigned int gtmEnableBefore   = IfxGtm_Mock_GetCallCount_enable();
    unsigned int cmuEnableBefore   = IfxGtm_Cmu_Mock_GetCallCount_enableClocks();
    unsigned int timerInitBefore   = IfxGtm_Tom_Timer_Mock_GetCallCount_init();
    unsigned int pwmInitBefore     = IfxGtm_Pwm_Mock_GetCallCount_init();
    unsigned int pinMapBefore      = IfxGtm_PinMap_Mock_GetCallCount_setTomTout();

    // Act: perform several updates
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();
    updateGtmTomPwmDutyCycles();

    // Assert: no re-init calls occurred
    TEST_ASSERT_EQUAL(gtmEnableBefore, IfxGtm_Mock_GetCallCount_enable());
    TEST_ASSERT_EQUAL(cmuEnableBefore, IfxGtm_Cmu_Mock_GetCallCount_enableClocks());
    TEST_ASSERT_EQUAL(timerInitBefore, IfxGtm_Tom_Timer_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL(pwmInitBefore,   IfxGtm_Pwm_Mock_GetCallCount_init());
    TEST_ASSERT_EQUAL(pinMapBefore,    IfxGtm_PinMap_Mock_GetCallCount_setTomTout());
}
