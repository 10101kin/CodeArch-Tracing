#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern for driver callback as per rules */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_NUM_CHANNELS                  (3U)
#define UT_DUTY_U_INIT                   (0.25f)
#define UT_DUTY_V_INIT                   (0.50f)
#define UT_DUTY_W_INIT                   (0.75f)
#define UT_DUTY_STEP                     (0.10f)
#define UT_DT_RISE_US                    (1.0f)
#define UT_DT_FALL_US                    (1.0f)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/********************
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 ********************/
void test_TC_G1_001_Init_PeripheralDisabled_ConfiguresClocksAndPWM(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled -> expect enable + CMU setup */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must be called when GTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be read once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_selectClkInput_getCallCount(), "CLK input selection must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "TOM CLK frequency must be set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK must be enabled once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinState_getCallCount(), "LED pin initial state must be set once");
}

void test_TC_G1_002_Init_PeripheralEnabled_SkipsClockSetupButInitsPWM(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled -> expect no enable/CMU calls */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "IfxGtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "IfxGtm_enable must NOT be called when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must NOT be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_selectClkInput_getCallCount(), "CLK input selection must NOT be configured when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "TOM CLK frequency must NOT be set when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK must NOT be enabled again when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must still be called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once even if GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinState_getCallCount(), "LED pin initial state must be set once even if GTM already enabled");
}

/********************
 * GROUP 2 - initGtmTom3phInv: configuration values
 ********************/
void test_TC_G2_001_Init_SetsFrequencyAndNumChannels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM init frequency must be 20 kHz (switching frequency)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM must be configured for exactly 3 logical channels");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must declare 3 channels");
}

void test_TC_G2_002_Init_ConfiguresLedOutputLow(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be set as push-pull output once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinState_getCallCount(), "LED pin must be driven to initial low level once");
}

/********************
 * GROUP 3 - initGtmTom3phInv: runtime update logic (integration check post-init)
 ********************/
void test_TC_G3_001_UpdateOnce_AppliesIncrementFromInitialDuties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be issued exactly once for one update call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_U_INIT + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increase by 10% from 25% to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_V_INIT + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increase by 10% from 50% to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DUTY_W_INIT + UT_DUTY_STEP, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increase by 10% from 75% to 85%");
}

void test_TC_G3_002_UpdateMultiple_WrapIndependently(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 3; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert after 3 updates: U=0.55, V=0.80, W wraps to 0.10 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.55f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 55% after three 10% steps from 25%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.80f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 80% after three 10% steps from 50%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 10% after exceeding 100%");
}

/********************
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
 ********************/
void test_TC_G4_001_Irq_TogglesLedOnce(void)
{
    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED exactly once");
}

void test_TC_G4_002_Irq_TogglesLedAccumulated(void)
{
    /* Act */
    interruptGtmAtom0Ch0();
    interruptGtmAtom0Ch0();
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED once per invocation (3 total)");
}

/********************
 * GROUP 5 - updateGtmTom3phInvDuty: configuration values
 ********************/
void test_TC_G5_001_Update_FirstCall_UsesNumChannels3AndFrequencyUnchanged(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert (static config checks remain consistent) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "PWM must remain configured for 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency must remain 20 kHz");
}

void test_TC_G5_002_Update_AppliesExpectedDutiesAfterFiveCalls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* After 5 updates: U=0.75, V wraps on exact 1.0 -> 0.10, W wrapped at 3rd -> then 0.30 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.75f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 75% after five steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap to 10% on reaching 100%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.30f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 30% after wrap at step 3 and two more steps");
}

/********************
 * GROUP 6 - updateGtmTom3phInvDuty: runtime update logic
 ********************/
void test_TC_G6_001_Update_CallCountIncrementsPerInvocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 7; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(7, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called once per update invocation (7 times)");
}

void test_TC_G6_002_Update_WrapOnExactOnePointZero(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert: Phase V hits exactly 1.0 at 5th step -> wraps to 0.10 */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap to 10% when reaching exactly 100%");
}

/********************
 * GROUP 7 - IfxGtm_periodEventFunction: configuration values
 ********************/
void test_TC_G7_001_PeriodCallback_DoesNothing_NoDriverCalls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "Period callback must not configure pins");
}

void test_TC_G7_002_PeriodCallback_DoesNotTriggerInitCalls(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_initConfig_getCallCount(), "Period callback must not call initConfig");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "Period callback must not call init");
}

/********************
 * GROUP 8 - IfxGtm_periodEventFunction: ISR / interrupt behavior (no-ops)
 ********************/
void test_TC_G8_001_PeriodCallback_MultipleCalls_NoDriverCallsAccumulate(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED even after multiple calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties even after multiple calls");
}

void test_TC_G8_002_PeriodCallback_NullDataAccepted_NoSideEffects(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "Period callback must not enable GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Period callback must not touch CMU clocks");
}

/********************
 * GROUP 9 - interruptGtmAtom0Ch0: initialization / enable guard (none expected)
 ********************/
void test_TC_G9_001_Irq_DoesNotTouchGtmEnableGuard_WhenGtmDisabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;

    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_isEnabled_getCallCount(), "ISR must not query GTM enable state");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "ISR must not enable GTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED regardless of GTM state");
}

void test_TC_G9_002_Irq_DoesNotTouchGtmEnableGuard_WhenGtmEnabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_isEnabled_getCallCount(), "ISR must not query GTM enable state when GTM enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "ISR must not enable GTM when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED");
}

/********************
 * GROUP 10 - interruptGtmAtom0Ch0: ISR / interrupt behavior
 ********************/
void test_TC_G10_001_Irq_MultipleToggles_Accumulate(void)
{
    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        interruptGtmAtom0Ch0();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED on every invocation (5 times)");
}

void test_TC_G10_002_Irq_DoesNotAffectOtherDrivers(void)
{
    /* Act */
    interruptGtmAtom0Ch0();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ISR must not configure pins");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinState_getCallCount(), "ISR must not set pin state");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not update PWM duties");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_Init_PeripheralDisabled_ConfiguresClocksAndPWM);
    RUN_TEST(test_TC_G1_002_Init_PeripheralEnabled_SkipsClockSetupButInitsPWM);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_Init_SetsFrequencyAndNumChannels);
    RUN_TEST(test_TC_G2_002_Init_ConfiguresLedOutputLow);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_UpdateOnce_AppliesIncrementFromInitialDuties);
    RUN_TEST(test_TC_G3_002_UpdateMultiple_WrapIndependently);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_Irq_TogglesLedOnce);
    RUN_TEST(test_TC_G4_002_Irq_TogglesLedAccumulated);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_Update_FirstCall_UsesNumChannels3AndFrequencyUnchanged);
    RUN_TEST(test_TC_G5_002_Update_AppliesExpectedDutiesAfterFiveCalls);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_Update_CallCountIncrementsPerInvocation);
    RUN_TEST(test_TC_G6_002_Update_WrapOnExactOnePointZero);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_PeriodCallback_DoesNothing_NoDriverCalls);
    RUN_TEST(test_TC_G7_002_PeriodCallback_DoesNotTriggerInitCalls);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_PeriodCallback_MultipleCalls_NoDriverCallsAccumulate);
    RUN_TEST(test_TC_G8_002_PeriodCallback_NullDataAccepted_NoSideEffects);

    /* GROUP 9 */
    RUN_TEST(test_TC_G9_001_Irq_DoesNotTouchGtmEnableGuard_WhenGtmDisabled);
    RUN_TEST(test_TC_G9_002_Irq_DoesNotTouchGtmEnableGuard_WhenGtmEnabled);

    /* GROUP 10 */
    RUN_TEST(test_TC_G10_001_Irq_MultipleToggles_Accumulate);
    RUN_TEST(test_TC_G10_002_Irq_DoesNotAffectOtherDrivers);

    return UNITY_END();
}
