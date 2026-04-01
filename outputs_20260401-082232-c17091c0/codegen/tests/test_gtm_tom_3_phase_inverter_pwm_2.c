#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm_2.h"
#include "gtm_tom_3_phase_inverter_pwm_2.h"

/* Extern ISR from production file (defined with IFX_INTERRUPT) */
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON              (1e-4f)
#define UT_NUM_CHANNELS               (3)
#define UT_PWM_FREQ_HZ                (20000.0f)
#define UT_DEADTIME_S                 (5e-07f)
#define UT_INIT_DUTY_U_PERCENT        (25.0f)
#define UT_INIT_DUTY_V_PERCENT        (50.0f)
#define UT_INIT_DUTY_W_PERCENT        (75.0f)
#define UT_STEP_PERCENT               (10.0f)
#define UT_ISR_PRIORITY               (20)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_2_reset(); }
void tearDown(void) {}

/* ------------------------------
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 * ------------------------------ */
void test_TC_G1_001_enable_guard_gtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM must NOT be re-enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU functional clocks must be enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Diagnostic/toggle GPIO must be configured as output once");
}

void test_TC_G1_002_enable_guard_gtm_disabled_enables_and_clocks(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE; /* GTM disabled -> should enable */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable status must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU functional clocks must be enabled once after GTM enable");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must be called once");
}

/* ------------------------------
 * GROUP 2 - initGtmTom3phInv: configuration values
 * ------------------------------ */
void test_TC_G2_001_init_config_frequency_and_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert: verify application-configured values captured by init() spies */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical PWM channels must be 3 for 3-phase");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_config_gpio_output_mode_and_no_toggle(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert: LED pin configured as output; no ISR toggles should occur during init */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as push-pull output during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "LED must NOT toggle during initialization");
}

/* ------------------------------
 * GROUP 3 - runtime update logic (percent-based, step=10, wrap when >=100)
 * ------------------------------ */
void test_TC_G3_001_update_increments_duty_no_wrap(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: single update */
    updateGtmTom3phInvDuty();

    /* Assert: exactly one duty update occurred (immediate or synced), and values increased by step */
    unsigned int totalDutyUpdateCalls = (unsigned int)(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount() +
                                                      mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount());
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, totalDutyUpdateCalls, "Exactly one PWM duty update call expected for one logical update");

    /* Select correct spy buffer depending on which API was used */
    float *duties = (mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount() > 0)
                      ? mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties
                      : mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties;

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), duties[0], "Phase U duty must increment by step to 35.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), duties[1], "Phase V duty must increment by step to 60.0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), duties[2], "Phase W duty must increment by step to 85.0%");
}

void test_TC_G3_002_update_wrap_around_independent(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act: perform 8 updates to exercise wrap logic across channels */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Assert: exactly 8 updates via HAL (immediate or synced) */
    unsigned int totalDutyUpdateCalls = (unsigned int)(mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount() +
                                                      mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount());
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, totalDutyUpdateCalls, "PWM duty update must be called once per update invocation (8 total)");

    float *duties = (mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount() > 0)
                      ? mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties
                      : mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties;

    /* After 8 updates: U=10, V=40, W=60 with wrap rule (>=100 -> 0 then +step) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, duties[0], "Phase U must wrap to 10.0% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, duties[1], "Phase V must wrap independently to 40.0% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, duties[2], "Phase W must wrap independently to 60.0% after 8 updates");
}

/* ------------------------------
 * GROUP 4 - ISR / interrupt behavior (LED toggles on each period ISR)
 * ------------------------------ */
void test_TC_G4_001_isr_single_toggle(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_multiple_toggles_cumulative(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxPort_togglePin_getCallCount(), "LED toggle count must accumulate across ISR calls (2 toggles)");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_enable_guard_gtm_already_enabled);
    RUN_TEST(test_TC_G1_002_enable_guard_gtm_disabled_enables_and_clocks);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_config_frequency_and_channels);
    RUN_TEST(test_TC_G2_002_init_config_gpio_output_mode_and_no_toggle);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_update_increments_duty_no_wrap);
    RUN_TEST(test_TC_G3_002_update_wrap_around_independent);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_single_toggle);
    RUN_TEST(test_TC_G4_002_isr_multiple_toggles_cumulative);

    return UNITY_END();
}
