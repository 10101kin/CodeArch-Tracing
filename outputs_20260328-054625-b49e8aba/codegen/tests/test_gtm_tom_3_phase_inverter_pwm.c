#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declaration for driver period-event callback implemented in production .c */
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                         (1e-4f)
#define UT_PWM_FREQUENCY_HZ                      (20000.0f)
#define UT_NUM_CHANNELS                          (3U)
#define UT_DEADTIME_NS                           (1000U)
#define UT_DUTY_INIT_U                           (25.0f)
#define UT_DUTY_INIT_V                           (50.0f)
#define UT_DUTY_INIT_W                           (75.0f)
#define UT_DUTY_STEP                              (10.0f)
#define UT_DUTY_WRAP_THRESHOLD                   (100.0f)
#define UT_ISR_PRIORITY                           (20U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_Init_Disabled_Peripheral_Enables_GTM_and_Clocks(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must be checked exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "GTM CMU clocks must be enabled for TOM when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug/LED pin must be configured once during init");
}

void test_TC_G1_002_Init_AlreadyEnabled_Skips_Enable_And_Clock(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must be checked exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "GTM CMU clock enable must be skipped when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must still be called once");
}

/* GROUP 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_Init_Sets_Frequency_And_ChannelCount_Correctly(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Keep GTM enabled to focus on config */

    /* Act */
    initGtmTom3phInv();

    /* Assert - frequency is PWM switching frequency (20 kHz), not CMU clock */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_initConfig_lastFrequency, "initConfig.frequency must be 20 kHz");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_init_lastFrequency, "init.frequency must be 20 kHz");

    /* Assert - three complementary pairs (3 logical channels) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must be created for 3 logical channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "init must be applied for 3 logical channels");
}

void test_TC_G2_002_Init_InitialDuties_Implied_By_First_Update_From_25_50_75(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* Expect 25/50/75 -> 35/60/85 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_INIT_U + UT_DUTY_STEP), (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must step from 25 to 35 percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_INIT_V + UT_DUTY_STEP), (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must step from 50 to 60 percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_DUTY_INIT_W + UT_DUTY_STEP), (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must step from 75 to 85 percent");
}

/* GROUP 3 - initGtmTom3phInv: runtime update logic (derived from initial state) */
void test_TC_G3_001_FirstUpdate_Increments_Duties_When_Below_Boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* 25/50/75 -> 35/60/85 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be 35 percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 60 percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 85 percent");
}

void test_TC_G3_002_ThirdUpdate_Wraps_Only_W_Channel_Independently(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* 25/50/75 -> 35/60/85 */
    updateGtmTom3phInvDuty(); /* 35/60/85 -> 45/70/95 */
    updateGtmTom3phInvDuty(); /* 45/70/95 -> 55/80/0 (W wraps) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update; after 3 updates call count must be 3");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must not wrap at 55 percent after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must not wrap at 80 percent after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,  (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 0 percent after crossing 100");
}

/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior */
void test_TC_G4_001_PeriodEvent_Toggles_Debug_Pin_Once_Per_Call(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv(); /* Ensure ISR-related configuration is nominal */

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert: Either direct IfxPort_togglePin() or a wrapper may be used; sum both counters */
    unsigned int totalToggles = mock_IfxPort_togglePin_getCallCount() + mock_togglePin_getCallCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, totalToggles, "Period-event callback must toggle debug pin exactly once per call");
}

void test_TC_G4_002_PeriodEvent_Toggle_Accumulates_Across_Multiple_Calls(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    unsigned int totalToggles = mock_IfxPort_togglePin_getCallCount() + mock_togglePin_getCallCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, totalToggles, "Period-event callback must toggle debug pin once per invocation (3 total)");
}

/* GROUP 5 - updateGtmTom3phInvDuty: configuration values */
void test_TC_G5_001_Update_Uses_Three_Channel_Array_And_Applies_All_Duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    updateGtmTom3phInvDuty(); /* Expect 35/60/85 */

    /* Assert - confirm 3-phase content and that init configured 3 channels */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver must be initialized for exactly 3 logical channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be updated");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be updated");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be updated");
}

void test_TC_G5_002_Frequency_Remains_At_20kHz_After_Update_Call(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    updateGtmTom3phInvDuty();

    /* Assert - PWM switching frequency remains 20 kHz as configured during init */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, (float)mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must remain at 20 kHz after runtime updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL duty update call expected");
}

/* GROUP 6 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G6_001_EightUpdates_Produce_Expected_Duty_Sequence_With_Wraps(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }
    /* After 8 steps from 25/50/75 -> expected 0/30/50 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be invoked once per step (8 total)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,  (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must wrap to 0 after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must be 30 after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, (float)mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must be 50 after 8 steps");
}

void test_TC_G6_002_HalUpdate_Called_Once_Per_Update_Not_Per_Channel(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();
    const unsigned steps = 5U;
    for (unsigned i = 0; i < steps; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(steps, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once per update call (not per channel)");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_Init_Disabled_Peripheral_Enables_GTM_and_Clocks);
    RUN_TEST(test_TC_G1_002_Init_AlreadyEnabled_Skips_Enable_And_Clock);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_Init_Sets_Frequency_And_ChannelCount_Correctly);
    RUN_TEST(test_TC_G2_002_Init_InitialDuties_Implied_By_First_Update_From_25_50_75);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_FirstUpdate_Increments_Duties_When_Below_Boundary);
    RUN_TEST(test_TC_G3_002_ThirdUpdate_Wraps_Only_W_Channel_Independently);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_PeriodEvent_Toggles_Debug_Pin_Once_Per_Call);
    RUN_TEST(test_TC_G4_002_PeriodEvent_Toggle_Accumulates_Across_Multiple_Calls);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_Update_Uses_Three_Channel_Array_And_Applies_All_Duties);
    RUN_TEST(test_TC_G5_002_Frequency_Remains_At_20kHz_After_Update_Call);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_EightUpdates_Produce_Expected_Duty_Sequence_With_Wraps);
    RUN_TEST(test_TC_G6_002_HalUpdate_Called_Once_Per_Update_Not_Per_Channel);

    return UNITY_END();
}
