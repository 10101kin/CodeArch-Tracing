#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQ_HZ                   (20000U)
#define UT_NUM_CHANNELS                  (3U)
#define UT_DEADTIME_TICKS                (50U)    /* 0.5 us at 100 MHz FXCLK0 */
#define UT_MIN_PULSE_TICKS               (100U)   /* 1.0 us at 100 MHz FXCLK0 */
#define UT_INIT_DUTY_U                   (0.25f)
#define UT_INIT_DUTY_V                   (0.50f)
#define UT_INIT_DUTY_W                   (0.75f)
#define UT_FXCLK0_HZ                     (100000000U)

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* ========================= GROUP 1 - initGtmTomPwm: initialization / enable guard ========================= */
void test_TC_G1_001_init_enables_gtm_and_fxclk_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0; /* GTM not enabled */
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when disabled initially");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK domain must be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "Center-aligned mode must be set exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer must be started once after configuration");
}

void test_TC_G1_002_init_skips_enable_when_gtm_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* GTM already enabled */
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 1; /* FXCLK already enabled */
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "Clocks enable must be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_initConfig_getCallCount(), "Timer initConfig still required once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init still required once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(), "PwmHl initConfig still required once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init still required once");
}

/* ========================= GROUP 2 - initGtmTomPwm: configuration values ========================= */
void test_TC_G2_001_init_sets_frequency_channels_and_deadtime_correctly(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_init_lastNumChannels, "Channel count must be 3 for U,V,W phases");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ, mock_init_lastFrequency, "PWM switching frequency must be 20 kHz");
    for (unsigned i = 0; i < UT_NUM_CHANNELS; ++i)
    {
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_dt_lastDtRising[i], "Rising dead-time ticks must equal 0.5 us");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_dt_lastDtFalling[i], "Falling dead-time ticks must equal 0.5 us");
    }
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "PWM mode must be configured once (center-aligned)");
}

void test_TC_G2_002_init_applies_initial_duties_with_shadow_update_sequence(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert (shadow update sequence) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must be called once during init shadow update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "SetOnTime must be called once to apply initial duties");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate must be called once to sync outputs");

    /* Assert (initial duty ordering and bounds) */
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[0] >= UT_MIN_PULSE_TICKS, "U duty must respect minimum pulse ticks");
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[1] >= UT_MIN_PULSE_TICKS, "V duty must respect minimum pulse ticks");
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[2] >= UT_MIN_PULSE_TICKS, "W duty must respect minimum pulse ticks");
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[0] < mock_update_lastDuties[1], "Initial U (25%) must be less than V (50%)");
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[1] < mock_update_lastDuties[2], "Initial V (50%) must be less than W (75%)");
}

/* ========================= GROUP 3 - initGtmTomPwm: runtime update logic (initial shadow update) ========================= */
void test_TC_G3_001_init_uses_single_shadow_update_for_all_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "Only one SetOnTime call expected (not per channel)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must occur once around initial write");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate must occur once to synchronize");
}

void test_TC_G3_002_init_starts_timer_after_configuration(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_run_getCallCount(), "Timer_run must be invoked once after init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer must be initialized exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl must be initialized exactly once");
}

/* ========================= GROUP 4 - updateGtmTomPwmDutyCycles: initialization / enable guard ========================= */
void test_TC_G4_001_update_performs_single_shadow_update_call_per_invocation(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    uint32_t baseDisable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32_t baseSet     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32_t baseApply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseDisable + 1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must increment by 1 per update call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseSet + 1,     mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "SetOnTime must increment by 1 per update call");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseApply + 1,   mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "ApplyUpdate must increment by 1 per update call");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_Timer_init_getCallCount(), "Timer init must still be exactly once after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Tom_PwmHl_init_getCallCount(), "PwmHl init must still be exactly once after update");
}

void test_TC_G4_002_update_multiple_calls_accumulate_expected_counts(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1; /* already enabled */
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    const uint32_t nCalls = 5;

    /* Act */
    for (uint32_t i = 0; i < nCalls; ++i)
    {
        updateGtmTomPwmDutyCycles();
    }

    /* Assert: totals = 1 (init) + nCalls */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + nCalls, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate total must equal init(1) + updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + nCalls, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "SetOnTime total must equal init(1) + updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + nCalls, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "ApplyUpdate total must equal init(1) + updates");
}

/* ========================= GROUP 5 - updateGtmTomPwmDutyCycles: configuration values ========================= */
void test_TC_G5_001_update_does_not_change_frequency_channelcount_or_deadtime(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_init_lastNumChannels, "Channel count must remain 3 after update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQ_HZ,  mock_init_lastFrequency,   "PWM frequency must remain 20 kHz after update");
    for (unsigned i = 0; i < UT_NUM_CHANNELS; ++i)
    {
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_dt_lastDtRising[i], "Rising dead-time must remain unchanged after update");
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DEADTIME_TICKS, mock_dt_lastDtFalling[i], "Falling dead-time must remain unchanged after update");
    }
}

void test_TC_G5_002_update_does_not_reconfigure_pwm_mode(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    uint32_t baseSetMode = mock_IfxGtm_Tom_PwmHl_setMode_getCallCount();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert: setMode only done in init, not during updates */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseSetMode, mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(), "PWM mode must not be reconfigured during update");
}

/* ========================= GROUP 6 - updateGtmTomPwmDutyCycles: runtime update logic ========================= */
void test_TC_G6_001_update_increments_duty_when_below_boundary(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    uint32_t d0_U = mock_update_lastDuties[0];
    uint32_t d0_V = mock_update_lastDuties[1];
    uint32_t d0_W = mock_update_lastDuties[2];

    uint32_t baseDisable = mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount();
    uint32_t baseSet     = mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount();
    uint32_t baseApply   = mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount();

    /* Act */
    updateGtmTomPwmDutyCycles();

    /* Assert: confirm increment (at least U should increase from 25%) */
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[0] > d0_U, "U duty should increase after one update step when below max");
    TEST_ASSERT_MESSAGE(mock_update_lastDuties[1] != d0_V || mock_update_lastDuties[2] != d0_W, "At least one of V/W should change after update");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseDisable + 1, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate increments by 1 during update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseSet + 1,     mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(),    "SetOnTime increments by 1 during update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(baseApply + 1,   mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(),  "ApplyUpdate increments by 1 during update");
}

void test_TC_G6_002_update_wraps_each_channel_independently_when_exceeding_max(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;
    mock_IfxGtm_isModuleSuspended_returnValue = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = 1;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = UT_FXCLK0_HZ;
    mock_IfxGtm_Tom_Timer_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = 1;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = 1;
    initGtmTomPwm();

    uint32_t prev[3] = { mock_update_lastDuties[0], mock_update_lastDuties[1], mock_update_lastDuties[2] };
    uint8_t wrapped[3] = {0, 0, 0};

    const uint32_t N = 1000U; /* sufficient to force wrap on all channels */
    for (uint32_t i = 0; i < N; ++i)
    {
        updateGtmTomPwmDutyCycles();
        for (unsigned ch = 0; ch < 3; ++ch)
        {
            uint32_t cur = mock_update_lastDuties[ch];
            if (cur < prev[ch]) { wrapped[ch] = 1; }
            prev[ch] = cur;
        }
    }

    TEST_ASSERT_MESSAGE(wrapped[0] == 1, "U channel must wrap back to min threshold at some point");
    TEST_ASSERT_MESSAGE(wrapped[1] == 1, "V channel must wrap back to min threshold at some point");
    TEST_ASSERT_MESSAGE(wrapped[2] == 1, "W channel must wrap back to min threshold at some point");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + N, mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(), "SetOnTime must be called once per update plus once in init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + N, mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(), "DisableUpdate must be called once per update plus once in init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1 + N, mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(), "ApplyUpdate must be called once per update plus once in init");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_gtm_and_fxclk_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_gtm_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_channels_and_deadtime_correctly);
    RUN_TEST(test_TC_G2_002_init_applies_initial_duties_with_shadow_update_sequence);

    RUN_TEST(test_TC_G3_001_init_uses_single_shadow_update_for_all_channels);
    RUN_TEST(test_TC_G3_002_init_starts_timer_after_configuration);

    RUN_TEST(test_TC_G4_001_update_performs_single_shadow_update_call_per_invocation);
    RUN_TEST(test_TC_G4_002_update_multiple_calls_accumulate_expected_counts);

    RUN_TEST(test_TC_G5_001_update_does_not_change_frequency_channelcount_or_deadtime);
    RUN_TEST(test_TC_G5_002_update_does_not_reconfigure_pwm_mode);

    RUN_TEST(test_TC_G6_001_update_increments_duty_when_below_boundary);
    RUN_TEST(test_TC_G6_002_update_wraps_each_channel_independently_when_exceeding_max);

    return UNITY_END();
}
