#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_PHASES                    (3U)
#define UT_COMPLEMENTARY_PAIRS           (3U)
#define UT_PWM_FREQUENCY_HZ              (20000U)
#define UT_EGTM_EXPECTED_FXCLK0_HZ       (200000000U)
#define UT_DEAD_TIME_US                  (0.5f)
#define UT_MIN_PULSE_US                  (1.0f)
#define UT_INITIAL_DUTY_U                (0.25f)
#define UT_INITIAL_DUTY_V                (0.50f)
#define UT_INITIAL_DUTY_W                (0.75f)
#define UT_MIN_DUTY_FRAC                 (0.02f)   /* 1.0 us / 50.0 us at 20 kHz */
#define UT_MAX_DUTY_FRAC                 (0.98f)   /* Reserve margin below 100% */

/* Optional ISR symbol expected in production per target naming guidance */
extern void interruptEgtmAtom(void);

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ============================= GROUP 1: initEgtmAtom3phInv - initialization / enable guard ============================= */
void test_TC_01_001_init_enables_egtm_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 0; /* eGTM disabled */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called exactly once during init when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency must be read once inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider must be configured (1:1) when enabling eGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_selectClkInput_getCallCount(), "Clk_0 must be selected from GCLK during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Fxclk/Clk0 clocks must be enabled during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
}

void test_TC_01_002_init_skips_enable_and_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1; /* eGTM already enabled */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be checked exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(), "GCLK divider configuration must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_selectClkInput_getCallCount(), "Clk_0 selection must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "Clock enables must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must still be called once");
}

/* ============================= GROUP 2: initEgtmAtom3phInv - configuration values ============================= */
void test_TC_02_001_init_config_sets_frequency_and_num_channels_in_initConfig(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_PHASES, mock_IfxEgtm_Pwm_initConfig_lastNumChannels, "initConfig must set 3 logical channels for U/V/W");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_initConfig_lastFrequency, "initConfig must set PWM frequency to 20 kHz");
}

void test_TC_02_002_init_config_sets_frequency_and_num_channels_in_init_call_and_led_gpio_configured(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_PHASES, mock_IfxEgtm_Pwm_init_lastNumChannels, "IfxEgtm_Pwm_init must receive 3 channels");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "IfxEgtm_Pwm_init must receive 20 kHz frequency");
    TEST_ASSERT_TRUE_MESSAGE((mock_IfxPort_setPinModeOutput_getCallCount() >= 1U), "Diagnostic LED GPIO should be configured as output at least once after PWM init");
}

/* ============================= GROUP 3: initEgtmAtom3phInv - runtime update logic (post-init readiness) ============================= */
void test_TC_03_001_after_init_first_update_applies_three_duties_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call HAL exactly once per invocation");
    /* Verify duty array is populated with 3 entries within [0,1] */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 1.0f, "Phase U duty must be within [0,1]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 1.0f, "Phase V duty must be within [0,1]");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 1.0f, "Phase W duty must be within [0,1]");
}

void test_TC_03_002_repeated_updates_eventually_wrap_a_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    /* Run multiple updates and detect a wrap (duty decreases vs previous due to wrap to min fraction) */
    float prevW = -1.0f;
    bool wrapObserved = false;
    for (int i = 0; i < 500; ++i)
    {
        updateEgtmAtom3phInvDuty();
        float w = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        if (prevW >= 0.0f && w + UT_FLOAT_EPSILON < prevW)
        {
            wrapObserved = true;
            break;
        }
        prevW = w;
    }

    /* Assert */
    TEST_ASSERT_TRUE_MESSAGE(wrapObserved, "At least one channel (W) must eventually wrap to the min duty fraction after repeated updates");
}

/* ============================= GROUP 4: initEgtmAtom3phInv - ISR / interrupt behavior ============================= */
void test_TC_04_001_isr_toggle_increments_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();
    uint32 before = mock_IfxPort_togglePin_getCallCount();

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_IfxPort_togglePin_getCallCount(), "ISR must toggle diagnostic GPIO exactly once per invocation");
}

void test_TC_04_002_isr_toggle_accumulates_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();
    uint32 before = mock_IfxPort_togglePin_getCallCount();

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3U, mock_IfxPort_togglePin_getCallCount(), "ISR toggling must accumulate across multiple invocations");
}

/* ============================= GROUP 5: updateEgtmAtom3phInvDuty - initialization / enable guard ============================= */
void test_TC_05_001_update_calls_hal_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1; /* Regardless of enabled state, update always calls HAL after init */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once per update invocation");
}

void test_TC_05_002_update_following_disabled_init_still_updates_hal(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 0; /* Force full enable/CMU path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;

    /* Act */
    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "Driver must be initialized once before updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Update must call HAL once even when init performed full enable path");
}

/* ============================= GROUP 6: updateEgtmAtom3phInvDuty - configuration values ============================= */
void test_TC_06_001_update_duties_respect_min_fraction_bounds(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    float du = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float dv = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float dw = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
    TEST_ASSERT_TRUE_MESSAGE(du >= UT_MIN_DUTY_FRAC - UT_FLOAT_EPSILON && du <= UT_MAX_DUTY_FRAC + UT_FLOAT_EPSILON, "Phase U duty must respect min/max fraction bounds after update");
    TEST_ASSERT_TRUE_MESSAGE(dv >= UT_MIN_DUTY_FRAC - UT_FLOAT_EPSILON && dv <= UT_MAX_DUTY_FRAC + UT_FLOAT_EPSILON, "Phase V duty must respect min/max fraction bounds after update");
    TEST_ASSERT_TRUE_MESSAGE(dw >= UT_MIN_DUTY_FRAC - UT_FLOAT_EPSILON && dw <= UT_MAX_DUTY_FRAC + UT_FLOAT_EPSILON, "Phase W duty must respect min/max fraction bounds after update");
}

void test_TC_06_002_pwm_frequency_remains_configured_after_updates(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must remain 20 kHz after runtime updates");
}

/* ============================= GROUP 7: updateEgtmAtom3phInvDuty - runtime update logic ============================= */
void test_TC_07_001_duty_increments_from_initial_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    float du = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float dv = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float dw = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    /* Assert: U/V must have increased above their initial fractions; W may wrap if step is large, so guard with range */
    TEST_ASSERT_TRUE_MESSAGE(du > (UT_INITIAL_DUTY_U + UT_FLOAT_EPSILON) || du < (UT_MIN_DUTY_FRAC + 0.1f), "Phase U duty should increase from initial value, unless it wrapped to near minimum");
    TEST_ASSERT_TRUE_MESSAGE(dv > (UT_INITIAL_DUTY_V + UT_FLOAT_EPSILON) || dv < (UT_MIN_DUTY_FRAC + 0.1f), "Phase V duty should increase from initial value, unless it wrapped to near minimum");
    TEST_ASSERT_TRUE_MESSAGE(dw >= 0.0f && dw <= 1.0f, "Phase W duty must be a valid fraction after first update");
}

void test_TC_07_002_each_channel_wraps_independently_and_hal_called_once_per_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_EGTM_EXPECTED_FXCLK0_HZ;
    initEgtmAtom3phInv();

    /* Act: run multiple updates, track the first wrap index for each channel */
    int wrapIndexU = -1, wrapIndexV = -1, wrapIndexW = -1;
    float prevU = -1.0f, prevV = -1.0f, prevW = -1.0f;
    const int N = 300;
    for (int i = 0; i < N; ++i)
    {
        updateEgtmAtom3phInvDuty();
        float u = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
        float v = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
        float w = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];
        if (prevU >= 0.0f && wrapIndexU < 0 && (u + UT_FLOAT_EPSILON) < prevU) wrapIndexU = i;
        if (prevV >= 0.0f && wrapIndexV < 0 && (v + UT_FLOAT_EPSILON) < prevV) wrapIndexV = i;
        if (prevW >= 0.0f && wrapIndexW < 0 && (w + UT_FLOAT_EPSILON) < prevW) wrapIndexW = i;
        prevU = u; prevV = v; prevW = w;
    }

    /* Assert: HAL called exactly once per update, and at least two channels wrap at different iterations */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE((uint32)N, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL duty update must be called exactly once for each update invocation");

    /* It's expected that U/V/W are phase-shifted and therefore wrap at different times */
    bool atLeastTwoDifferent = false;
    if ((wrapIndexU >= 0 && wrapIndexV >= 0 && wrapIndexU != wrapIndexV) ||
        (wrapIndexU >= 0 && wrapIndexW >= 0 && wrapIndexU != wrapIndexW) ||
        (wrapIndexV >= 0 && wrapIndexW >= 0 && wrapIndexV != wrapIndexW))
    {
        atLeastTwoDifferent = true;
    }
    TEST_ASSERT_TRUE_MESSAGE(atLeastTwoDifferent, "At least two phases must wrap at different update iterations, indicating independent wrap behavior");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_01_001_init_enables_egtm_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_01_002_init_skips_enable_and_cmu_when_already_enabled);

    RUN_TEST(test_TC_02_001_init_config_sets_frequency_and_num_channels_in_initConfig);
    RUN_TEST(test_TC_02_002_init_config_sets_frequency_and_num_channels_in_init_call_and_led_gpio_configured);

    RUN_TEST(test_TC_03_001_after_init_first_update_applies_three_duties_once);
    RUN_TEST(test_TC_03_002_repeated_updates_eventually_wrap_a_channel);

    RUN_TEST(test_TC_04_001_isr_toggle_increments_once);
    RUN_TEST(test_TC_04_002_isr_toggle_accumulates_multiple_calls);

    RUN_TEST(test_TC_05_001_update_calls_hal_once_per_invocation);
    RUN_TEST(test_TC_05_002_update_following_disabled_init_still_updates_hal);

    RUN_TEST(test_TC_06_001_update_duties_respect_min_fraction_bounds);
    RUN_TEST(test_TC_06_002_pwm_frequency_remains_configured_after_updates);

    RUN_TEST(test_TC_07_001_duty_increments_from_initial_values);
    RUN_TEST(test_TC_07_002_each_channel_wraps_independently_and_hal_called_once_per_update);

    return UNITY_END();
}
