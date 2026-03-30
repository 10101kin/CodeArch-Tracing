#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Externs provided by the mock layer */
extern unsigned int mock_IfxGtm_Pwm_init_callCount;
extern unsigned int mock_IfxGtm_Pwm_initConfig_callCount;
extern unsigned int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern unsigned int mock_IfxGtm_isEnabled_callCount;
extern unsigned int mock_IfxGtm_enable_callCount;
extern unsigned int mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern unsigned int mock_IfxGtm_Cmu_enableClocks_callCount;
extern unsigned int mock_IfxPort_togglePin_callCount;
extern unsigned int mock_togglePin_callCount;
extern unsigned int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxGtm_isEnabled_returnValue;
extern unsigned int mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
extern unsigned int mock_IfxGtm_Pwm_init_lastNumChannels;
extern float mock_IfxGtm_Pwm_init_lastFrequency;
extern unsigned int mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float mock_IfxGtm_Pwm_initConfig_lastFrequency;
extern float mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern float mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[];
extern float mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[];

/* ISR defined in production .c (IFX_INTERRUPT) */
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3u)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_STEP_PERCENT                  (10.0f)
#define UT_MAX_DUTY_PERCENT              (100.0f)
#define UT_DEADTIME_NS_RISING            (1000.0f)
#define UT_DEADTIME_NS_FALLING           (1000.0f)
#define UT_GTM_GCLK_HZ                   (100000000u)
#define UT_GTM_FXCLK0_HZ                 (100000000u)

void setUp(void) {
    mock_gtm_tom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/***********************
 * GROUP 1 - init guard
 ***********************/
void test_TC_G1_001_init_guard_already_enabled(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable check must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must NOT be re-enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must NOT be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks must NOT be enabled when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug/LED GPIO configured as output exactly once");
}

void test_TC_G1_002_init_guard_was_disabled(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled -> guard path */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable check must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be read during enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks must be enabled during guard");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug/LED GPIO configured as output exactly once");
}

/*********************************
 * GROUP 2 - init configuration
 *********************************/
void test_TC_G2_001_init_config_frequency_and_channels(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* skip guard actions */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert: app-configured values must be visible on init() spy */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "There must be 3 logical channels (U,V,W)");
}

void test_TC_G2_002_init_no_duty_update_during_init(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert: no immediate duty update inside init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must run exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must run exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No duty update API call should occur during init");
}

/*************************************
 * GROUP 3 - ISR / interrupt behavior
 *************************************/
void test_TC_G3_001_isr_toggle_once(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED exactly once");
}

void test_TC_G3_002_isr_toggle_accumulates(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "ISR toggle count must accumulate across calls");
}

/*********************************************
 * GROUP 4 - update: configuration/units check
 *********************************************/
void test_TC_G4_001_update_after_init_first_step(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: percent duties after one step */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per update invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must step by +10% to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must step by +10% to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must step by +10% to 85%");
}

void test_TC_G4_002_update_duty_units_are_percent_not_fraction(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: ensure units are 0..100 percent (not 0.0..1.0) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 35.0 percent, not 0.35 fraction");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 60.0 percent, not 0.60 fraction");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must be 85.0 percent, not 0.85 fraction");
}

/****************************************
 * GROUP 5 - update runtime wrap logic
 ****************************************/
void test_TC_G5_001_update_wrap_behavior_independent_channels(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act: call 8 times to force different wrap points for U/V/W */
    for (int i = 0; i < 8; ++i) {
        updateGtmTom3phInvDuty();
    }

    /* After 8 steps:
       U: 25 -> wrap at step 8 -> 10
       V: 50 -> wrap at step 5 -> then +3 steps -> 40
       W: 75 -> wrap at step 3 -> then +5 steps -> 60
    */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per update invocation (8 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must wrap to 10% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must wrap then reach 40% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap then reach 60% after 8 steps");
}

void test_TC_G5_002_update_three_steps_with_wrap_on_W_only(void) {
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_GCLK_HZ;
    initGtmTom3phInv();

    /* Act: three sequential updates */
    updateGtmTom3phInvDuty(); /* U:35 V:60 W:85 */
    updateGtmTom3phInvDuty(); /* U:45 V:70 W:95 */
    updateGtmTom3phInvDuty(); /* U:55 V:80 W:10 (wrap) */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must be called once per update invocation (3 calls)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U must be 55% after three steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V must be 80% after three steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W must wrap to 10% on third step");
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_guard_already_enabled);
    RUN_TEST(test_TC_G1_002_init_guard_was_disabled);

    RUN_TEST(test_TC_G2_001_init_config_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_init_no_duty_update_during_init);

    RUN_TEST(test_TC_G3_001_isr_toggle_once);
    RUN_TEST(test_TC_G3_002_isr_toggle_accumulates);

    RUN_TEST(test_TC_G4_001_update_after_init_first_step);
    RUN_TEST(test_TC_G4_002_update_duty_units_are_percent_not_fraction);

    RUN_TEST(test_TC_G5_001_update_wrap_behavior_independent_channels);
    RUN_TEST(test_TC_G5_002_update_three_steps_with_wrap_on_W_only);

    return UNITY_END();
}
