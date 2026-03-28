#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Externs for internal ISR and callback defined in production .c */
extern void interruptGtmAtom(void);
extern void IfxGtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQUENCY_HZ             (20000U)
#define UT_NUM_CHANNELS                 (6U)
#define UT_DEAD_TIME_NS_RISING          (1000U)
#define UT_DEAD_TIME_NS_FALLING         (1000U)
#define UT_INIT_U_PERCENT               (25.0f)
#define UT_INIT_V_PERCENT               (50.0f)
#define UT_INIT_W_PERCENT               (75.0f)
#define UT_STEP_PERCENT                 (10.0f)
#define UT_MODULE_CLOCK_HZ              (300000000U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ------------------------------
 * GROUP 1 - initGtmTom3phInv: initialization / enable guard
 * ------------------------------ */
void test_TC_G1_001_init_enables_and_configures_cmu_when_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_MODULE_CLOCK_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM enabled when previously disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks enabled once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured as output once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinLow_getCallCount(), "LED pin driven LOW once");
}

void test_TC_G1_002_init_skips_cmu_when_gtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency must NOT be read when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU clocks must NOT be enabled when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init called once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured as output once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinLow_getCallCount(), "LED pin driven LOW once");
}

/* ------------------------------
 * GROUP 2 - initGtmTom3phInv: configuration values
 * ------------------------------ */
void test_TC_G2_001_init_sets_20kHz_and_six_channels_in_driver_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* skip CMU path */

    /* Act */
    initGtmTom3phInv();

    /* Assert: verify application-configured values using init() spies (not initConfig) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels must be 6");
}

void test_TC_G2_002_init_calls_initConfig_and_init_once_each(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "init must be called exactly once");
}

/* ------------------------------
 * GROUP 3 - initGtmTom3phInv: runtime update logic (validate first updates after init)
 * ------------------------------ */
void test_TC_G3_001_first_update_increments_duty_below_wrap_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: U=35, V=60, W=85 -> duplicated to complementary pairs */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Ch0 duty should be 35% (U)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Ch1 duty should be 35% (U comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Ch2 duty should be 60% (V)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[3], "Ch3 duty should be 60% (V comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[4], "Ch4 duty should be 85% (W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[5], "Ch5 duty should be 85% (W comp)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once");
}

void test_TC_G3_002_three_updates_wrap_W_only_and_others_increment(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 3 updates -> U:55, V:80, W:10 (wrap) */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Ch0 duty should be 55% (U)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Ch1 duty should be 55% (U comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Ch2 duty should be 80% (V)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[3], "Ch3 duty should be 80% (V comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[4], "Ch4 duty should be 10% (W wrap to step)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[5], "Ch5 duty should be 10% (W comp wrap)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update should be called once per update (3 times)");
}

/* ------------------------------
 * GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior
 * ------------------------------ */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert: exactly one toggle function (IfxPort_togglePin or wrapper) called once */
    uint32 cntIfx = mock_IfxPort_togglePin_getCallCount();
    uint32 cntGen = mock_togglePin_getCallCount();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, (cntIfx + cntGen), "ISR should toggle LED exactly once");
}

void test_TC_G4_002_period_callback_is_empty_no_toggle(void)
{
    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert: no toggles happen from empty callback */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_togglePin_getCallCount(), "Period callback must not toggle LED via IfxPort_togglePin");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_togglePin_getCallCount(), "Period callback must not toggle LED via wrapper togglePin");
}

/* ------------------------------
 * GROUP 5 - updateGtmTom3phInvDuty: initialization / enable guard
 * ------------------------------ */
void test_TC_G5_001_update_called_once_when_gtm_initially_disabled_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* Force CMU path in init */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_MODULE_CLOCK_HZ;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once after init (GTM initially disabled)");
}

void test_TC_G5_002_update_called_once_when_gtm_initially_enabled_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* Skip CMU path */
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once after init (GTM initially enabled)");
}

/* ------------------------------
 * GROUP 6 - updateGtmTom3phInvDuty: configuration values
 * ------------------------------ */
void test_TC_G6_001_update_expands_phase_duties_to_complementary_pairs(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 2 updates -> U:45, V:70, W:95 */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Ch0 duty should be 45% (U)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Ch1 duty should be 45% (U comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Ch2 duty should be 70% (V)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[3], "Ch3 duty should be 70% (V comp)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[4], "Ch4 duty should be 95% (W)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[5], "Ch5 duty should be 95% (W comp)");
}

void test_TC_G6_002_update_does_not_change_pwm_frequency_or_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert: initialization parameters remain unchanged */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM frequency must remain 20 kHz after updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels must remain 6 after updates");
}

/* ------------------------------
 * GROUP 7 - updateGtmTom3phInvDuty: runtime update logic
 * ------------------------------ */
void test_TC_G7_001_wrap_rule_applies_at_boundary_per_phase(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 3 updates -> W wraps to 10, U/V below boundary */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 55% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "V should be 80% after 3 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[4], "W should wrap to 10% after reaching/exceeding 100%");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once per update (3 times)");
}

void test_TC_G7_002_eight_updates_result_in_expected_wrapped_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    initGtmTom3phInv();

    /* Act: 8 updates */
    for (int i = 0; i < 8; ++i)
    {
        updateGtmTom3phInvDuty();
    }

    /* Expected after 8 steps (step=10):
     * U: 25 -> 10 (wrap on 8th) ; V: 50 -> 40 ; W: 75 -> 60
     * Duplicated to complementary pairs.
     */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U should be 10% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "U comp should be 10% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "V should be 40% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[3], "V comp should be 40% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[4], "W should be 60% after 8 steps");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[5], "W comp should be 60% after 8 steps");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Immediate duty update must be called once per update (8 times)");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_cmu_when_gtm_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_gtm_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_20kHz_and_six_channels_in_driver_init);
    RUN_TEST(test_TC_G2_002_init_calls_initConfig_and_init_once_each);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_duty_below_wrap_boundary);
    RUN_TEST(test_TC_G3_002_three_updates_wrap_W_only_and_others_increment);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_period_callback_is_empty_no_toggle);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_called_once_when_gtm_initially_disabled_after_init);
    RUN_TEST(test_TC_G5_002_update_called_once_when_gtm_initially_enabled_after_init);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_update_expands_phase_duties_to_complementary_pairs);
    RUN_TEST(test_TC_G6_002_update_does_not_change_pwm_frequency_or_channel_count);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_wrap_rule_applies_at_boundary_per_phase);
    RUN_TEST(test_TC_G7_002_eight_updates_result_in_expected_wrapped_duties);

    return UNITY_END();
}
