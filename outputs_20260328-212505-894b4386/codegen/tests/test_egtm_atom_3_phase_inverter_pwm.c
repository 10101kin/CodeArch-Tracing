#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                  (1e-4f)
#define UT_NUM_CHANNELS                   (3)
#define UT_PWM_FREQUENCY_HZ               (20000U)
#define UT_INIT_DUTY_U_PERCENT            (25.0f)
#define UT_INIT_DUTY_V_PERCENT            (50.0f)
#define UT_INIT_DUTY_W_PERCENT            (75.0f)
#define UT_DEADTIME_RISING_S              (1e-6f)
#define UT_DEADTIME_FALLING_S             (1e-6f)
#define UT_EGTM_CMU_GCLK_HZ               (100000000U)

/* ISR extern (defined in production .c with IFX_INTERRUPT) */
extern void interruptEgtmAtom(void);

/* Helpers */
static float circularDeltaPercent(float prev, float curr)
{
    float d = curr - prev;
    if (d < 0.0f)
    {
        d += 100.0f;
    }
    return d;
}

void setUp(void)
{
    mock_egtm_atom_3_phase_inverter_pwm_reset();
}

void tearDown(void) {}

/*******************************
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 *******************************/
void test_TC_G1_001_init_calls_target_tc4xx_apis_when_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1; /* already enabled: no CMU setup */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency should not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider should not be set when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK divider should not be set when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks should not be re-enabled when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug LED must be configured as output once");
}

void test_TC_G1_002_init_enables_module_and_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 0; /* disabled: expect enable + CMU setup */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_CMU_GCLK_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called inside guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider must be configured 1:1");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(), "ECLK divider must be configured as required (e.g., 1:1)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled inside guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug LED must be configured as output once");
}

/*******************************
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 *******************************/
void test_TC_G2_001_init_applies_config_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: verify application-configured values via init() spies (not initConfig) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 (U,V,W)");
}

void test_TC_G2_002_init_does_not_call_runtime_update_apis(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Runtime update API must not be called during init");
}

/*******************************
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic (duty ramp semantics verified after init)
 *******************************/
void test_TC_G3_001_first_update_increments_all_phases_by_same_step(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    float sU = circularDeltaPercent(UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    float sV = circularDeltaPercent(UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1]);
    float sW = circularDeltaPercent(UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sV, "All phases must advance by the same duty step on first update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sW, "All phases must advance by the same duty step on first update");
    TEST_ASSERT_TRUE_MESSAGE(sU > 0.0f && sU <= 100.0f, "Duty step must be within (0,100]");
}

void test_TC_G3_002_wrap_occurs_independently_per_phase_w_wrap_on_W_first(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Measure step S from first update */
    updateEgtmAtom3phInvDuty();
    float S = circularDeltaPercent(UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    TEST_ASSERT_TRUE_MESSAGE(S > 0.0f, "Measured duty step S must be > 0");

    /* Compute kW = ceil((100-75)/S) updates to cause W wrap from initial */
    float remainingW = 100.0f - UT_INIT_DUTY_W_PERCENT; /* 25 */
    unsigned kW = (unsigned)((remainingW + S - 1.0f) / S); /* ceil */
    if (kW == 0) { kW = 1; }

    /* We already performed 1 update. Perform (kW - 1) more to reach total kW updates */
    /* Capture previous values immediately before the final (wrap) update */
    float prevU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float prevW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    for (unsigned i = 1; i < kW; ++i)
    {
        if (i == (kW - 1))
        {
            prevU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
            prevW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        }
        updateEgtmAtom3phInvDuty();
    }

    /* After total kW updates, W should have wrapped: newW < prevW and equals S */
    float newU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float newW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    TEST_ASSERT_TRUE_MESSAGE(newW < prevW, "W phase must wrap at its threshold (duty decreases across wrap)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, S, newW, "After wrap, W duty must equal the step (not zero)");

    /* U must not wrap at the same time (should still be increasing) */
    TEST_ASSERT_TRUE_MESSAGE(newU > prevU, "U phase should not wrap when W wraps (independent wrap behavior)");
}

/*******************************
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior
 *******************************/
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();

    /* Assert: use mock_togglePin_callCount (preferred single counter) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "ISR toggle must accumulate across multiple calls");
}

/*******************************
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration values via spies
 *******************************/
void test_TC_G5_001_update_calls_hal_once_and_updates_three_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once");

    float sU = circularDeltaPercent(UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    float sV = circularDeltaPercent(UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1]);
    float sW = circularDeltaPercent(UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sV, "All phases must advance by the same step (config: 3 logical channels)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sW, "All phases must advance by the same step (config: 3 logical channels)");
}

void test_TC_G5_002_duties_remain_within_0_100_after_many_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    const unsigned loops = 100;
    for (unsigned i = 0; i < loops; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 100.0f, "U duty must stay within [0,100]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 100.0f, "V duty must stay within [0,100]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 100.0f, "W duty must stay within [0,100]");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(loops, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called once per update loop iteration");
}

/*******************************
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic
 *******************************/
void test_TC_G6_001_update_called_multiple_times_results_in_equal_hal_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Act */
    const unsigned nUpdates = 10;
    for (unsigned i = 0; i < nUpdates; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(nUpdates, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL must be called exactly once per update() call");

    /* Verify constant step (circular) across last two updates */
    /* Perform one more update to compute step from consecutive readings */
    float prevU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float prevV = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float prevW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
    updateEgtmAtom3phInvDuty();
    float sU = circularDeltaPercent(prevU, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    float sV = circularDeltaPercent(prevV, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1]);
    float sW = circularDeltaPercent(prevW, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sV, "Consecutive updates must maintain equal step across phases");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, sU, sW, "Consecutive updates must maintain equal step across phases");
}

void test_TC_G6_002_after_wrap_post_step_value_equals_step_for_each_phase(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    initEgtmAtom3phInv();

    /* Determine step S from first update against initial U */
    updateEgtmAtom3phInvDuty();
    float stepS = circularDeltaPercent(UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]);
    TEST_ASSERT_TRUE_MESSAGE(stepS > 0.0f, "Measured duty step must be > 0");

    /* Now iterate until each channel wraps at least once, asserting post-wrap equals step */
    float prev[3] = {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1],
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]
    };
    bool wrappedSeen[3] = { false, false, false };

    /* Upper bound iterations to ensure coverage even for small steps */
    for (unsigned iter = 0; iter < 2000 && (!wrappedSeen[0] || !wrappedSeen[1] || !wrappedSeen[2]); ++iter)
    {
        updateEgtmAtom3phInvDuty();
        float curr[3] = {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0],
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1],
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]
        };
        for (int i = 0; i < 3; ++i)
        {
            if (!wrappedSeen[i] && (curr[i] < prev[i]))
            {
                /* Wrap detected: after wrap, duty must equal step */
                TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, stepS, curr[i], "After wrap, duty must equal step (not zero)");
                wrappedSeen[i] = true;
            }
            prev[i] = curr[i];
        }
    }

    TEST_ASSERT_TRUE_MESSAGE(wrappedSeen[0], "U must wrap at least once within bounded iterations");
    TEST_ASSERT_TRUE_MESSAGE(wrappedSeen[1], "V must wrap at least once within bounded iterations");
    TEST_ASSERT_TRUE_MESSAGE(wrappedSeen[2], "W must wrap at least once within bounded iterations");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_init_calls_target_tc4xx_apis_when_enabled);
    RUN_TEST(test_TC_G1_002_init_enables_module_and_cmu_when_disabled);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_init_applies_config_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_init_does_not_call_runtime_update_apis);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_first_update_increments_all_phases_by_same_step);
    RUN_TEST(test_TC_G3_002_wrap_occurs_independently_per_phase_w_wrap_on_W_first);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_update_calls_hal_once_and_updates_three_channels);
    RUN_TEST(test_TC_G5_002_duties_remain_within_0_100_after_many_updates);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_update_called_multiple_times_results_in_equal_hal_calls);
    RUN_TEST(test_TC_G6_002_after_wrap_post_step_value_equals_step_for_each_phase);

    return UNITY_END();
}
