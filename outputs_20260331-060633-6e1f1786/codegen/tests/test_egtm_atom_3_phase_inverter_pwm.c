#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR and period callback implemented in production .c */
extern void interruptEgtmAtom(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_DEADTIME_RISING_US            (1.0f)
#define UT_DEADTIME_FALLING_US           (1.0f)
#define UT_ISR_PRIORITY                  (20U)
#define UT_WAIT_TIME_MS                  (500.0f)
#define UT_FXCLK0_ASSUMED_FREQUENCY_HZ   (100000000U)

void setUp(void)
{
    mock_egtm_atom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/***********************
 * GROUP 1 - init: enable guard
 ***********************/
void test_TC_G01_001_Init_Calls_Target_APIs_When_EGTM_Disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* Force enable path */
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read during enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "EGTM CMU clocks must be enabled during enable path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin mode must be configured once during init");
}

void test_TC_G01_002_Init_Skips_Enable_When_EGTM_Already_Enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* Skip enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must NOT be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "EGTM CMU clocks must NOT be re-enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin mode must be configured once during init");
}

/***********************
 * GROUP 2 - init: configuration values
 ***********************/
void test_TC_G02_001_Init_Config_Sets_NumChannels_And_Frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxEgtm_Pwm_init_lastNumChannels, "init: numChannels must be 3 for 3-phase complementary PWM");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, (float)mock_IfxEgtm_Pwm_init_lastFrequency, "init: PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
}

void test_TC_G02_002_Init_Uses_TC4xx_APIs_Only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: positive verification on TC4xx API usage */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "TC4xx IfxEgtm_Pwm_initConfig must be used");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "TC4xx IfxEgtm_Pwm_init must be used");
    /* Also verify no unintended duty update during init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must not be called during init");
}

/***********************
 * GROUP 3 - init: runtime update logic (single/limited updates)
 ***********************/
void test_TC_G03_001_Single_Update_Increments_All_Duties(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "One update call should lead to one HAL duty update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should increment by step to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should increment by step to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should increment by step to 85%");
}

void test_TC_G03_002_Third_Update_Wraps_W_Only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;
    initEgtmAtom3phInv();

    /* Act: perform three updates */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Three updates should lead to three HAL duty updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty after 3 updates should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty after 3 updates should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should wrap to step value 10% after crossing 100%");
}

/***********************
 * GROUP 4 - init: ISR / interrupt behavior
 ***********************/
void test_TC_G04_001_ISR_Toggles_LED_Once_Per_Call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_G04_002_ISR_Toggle_Accumulates_Across_Calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "Two ISR calls must produce two toggles");
}

/***********************
 * GROUP 5 - update: configuration values (preconditions from init)
 ***********************/
void test_TC_G05_001_Init_Sets_Channel_Count_For_UpdatePath(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, (unsigned int)mock_IfxEgtm_Pwm_init_lastNumChannels, "update path: numChannels must be 3");
}

void test_TC_G05_002_Init_Sets_Frequency_For_UpdatePath(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, (float)mock_IfxEgtm_Pwm_init_lastFrequency, "update path: PWM frequency must be 20 kHz");
}

/***********************
 * GROUP 6 - update: runtime update logic
 ***********************/
void test_TC_G06_001_Multiple_Updates_Cause_Wrap_On_All_Channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_FXCLK0_ASSUMED_FREQUENCY_HZ;
    initEgtmAtom3phInv();

    /* Act: perform eight updates */
    for (int i = 0; i < 8; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert: expected after 8 steps → U=10, V=40, W=60 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates should lead to eight HAL duty updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty should wrap to 10% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty should be 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty should be 60% after 8 updates");
}

void test_TC_G06_002_Update_Called_Once_Per_ApplicationCall(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Unified API must be called once per duty update request (not per channel)");
}

/***********************
 * GROUP 7 - ISR: initialization / enable guard aspects
 ***********************/
void test_TC_G07_001_ISR_Toggle_Without_Reinit_After_Init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "ISR toggle must accumulate without requiring reinit");
}

void test_TC_G07_002_ISR_Does_Not_Invoke_Duty_Update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not update PWM duties");
}

/***********************
 * GROUP 8 - ISR: configuration values (no side effects beyond toggle)
 ***********************/
void test_TC_G08_001_PeriodCallback_Does_Not_Toggle_LED(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_togglePin_callCount, "Period callback must not toggle LED");
}

void test_TC_G08_002_ISR_Toggle_After_PeriodCallback(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "After period callback, ISR should toggle LED once");
}

/***********************
 * GROUP 9 - ISR: behavior verification
 ***********************/
void test_TC_G09_001_ISR_Multiple_Toggles_Accumulate(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (int i = 0; i < 5; ++i)
    {
        interruptEgtmAtom();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5, mock_togglePin_callCount, "Five ISR calls must produce five toggles");
}

void test_TC_G09_002_ISR_Does_Not_Trigger_PWM_Update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call duty update API");
}

/***********************
 * GROUP 10 - Period callback: configuration values
 ***********************/
void test_TC_G10_001_PeriodCallback_Exists_And_Accepts_Null(void)
{
    /* Arrange */
    /* No init required to verify linkage and side-effect-free behavior */

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must have no GPIO side effects");
}

void test_TC_G10_002_PeriodCallback_No_Toggle_On_Consecutive_Calls(void)
{
    /* Arrange */
    IfxEgtm_periodEventFunction(NULL);

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "Period callback must not toggle even on repeated calls");
}

/***********************
 * GROUP 11 - Period callback: ISR/interrupt behavior separation
 ***********************/
void test_TC_G11_001_PeriodCallback_Toggle_Is_Zero_After_Init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_callCount, "After init, period callback must not toggle LED");
}

void test_TC_G11_002_PeriodCallback_Does_Not_Trigger_Duty_Update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty();
    unsigned int before = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    unsigned int after = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, after, "Period callback must not call duty update API");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G01_001_Init_Calls_Target_APIs_When_EGTM_Disabled);
    RUN_TEST(test_TC_G01_002_Init_Skips_Enable_When_EGTM_Already_Enabled);

    RUN_TEST(test_TC_G02_001_Init_Config_Sets_NumChannels_And_Frequency);
    RUN_TEST(test_TC_G02_002_Init_Uses_TC4xx_APIs_Only);

    RUN_TEST(test_TC_G03_001_Single_Update_Increments_All_Duties);
    RUN_TEST(test_TC_G03_002_Third_Update_Wraps_W_Only);

    RUN_TEST(test_TC_G04_001_ISR_Toggles_LED_Once_Per_Call);
    RUN_TEST(test_TC_G04_002_ISR_Toggle_Accumulates_Across_Calls);

    RUN_TEST(test_TC_G05_001_Init_Sets_Channel_Count_For_UpdatePath);
    RUN_TEST(test_TC_G05_002_Init_Sets_Frequency_For_UpdatePath);

    RUN_TEST(test_TC_G06_001_Multiple_Updates_Cause_Wrap_On_All_Channels);
    RUN_TEST(test_TC_G06_002_Update_Called_Once_Per_ApplicationCall);

    RUN_TEST(test_TC_G07_001_ISR_Toggle_Without_Reinit_After_Init);
    RUN_TEST(test_TC_G07_002_ISR_Does_Not_Invoke_Duty_Update);

    RUN_TEST(test_TC_G08_001_PeriodCallback_Does_Not_Toggle_LED);
    RUN_TEST(test_TC_G08_002_ISR_Toggle_After_PeriodCallback);

    RUN_TEST(test_TC_G09_001_ISR_Multiple_Toggles_Accumulate);
    RUN_TEST(test_TC_G09_002_ISR_Does_Not_Trigger_PWM_Update);

    RUN_TEST(test_TC_G10_001_PeriodCallback_Exists_And_Accepts_Null);
    RUN_TEST(test_TC_G10_002_PeriodCallback_No_Toggle_On_Consecutive_Calls);

    RUN_TEST(test_TC_G11_001_PeriodCallback_Toggle_Is_Zero_After_Init);
    RUN_TEST(test_TC_G11_002_PeriodCallback_Does_Not_Trigger_Duty_Update);

    return UNITY_END();
}
