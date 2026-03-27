#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Period-event callback provided by production module */
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQ_HZ                   (20000.0f)
#define UT_INIT_DUTY_U                   (0.25f)
#define UT_INIT_DUTY_V                   (0.50f)
#define UT_INIT_DUTY_W                   (0.75f)
#define UT_DEADTIME_RISING_S             (1.0e-6f)
#define UT_DEADTIME_FALLING_S            (1.0e-6f)
#define UT_EXPECTED_NUM_CHANNELS         (3u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* Helper to compute a positive modular delta in [0,1) for duty changes */
static float duty_delta_mod1(float prev, float curr)
{
    float d = curr - prev;
    if (d < 0.0f) d += 1.0f;
    return d;
}

/**********************
 * GROUP 1 - init: enable guard and target API usage
 **********************/
void test_TC_G1_001_init_enables_clocks_when_module_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 0; /* peripheral disabled */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 100000000.0f; /* 100 MHz */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - TC4xx API usage and enable path */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider must be configured when enabling eGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK divider must be configured when enabling eGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled when eGTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "Status GPIO must be configured once after PWM init");
}

void test_TC_G1_002_init_skips_clock_setup_when_module_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1; /* already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - CMU/enable path must be skipped */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must NOT be read when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK divider config must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clock enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must still be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "Status GPIO must be configured once after PWM init");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_config_has_expected_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1; /* skip enable path to focus on config */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - PWM frequency is the switching frequency, not module clock */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM init frequency must be 20 kHz");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxEgtm_Pwm_initConfig_lastFrequency, "PWM initConfig frequency must be 20 kHz");

    /* Assert - channel count */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_EXPECTED_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "PWM init must configure 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_EXPECTED_NUM_CHANNELS, mock_IfxEgtm_Pwm_initConfig_lastNumChannels, "PWM initConfig must specify 3 channels");
}

void test_TC_G2_002_init_sets_initial_duties_and_deadtime(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - initial duty cycles (from configuration for U/V/W) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Initial U duty must be 25%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Initial V duty must be 50%%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Initial W duty must be 75%%");

    /* Assert - dead-time values (rising/falling) for all channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_S,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[0], "DT rising ch0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_S,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[1], "DT rising ch1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_RISING_S,  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[2], "DT rising ch2 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[0], "DT falling ch0 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[1], "DT falling ch1 must be 1.0 us");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEADTIME_FALLING_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[2], "DT falling ch2 must be 1.0 us");
}

/**********************
 * GROUP 3 - init: runtime update logic checks (post-init)
 **********************/
void test_TC_G3_001_first_update_increments_duty_modulo_one(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert - each channel must advance by a positive modular delta */
    float dU = duty_delta_mod1(UT_INIT_DUTY_U, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    float dV = duty_delta_mod1(UT_INIT_DUTY_V, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1]);
    float dW = duty_delta_mod1(UT_INIT_DUTY_W, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]);
    TEST_ASSERT_MESSAGE(dU > 0.0f, "U duty must increase on first update (mod 1)");
    TEST_ASSERT_MESSAGE(dV > 0.0f, "V duty must increase on first update (mod 1)");
    TEST_ASSERT_MESSAGE(dW > 0.0f, "W duty must increase on first update (mod 1)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once for the update");
}

void test_TC_G3_002_multiple_updates_produce_wrap_for_all_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Drive multiple updates until each channel shows a wrap (curr < prev) */
    float prev[3] = {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]
    };
    int wrapped[3] = {0, 0, 0};

    /* Act */
    const int maxSteps = 200; /* ample headroom for typical step sizes */
    int steps = 0;
    while ((!(wrapped[0] && wrapped[1] && wrapped[2])) && (steps < maxSteps))
    {
        updateEgtmAtom3phInvDuty();
        float currU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
        float currV = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
        float currW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        if (!wrapped[0] && (currU < prev[0])) wrapped[0] = 1;
        if (!wrapped[1] && (currV < prev[1])) wrapped[1] = 1;
        if (!wrapped[2] && (currW < prev[2])) wrapped[2] = 1;
        prev[0] = currU; prev[1] = currV; prev[2] = currW;
        steps++;
    }

    /* Assert */
    TEST_ASSERT_MESSAGE(wrapped[0], "U channel must wrap within a reasonable number of updates");
    TEST_ASSERT_MESSAGE(wrapped[1], "V channel must wrap within a reasonable number of updates");
    TEST_ASSERT_MESSAGE(wrapped[2], "W channel must wrap within a reasonable number of updates");
}

/**********************
 * GROUP 4 - ISR / interrupt behavior
 **********************/
void test_TC_G4_001_period_event_isr_toggles_once(void)
{
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1u, mock_togglePin_callCount, "ISR must toggle exactly once per invocation");
}

void test_TC_G4_002_period_event_isr_toggle_accumulates(void)
{
    /* Arrange */
    uint32_t before = mock_togglePin_callCount;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3u, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

/**********************
 * GROUP 5 - update: enable guard interaction (init path variants)
 **********************/
void test_TC_G5_001_update_calls_hal_once_after_init_enabled_true(void)
{
    /* Arrange: init with already-enabled path */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "No HAL duty update should have happened yet in this path");

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL duty update expected after one call");
}

void test_TC_G5_002_update_calls_hal_once_after_init_enabled_false(void)
{
    /* Arrange: init with enable-and-clock path */
    mock_IfxEgtm_isEnabled_returnValue = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 100000000.0f;
    initEgtmAtom3phInv();

    /* Sanity: enable + CMU should have been configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called on disabled path");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled on disabled path");

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Exactly one HAL duty update expected after one call");
}

/**********************
 * GROUP 6 - update: configuration-related behaviors
 **********************/
void test_TC_G6_001_update_applies_equal_step_across_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act: two successive updates */
    updateEgtmAtom3phInvDuty();
    float u1 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float v1 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float w1 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    updateEgtmAtom3phInvDuty();
    float u2 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float v2 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float w2 = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Compute modular deltas between the two updates for each channel */
    float dU = duty_delta_mod1(u1, u2);
    float dV = duty_delta_mod1(v1, v2);
    float dW = duty_delta_mod1(w1, w2);

    /* Assert - equal step across all channels and positive */
    TEST_ASSERT_MESSAGE(dU > 0.0f, "U step must be positive");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, dU, dV, "All channels must advance with the same step (V vs U)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, dU, dW, "All channels must advance with the same step (W vs U)");
}

void test_TC_G6_002_update_keeps_duties_in_valid_range(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act: perform several updates */
    const int N = 25;
    for (int i = 0; i < N; ++i)
    {
        updateEgtmAtom3phInvDuty();
        float u = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
        float v = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
        float w = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        TEST_ASSERT_MESSAGE(u >= 0.0f && u < 1.0f, "U duty must remain in [0,1)");
        TEST_ASSERT_MESSAGE(v >= 0.0f && v < 1.0f, "V duty must remain in [0,1)");
        TEST_ASSERT_MESSAGE(w >= 0.0f && w < 1.0f, "W duty must remain in [0,1)");
    }
}

/**********************
 * GROUP 7 - update: runtime update logic
 **********************/
void test_TC_G7_001_hal_update_called_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    uint32_t before = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();
    const int N = 10;
    for (int i = 0; i < N; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }
    uint32_t after = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + (uint32_t)N, after, "HAL duty update must be called exactly once per update invocation");
}

void test_TC_G7_002_each_channel_wraps_independently_within_reasonable_steps(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    float prev[3] = {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]
    };
    int wrapped[3] = {0, 0, 0};

    /* Act */
    const int maxSteps = 300; /* extra headroom */
    for (int i = 0; i < maxSteps && (!(wrapped[0] && wrapped[1] && wrapped[2])); ++i)
    {
        updateEgtmAtom3phInvDuty();
        float currU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
        float currV = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
        float currW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        if (!wrapped[0] && (currU < prev[0])) wrapped[0] = 1;
        if (!wrapped[1] && (currV < prev[1])) wrapped[1] = 1;
        if (!wrapped[2] && (currW < prev[2])) wrapped[2] = 1;
        prev[0] = currU; prev[1] = currV; prev[2] = currW;
    }

    /* Assert */
    TEST_ASSERT_MESSAGE(wrapped[0], "U channel must wrap independently");
    TEST_ASSERT_MESSAGE(wrapped[1], "V channel must wrap independently");
    TEST_ASSERT_MESSAGE(wrapped[2], "W channel must wrap independently");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_clocks_when_module_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_clock_setup_when_module_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_config_has_expected_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_sets_initial_duties_and_deadtime);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_duty_modulo_one);
    RUN_TEST(test_TC_G3_002_multiple_updates_produce_wrap_for_all_channels);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_period_event_isr_toggles_once);
    RUN_TEST(test_TC_G4_002_period_event_isr_toggle_accumulates);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_after_init_enabled_true);
    RUN_TEST(test_TC_G5_002_update_calls_hal_once_after_init_enabled_false);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_update_applies_equal_step_across_channels);
    RUN_TEST(test_TC_G6_002_update_keeps_duties_in_valid_range);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_hal_update_called_once_per_call);
    RUN_TEST(test_TC_G7_002_each_channel_wraps_independently_within_reasonable_steps);

    return UNITY_END();
}
