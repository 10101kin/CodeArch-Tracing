#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"
#include <stdint.h>

/* Extern declarations for production-local functions (callback/ISR) */
extern void IfxGtm_periodEventFunction(void *data);
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_DT_RISING_US                  (1.0f)
#define UT_DT_FALLING_US                 (1.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)

/* Helpers */
static void performNUpdates(uint32_t n)
{
    for (uint32_t i = 0; i < n; ++i)
    {
        updateGtmTom3phInvDuty();
    }
}

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/*********************
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 *********************/
void test_TC_G1_001_Init_WhenGtmAlreadyEnabled_SkipsEnableAndClocks_ConfigAndInitOnce(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must not be re-enabled if already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must not be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must not be re-enabled when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig() must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init() must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED/debug GPIO must be configured once");
}

void test_TC_G1_002_Init_WhenGtmDisabled_EnablesModule_ConfiguresClocks_ThenInit(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* Disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK clocks must be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig() must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init() must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED/debug GPIO must be configured once");
}

/*********************
 * GROUP 2 - initGtmTom3phInv: configuration values
 *********************/
void test_TC_G2_001_Init_Configures3Channels_At20kHz(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert (assert on init spy, not initConfig spy) */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase complementary PWM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_Init_FrequencyIndependentFromModuleClock(void)
{
    /* Arrange - force GTM disabled so CMU freq path is taken */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 100000000U; /* 100 MHz module clock */

    /* Act */
    initGtmTom3phInv();

    /* Assert - PWM frequency must still be 20 kHz */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency in config must be the switching frequency, not CMU clock");
}

/*********************
 * GROUP 3 - initGtmTom3phInv: runtime update logic
 * (verifies update behavior after initialization)
 *********************/
void test_TC_G3_001_FirstUpdate_IncrementsDutiesByStep_OneHalCall(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert - 25->35, 50->60, 75->85 (fractions) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One HAL call per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.35f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 35% (0.35)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 60% (0.60)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.85f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 85% (0.85)");
}

void test_TC_G3_002_ThirdUpdate_WrapsPhaseW_To10Percent_CallsOncePerUpdate(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act - three updates */
    performNUpdates(3);

    /* Assert - U:55% (0.55), V:80% (0.80), W wraps to 10% (0.10) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three updates must trigger three HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.55f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty after 3 updates must be 55% (0.55)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.80f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty after 3 updates must be 80% (0.80)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty after 3 updates must wrap to 10% (0.10)");
}

/*********************
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
 *********************/
void test_TC_G4_001_ISR_TogglesLedOnce(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_G4_002_ISR_Toggle_AccumulatesAcrossCalls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "LED toggle count must accumulate across ISR calls");
}

/*********************
 * GROUP 5 - updateGtmTom3phInvDuty: configuration values
 *********************/
void test_TC_G5_001_Update_ProducesThreeNormalizedDuties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver configured for 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.35f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U normalized duty present");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V normalized duty present");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.85f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W normalized duty present");
}

void test_TC_G5_002_Update_NormalizedDuties_AreWithin0and1(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 1.0f, "U duty must be within [0,1]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 1.0f, "V duty must be within [0,1]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 1.0f, "W duty must be within [0,1]");
}

/*********************
 * GROUP 6 - updateGtmTom3phInvDuty: runtime update logic
 *********************/
void test_TC_G6_001_FiveUpdates_V_WrapsTo10_UStillBelowWrap(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    performNUpdates(5);

    /* After 5 updates: U=75% (0.75), V=10% (0.10), W=30% (0.30) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Five updates must trigger five HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.75f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U after 5 updates must be 75% (0.75)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V after 5 updates must wrap to 10% (0.10)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.30f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W after 5 updates must be 30% (0.30)");
}

void test_TC_G6_002_EightUpdates_U_WrapsTo10_VAndWProgressAccordingly(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    performNUpdates(8);

    /* After 8 updates: U=10% (0.10), V=40% (0.40), W=60% (0.60) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates must trigger eight HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.10f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U after 8 updates must wrap to 10% (0.10)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.40f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V after 8 updates must be 40% (0.40)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.60f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W after 8 updates must be 60% (0.60)");
}

/*********************
 * GROUP 7 - IfxGtm_periodEventFunction: configuration values
 *********************/
void test_TC_G7_001_PeriodEventCallback_DoesNothing_KeepsPwmConfigUntouched(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - Config spies from init remain as configured */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Num channels must remain 3 after period callback");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after period callback");
}

void test_TC_G7_002_PeriodEventCallback_WithoutInit_DoesNotCallHal(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert - No HAL duty update or toggles */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

/*********************
 * GROUP 8 - IfxGtm_periodEventFunction: ISR / interrupt behavior
 * (verify that the empty period callback itself does not toggle; ISR handles toggling)
 *********************/
void test_TC_G8_001_PeriodEventCallback_DoesNotToggleLed(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G8_002_PeriodEventCallback_MultipleCalls_DoNotToggleLed(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Multiple period callbacks must not toggle LED");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_Init_WhenGtmAlreadyEnabled_SkipsEnableAndClocks_ConfigAndInitOnce);
    RUN_TEST(test_TC_G1_002_Init_WhenGtmDisabled_EnablesModule_ConfiguresClocks_ThenInit);

    RUN_TEST(test_TC_G2_001_Init_Configures3Channels_At20kHz);
    RUN_TEST(test_TC_G2_002_Init_FrequencyIndependentFromModuleClock);

    RUN_TEST(test_TC_G3_001_FirstUpdate_IncrementsDutiesByStep_OneHalCall);
    RUN_TEST(test_TC_G3_002_ThirdUpdate_WrapsPhaseW_To10Percent_CallsOncePerUpdate);

    RUN_TEST(test_TC_G4_001_ISR_TogglesLedOnce);
    RUN_TEST(test_TC_G4_002_ISR_Toggle_AccumulatesAcrossCalls);

    RUN_TEST(test_TC_G5_001_Update_ProducesThreeNormalizedDuties);
    RUN_TEST(test_TC_G5_002_Update_NormalizedDuties_AreWithin0and1);

    RUN_TEST(test_TC_G6_001_FiveUpdates_V_WrapsTo10_UStillBelowWrap);
    RUN_TEST(test_TC_G6_002_EightUpdates_U_WrapsTo10_VAndWProgressAccordingly);

    RUN_TEST(test_TC_G7_001_PeriodEventCallback_DoesNothing_KeepsPwmConfigUntouched);
    RUN_TEST(test_TC_G7_002_PeriodEventCallback_WithoutInit_DoesNotCallHal);

    RUN_TEST(test_TC_G8_001_PeriodEventCallback_DoesNotToggleLed);
    RUN_TEST(test_TC_G8_002_PeriodEventCallback_MultipleCalls_DoNotToggleLed);

    return UNITY_END();
}
