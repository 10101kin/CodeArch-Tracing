#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* Extern declarations for non-header symbols (ISR and period callback) */
extern void IfxGtm_periodEventFunction(void *data);
extern void interruptGtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                (1e-4f)
#define UT_PWM_FREQ_HZ                  (20000.0f)
#define UT_NUM_CHANNELS                 (3u)
#define UT_INIT_DUTY_U_PERCENT          (25.0f)
#define UT_INIT_DUTY_V_PERCENT          (50.0f)
#define UT_INIT_DUTY_W_PERCENT          (75.0f)
#define UT_STEP_PERCENT                 (10.0f)
#define UT_CMU_MODULE_FREQ_HZ           (100000000u)
#define UT_MIN_PULSE_US                 (1.0f)

/* Spies from mock (provided by harness) */
extern float mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern unsigned int mock_IfxGtm_Pwm_init_lastNumChannels;
extern float mock_IfxGtm_Pwm_init_lastFrequency;

/* Return-value controls from mock (provided by harness) */
extern unsigned int mock_IfxGtm_isEnabled_returnValue;
extern unsigned int mock_IfxGtm_Cmu_isEnabled_returnValue;
extern unsigned int mock_IfxGtm_Cmu_getModuleFrequency_returnValue;

void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initGtmTom3phInv: initialization / enable guard */
void test_TC_G1_001_init_enables_peripheral_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_CMU_MODULE_FREQ_HZ;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM should be enabled when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency should be read once when enabling CMU");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLKs should be enabled when GTM was disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK frequency should be configured under enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "FxCLK frequency should be configured under enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug LED pin should be configured once");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM isEnabled should be queried once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable should NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency read should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "CMU FXCLK enable should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK set should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setClkFrequency_getCallCount(), "FxCLK set should be skipped when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Debug LED pin should be configured once");
}

/* GROUP 2 - initGtmTom3phInv: configuration values */
void test_TC_G2_001_init_sets_frequency_to_20kHz(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM switching frequency should be 20 kHz");
}

void test_TC_G2_002_init_sets_num_channels_to_3(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;

    /* Act */
    initGtmTom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of logical channels should be 3 (U,V,W)");
}

/* GROUP 3 - initGtmTom3phInv: runtime update logic (post-init first updates) */
void test_TC_G3_001_first_update_from_initial_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be applied once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase duty after first update should be 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase duty after first update should be 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase duty after first update should be 85%");
}

void test_TC_G3_002_second_update_accumulates_from_first(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be applied twice");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase duty after second update should be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase duty after second update should be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + 2.0f * UT_STEP_PERCENT, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase duty after second update should be 95%");
}

/* GROUP 4 - initGtmTom3phInv: ISR / interrupt behavior (direct ISR test) */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(), "ISR should toggle LED exactly once");
}

void test_TC_G4_002_isr_toggle_accumulates_across_calls(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_getCallCount(), "ISR toggling should accumulate across calls");
}

/* GROUP 5 - updateGtmTom3phInvDuty: configuration values (persistence) */
void test_TC_G5_001_pwm_init_frequency_persists_after_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQ_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "Configured PWM frequency should remain 20 kHz after updates");
}

void test_TC_G5_002_pwm_init_num_channels_persists_after_update(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Number of channels should remain 3 after updates");
}

/* GROUP 6 - updateGtmTom3phInvDuty: runtime update logic */
void test_TC_G6_001_update_wraps_independently_after_three_steps(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty(); /* 1 */
    updateGtmTom3phInvDuty(); /* 2 */
    updateGtmTom3phInvDuty(); /* 3 */

    /* Expected after 3 updates: U=55, V=80, W=10 (wrap on W) */
    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called once per update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase should be 55% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase should be 80% after 3 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase should wrap to 10% after 3 updates");
}

void test_TC_G6_002_update_wraps_all_channels_over_multiple_steps(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Simulate N updates and compute expected with the same wrap rule */
    const unsigned int N = 8u; /* Enough to force at least one wrap on each channel */
    float expected[3];
    expected[0] = UT_INIT_DUTY_U_PERCENT;
    expected[1] = UT_INIT_DUTY_V_PERCENT;
    expected[2] = UT_INIT_DUTY_W_PERCENT;

    for (unsigned int i = 0; i < N; ++i)
    {
        updateGtmTom3phInvDuty();
        for (unsigned int ch = 0; ch < 3u; ++ch)
        {
            float next = expected[ch] + UT_STEP_PERCENT;
            if (next >= 100.0f)
            {
                expected[ch] = 0.0f;
                expected[ch] += UT_STEP_PERCENT;
            }
            else
            {
                expected[ch] = next;
            }
            if (expected[ch] < 0.0f) expected[ch] = 0.0f;
            if (expected[ch] > 100.0f) expected[ch] = 100.0f;
        }
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(N, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update should be called once per update iteration");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, expected[0], mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase expected duty after N updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, expected[1], mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase expected duty after N updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, expected[2], mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase expected duty after N updates");
}

/* GROUP 7 - IfxGtm_periodEventFunction: configuration/no-op verification */
void test_TC_G7_001_period_callback_does_not_toggle_led_or_update_duty(void)
{
    /* Arrange */
    unsigned int toggle_before = mock_togglePin_getCallCount();
    unsigned int update_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_getCallCount(), "Period callback must not toggle the LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(update_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duty");
}

void test_TC_G7_002_period_callback_is_noop_with_nonnull_arg(void)
{
    /* Arrange */
    unsigned int toggle_before = mock_togglePin_getCallCount();
    unsigned int update_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    int dummy = 0x1234;
    IfxGtm_periodEventFunction(&dummy);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_getCallCount(), "Period callback must not toggle LED (nonnull data)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(update_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update duty (nonnull data)");
}

/* GROUP 8 - IfxGtm_periodEventFunction: ISR / interrupt behavior (still no-op) */
void test_TC_G8_001_period_callback_no_led_toggle_after_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();
    unsigned int toggle_before = mock_togglePin_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_getCallCount(), "Period callback must never toggle LED even after init");
}

void test_TC_G8_002_period_callback_multiple_calls_have_no_effect(void)
{
    /* Arrange */
    unsigned int toggle_before = mock_togglePin_getCallCount();
    unsigned int update_before = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount();

    /* Act */
    IfxGtm_periodEventFunction(NULL);
    IfxGtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(toggle_before, mock_togglePin_getCallCount(), "Multiple period callbacks must not toggle LED");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(update_before, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple period callbacks must not update duty");
}

/* GROUP 9 - interruptGtmAtom: initialization / enable guard irrelevance */
void test_TC_G9_001_isr_toggles_without_any_init(void)
{
    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(), "ISR should toggle even without module init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR should not trigger PWM duty updates");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Pwm_init_getCallCount(), "ISR should not initialize PWM");
}

void test_TC_G9_002_isr_toggles_even_if_gtm_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(), "ISR should toggle regardless of GTM enable state");
}

/* GROUP 10 - interruptGtmAtom: configuration values unaffected by ISR */
void test_TC_G10_001_isr_does_not_change_pwm_frequency(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();
    float freq_before = mock_IfxGtm_Pwm_init_lastFrequency;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, freq_before, mock_IfxGtm_Pwm_init_lastFrequency, "ISR must not affect configured PWM frequency");
}

void test_TC_G10_002_isr_does_not_change_num_channels(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();
    unsigned int num_before = mock_IfxGtm_Pwm_init_lastNumChannels;

    /* Act */
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(num_before, mock_IfxGtm_Pwm_init_lastNumChannels, "ISR must not affect number of channels");
}

/* GROUP 11 - interruptGtmAtom: ISR / interrupt behavior */
void test_TC_G11_001_isr_multiple_toggles_accumulate(void)
{
    /* Act */
    interruptGtmAtom();
    interruptGtmAtom();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_getCallCount(), "ISR toggling should accumulate across multiple calls");
}

void test_TC_G11_002_isr_toggle_with_background_pwm_activity(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = TRUE;
    initGtmTom3phInv();

    /* Act */
    updateGtmTom3phInvDuty();
    interruptGtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Background PWM update should occur exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_getCallCount(), "ISR should toggle LED once alongside PWM update");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_peripheral_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_frequency_to_20kHz);
    RUN_TEST(test_TC_G2_002_init_sets_num_channels_to_3);

    RUN_TEST(test_TC_G3_001_first_update_from_initial_duties);
    RUN_TEST(test_TC_G3_002_second_update_accumulates_from_first);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_across_calls);

    RUN_TEST(test_TC_G5_001_pwm_init_frequency_persists_after_update);
    RUN_TEST(test_TC_G5_002_pwm_init_num_channels_persists_after_update);

    RUN_TEST(test_TC_G6_001_update_wraps_independently_after_three_steps);
    RUN_TEST(test_TC_G6_002_update_wraps_all_channels_over_multiple_steps);

    RUN_TEST(test_TC_G7_001_period_callback_does_not_toggle_led_or_update_duty);
    RUN_TEST(test_TC_G7_002_period_callback_is_noop_with_nonnull_arg);

    RUN_TEST(test_TC_G8_001_period_callback_no_led_toggle_after_init);
    RUN_TEST(test_TC_G8_002_period_callback_multiple_calls_have_no_effect);

    RUN_TEST(test_TC_G9_001_isr_toggles_without_any_init);
    RUN_TEST(test_TC_G9_002_isr_toggles_even_if_gtm_disabled);

    RUN_TEST(test_TC_G10_001_isr_does_not_change_pwm_frequency);
    RUN_TEST(test_TC_G10_002_isr_does_not_change_num_channels);

    RUN_TEST(test_TC_G11_001_isr_multiple_toggles_accumulate);
    RUN_TEST(test_TC_G11_002_isr_toggle_with_background_pwm_activity);

    return UNITY_END();
}
