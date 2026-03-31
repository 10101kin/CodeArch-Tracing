#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Externs for functions not declared in the production header */
extern void interruptEgtmAtomPeriod(void);
extern void IfxEgtm_periodEventFunction(void *data);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                    (1e-4f)
#define UT_NUM_CHANNELS                     (3u)
#define UT_PWM_FREQUENCY_HZ                 (20000u)
#define UT_INIT_DUTY_U_PERCENT              (25.0f)
#define UT_INIT_DUTY_V_PERCENT              (50.0f)
#define UT_INIT_DUTY_W_PERCENT              (75.0f)
#define UT_DUTY_STEP_PERCENT                (10.0f)
#define UT_DEADTIME_RISING_US               (1.0f)
#define UT_DEADTIME_FALLING_US              (1.0f)
#define UT_ISR_PRIORITY                     (20u)
#define UT_WAIT_TIME_MS                     (500u)
/* Pin mapping (documentary only) */
#define UT_PIN_U_HS                         "P20.8"
#define UT_PIN_U_LS                         "P20.9"
#define UT_PIN_V_HS                         "P21.4"
#define UT_PIN_V_LS                         "P20.11"
#define UT_PIN_W_HS                         "P20.12"
#define UT_PIN_W_LS                         "P20.13"
#define UT_LED_PIN                          "P03.9"
#define UT_EGTM_ATOM_CLOCK                  "Fxclk_0"
#define UT_DTM_CLOCK                        "cmuClock0"
#define UT_DTM_TARGET_HZ                    (100000000u)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
void test_TC_1_001_init_enables_egtm_and_configures_cmu_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_DTM_TARGET_HZ;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called when configuring clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency must be called in enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency must be called in enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks must be called in enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin must be configured as output once");
}

void test_TC_1_002_init_skips_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "setGclkFrequency must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "setClkFrequency must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "enableClocks must not be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must still be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin setup must occur once");
}

/* GROUP 2 - initEgtmAtom3phInv: configuration values */
void test_TC_2_001_init_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of channels must be 3");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
}

void test_TC_2_002_init_calls_initConfig_and_init_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "init must be called exactly once");
}

/* GROUP 3 - initEgtmAtom3phInv: runtime update logic */
void test_TC_3_001_first_update_increments_duties_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update must call unified API once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should increment to 35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should increment to 60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT + UT_DUTY_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should increment to 85%");
}

void test_TC_3_002_third_update_wraps_phase_W_only(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */
    updateEgtmAtom3phInvDuty(); /* 3: W wraps to 10 */

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 55.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 3 updates, Phase U should be 55%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 80.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 3 updates, Phase V should be 80%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 3 updates, Phase W should wrap to 10%");
}

/* GROUP 4 - initEgtmAtom3phInv: ISR / interrupt behavior */
void test_TC_4_001_isr_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED exactly once per call");
}

void test_TC_4_002_isr_toggle_accumulates_over_multiple_calls(void)
{
    /* Act */
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "ISR toggles must accumulate across calls");
}

/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
void test_TC_5_001_first_update_applies_initial_plus_step_from_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 35.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U after first update = 25+10=35%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V after first update = 50+10=60%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 85.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W after first update = 75+10=85%");
}

void test_TC_5_002_update_calls_unified_api_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "IfxEgtm_Pwm_updateChannelsDutyImmediate must be called once per update");
}

/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
void test_TC_6_001_two_updates_progress_duties_linearly(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty(); /* 1 */
    updateEgtmAtom3phInvDuty(); /* 2 */

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Two updates should produce two HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 45.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "After 2 updates, Phase U should be 45%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 70.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "After 2 updates, Phase V should be 70%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 95.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "After 2 updates, Phase W should be 95%");
}

void test_TC_6_002_eight_updates_wrap_channels_independently(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    for (unsigned i = 0; i < 8u; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* After 8 updates: U=10, V=40, W=60 */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Eight updates should produce eight HAL calls");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U should wrap to 10% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 40.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V should be 40% after 8 updates");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 60.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W should be 60% after 8 updates");
}

/* GROUP 7 - interruptEgtmAtomPeriod: initialization / enable guard */
void test_TC_7_001_isr_can_be_called_before_init_and_toggles_led(void)
{
    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "ISR must toggle LED even if init not called");
}

void test_TC_7_002_isr_does_not_invoke_pwm_update_api(void)
{
    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not call PWM update API");
}

/* GROUP 8 - interruptEgtmAtomPeriod: configuration values */
void test_TC_8_001_isr_does_not_change_configured_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxEgtm_Pwm_init_lastFrequency;

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxEgtm_Pwm_init_lastFrequency, "ISR must not alter configured PWM frequency");
}

void test_TC_8_002_isr_does_not_change_configured_num_channels(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    unsigned int before = mock_IfxEgtm_Pwm_init_lastNumChannels;

    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before, mock_IfxEgtm_Pwm_init_lastNumChannels, "ISR must not alter configured number of channels");
}

/* GROUP 9 - interruptEgtmAtomPeriod: runtime update logic */
void test_TC_9_001_isr_does_not_update_duty_array(void)
{
    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ISR must not trigger duty update");
}

void test_TC_9_002_isr_multiple_calls_do_not_update_pwm(void)
{
    /* Act */
    interruptEgtmAtomPeriod();
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Multiple ISR calls must not trigger duty updates");
}

/* GROUP 10 - interruptEgtmAtomPeriod: ISR / interrupt behavior */
void test_TC_10_001_interrupt_toggles_led_once(void)
{
    /* Act */
    interruptEgtmAtomPeriod();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "interruptEgtmAtomPeriod must toggle LED once");
}

void test_TC_10_002_interrupt_toggles_led_five_times(void)
{
    /* Act */
    for (unsigned i = 0; i < 5u; ++i)
    {
        interruptEgtmAtomPeriod();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(5u, mock_togglePin_callCount, "ISR must toggle LED 5 times after 5 calls");
}

/* GROUP 11 - IfxEgtm_periodEventFunction: configuration values */
void test_TC_11_001_period_callback_has_no_side_effects_on_config(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    initEgtmAtom3phInv();
    unsigned int freq_before = mock_IfxEgtm_Pwm_init_lastFrequency;
    unsigned int ch_before   = mock_IfxEgtm_Pwm_init_lastNumChannels;

    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(freq_before, mock_IfxEgtm_Pwm_init_lastFrequency, "Period callback must not change PWM frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(ch_before,   mock_IfxEgtm_Pwm_init_lastNumChannels, "Period callback must not change number of channels");
}

void test_TC_11_002_period_callback_does_not_toggle_led(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED");
}

/* GROUP 12 - IfxEgtm_periodEventFunction: ISR / interrupt behavior */
void test_TC_12_001_period_callback_does_not_update_pwm(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not update PWM duties");
}

void test_TC_12_002_period_callback_multiple_calls_still_no_effect(void)
{
    /* Act */
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);
    IfxEgtm_periodEventFunction(NULL);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_togglePin_callCount, "Period callback must not toggle LED across multiple calls");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Period callback must not trigger PWM updates across multiple calls");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_1_001_init_enables_egtm_and_configures_cmu_when_disabled);
    RUN_TEST(test_TC_1_002_init_skips_cmu_when_already_enabled);

    RUN_TEST(test_TC_2_001_init_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_2_002_init_calls_initConfig_and_init_once_each);

    RUN_TEST(test_TC_3_001_first_update_increments_duties_without_wrap);
    RUN_TEST(test_TC_3_002_third_update_wraps_phase_W_only);

    RUN_TEST(test_TC_4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_4_002_isr_toggle_accumulates_over_multiple_calls);

    RUN_TEST(test_TC_5_001_first_update_applies_initial_plus_step_from_config);
    RUN_TEST(test_TC_5_002_update_calls_unified_api_once);

    RUN_TEST(test_TC_6_001_two_updates_progress_duties_linearly);
    RUN_TEST(test_TC_6_002_eight_updates_wrap_channels_independently);

    RUN_TEST(test_TC_7_001_isr_can_be_called_before_init_and_toggles_led);
    RUN_TEST(test_TC_7_002_isr_does_not_invoke_pwm_update_api);

    RUN_TEST(test_TC_8_001_isr_does_not_change_configured_frequency);
    RUN_TEST(test_TC_8_002_isr_does_not_change_configured_num_channels);

    RUN_TEST(test_TC_9_001_isr_does_not_update_duty_array);
    RUN_TEST(test_TC_9_002_isr_multiple_calls_do_not_update_pwm);

    RUN_TEST(test_TC_10_001_interrupt_toggles_led_once);
    RUN_TEST(test_TC_10_002_interrupt_toggles_led_five_times);

    RUN_TEST(test_TC_11_001_period_callback_has_no_side_effects_on_config);
    RUN_TEST(test_TC_11_002_period_callback_does_not_toggle_led);

    RUN_TEST(test_TC_12_001_period_callback_does_not_update_pwm);
    RUN_TEST(test_TC_12_002_period_callback_multiple_calls_still_no_effect);

    return UNITY_END();
}
