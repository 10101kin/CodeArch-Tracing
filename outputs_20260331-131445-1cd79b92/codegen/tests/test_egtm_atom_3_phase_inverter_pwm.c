#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern for ISR (may be defined only in .c) */
extern void resultISR(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_PWM_FREQUENCY_HZ_INT          (20000)
#define UT_PWM_FREQUENCY_HZ_FLOAT        (20000.0f)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_DTM_DEADTIME_US               (1.0f)
#define UT_MODULE_FREQ_HZ                (100000000u)
#define UT_ADC_TRIGGER_CHANNEL           (3)

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/**********************
 * GROUP 1 - init: enable guard and basic init sequencing
 **********************/
void test_TC_G1_001_init_enables_and_configures_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE; /* Force enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_MODULE_FREQ_HZ;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u; /* arbitrary period */
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - module enable and CMU setup occur once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable called when module disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency called once inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency called once for CLK0");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks called once with CLK0 enable");

    /* Assert - PWM init pattern executed */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init called once");

    /* Assert - ATOM timer/trigger and routing */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(), "ATOM Timer setFrequency called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_getPeriod_getCallCount(), "ATOM Timer getPeriod called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(), "ATOM Timer setTrigger called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_run_getCallCount(), "ATOM Timer run called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_PinMap_setAtomTout_getCallCount(), "PinMap_setAtomTout called once for P33.0 route");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "Trigger to ADC called once");

    /* Assert - LED pin configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured as push-pull output once");
}

void test_TC_G1_002_init_skips_cmu_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE; /* Skip enable path */
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - still checks enabled */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled called once");

    /* Assert - enable/CMU calls skipped */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(), "CMU setGclkFrequency not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(), "CMU setClkFrequency not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU enableClocks not called when already enabled");

    /* Assert - PWM/timer/route still configured */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(), "ATOM Timer setFrequency called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_getPeriod_getCallCount(), "ATOM Timer getPeriod called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(), "ATOM Timer setTrigger called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_run_getCallCount(), "ATOM Timer run called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_PinMap_setAtomTout_getCallCount(), "PinMap_setAtomTout called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "Trigger to ADC called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED pin configured as push-pull output once");
}

/**********************
 * GROUP 2 - init: configuration values
 **********************/
void test_TC_G2_001_init_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert - application values captured by init() spies */
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "PWM numChannels must be 3 for 3-phase");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_PWM_FREQUENCY_HZ_INT, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM frequency must be 20 kHz");
}

void test_TC_G2_002_init_calls_initConfig_then_init_once_each(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(), "init must be called exactly once");
}

/**********************
 * GROUP 3 - init: runtime/trigger setup logic
 **********************/
void test_TC_G3_001_init_configures_atom_timer_trigger_sequence(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(), "Timer frequency set once (20 kHz)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_getPeriod_getCallCount(), "Timer period read once to compute 50% trigger");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(), "Timer trigger configured once at 50% (edge-aligned)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Atom_Timer_run_getCallCount(), "Timer started once");
}

void test_TC_G3_002_init_routes_trigger_pin_and_trig_to_adc(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_PinMap_setAtomTout_getCallCount(), "ATOM0 CH3 routed to P33.0 exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "EGTM trigger to ADC configured exactly once");
}

/**********************
 * GROUP 4 - init: ISR visibility and basic toggle (post-init)
 **********************/
void test_TC_G4_001_isr_toggle_once_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "LED toggled once by ISR");
}

void test_TC_G4_002_isr_toggle_accumulates_multiple_calls_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    resultISR();
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_togglePin_callCount, "LED toggle count accumulates across ISR calls");
}

/**********************
 * GROUP 5 - update: configuration values reflected on update
 **********************/
void test_TC_G5_001_update_applies_in_range_duties_immediately(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 duties[3] = {UT_INIT_DUTY_U_PERCENT, UT_INIT_DUTY_V_PERCENT, UT_INIT_DUTY_W_PERCENT};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert - one per channel */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(), "Per-channel immediate duty update called 3 times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[0], "Phase U duty applied in percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[1], "Phase V duty applied in percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[2], "Phase W duty applied in percent");
}

void test_TC_G5_002_update_applies_exact_initial_duties_constants(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 duties[3] = {UT_INIT_DUTY_U_PERCENT, UT_INIT_DUTY_V_PERCENT, UT_INIT_DUTY_W_PERCENT};

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(), "Immediate duty updates executed for all 3 channels");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 25.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[0], "U=25% after update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[1], "V=50% after update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[2], "W=75% after update");
}

/**********************
 * GROUP 6 - update: runtime update logic (clamp and sequencing)
 **********************/
void test_TC_G6_001_update_clamps_out_of_range_values_and_calls_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 duties[3] = {-10.0f, 50.5f, 123.4f}; /* expect clamp to 0.0, 50.5, 100.0 */

    /* Act */
    updateEgtmAtom3phInvDuty(duties);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(), "Per-channel update called 3 times");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 0.0f,   mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[0], "Phase U clamped to 0%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.5f,  mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[1], "Phase V left as 50.5%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 100.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[2], "Phase W clamped to 100%");
}

void test_TC_G6_002_multiple_updates_accumulate_call_count_and_overwrite_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    float32 duties1[3] = {1.0f, 2.0f, 3.0f};
    float32 duties2[3] = {4.0f, 5.0f, 6.0f};

    /* Act */
    updateEgtmAtom3phInvDuty(duties1);
    updateEgtmAtom3phInvDuty(duties2);

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(6, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(), "Two updates → 6 per-channel calls total");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 4.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[0], "Last U duty reflects second update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 5.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[1], "Last V duty reflects second update");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 6.0f, mock_IfxEgtm_Pwm_updateChannelDutyImmediate_lastDuties[2], "Last W duty reflects second update");
}

/**********************
 * GROUP 7 - resultISR: initialization / enable guard cases
 **********************/
void test_TC_G7_001_isr_toggle_without_init(void)
{
    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR toggles LED even without init context");
}

void test_TC_G7_002_isr_toggle_after_init(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 2000u;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_togglePin_callCount, "ISR toggles LED exactly once after init");
}

/**********************
 * GROUP 8 - resultISR: ISR / interrupt behavior
 **********************/
void test_TC_G8_001_isr_two_calls_increment_by_two(void)
{
    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_togglePin_callCount, "Two ISR calls → two toggles");
}

void test_TC_G8_002_isr_many_calls_increment_correctly(void)
{
    /* Act */
    for (int i = 0; i < 10; ++i) {
        resultISR();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(10, mock_togglePin_callCount, "Ten ISR calls → ten toggles");
}

int main(void)
{
    UNITY_BEGIN();

    /* GROUP 1 */
    RUN_TEST(test_TC_G1_001_init_enables_and_configures_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_already_enabled);

    /* GROUP 2 */
    RUN_TEST(test_TC_G2_001_init_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_calls_initConfig_then_init_once_each);

    /* GROUP 3 */
    RUN_TEST(test_TC_G3_001_init_configures_atom_timer_trigger_sequence);
    RUN_TEST(test_TC_G3_002_init_routes_trigger_pin_and_trig_to_adc);

    /* GROUP 4 */
    RUN_TEST(test_TC_G4_001_isr_toggle_once_after_init);
    RUN_TEST(test_TC_G4_002_isr_toggle_accumulates_multiple_calls_after_init);

    /* GROUP 5 */
    RUN_TEST(test_TC_G5_001_update_applies_in_range_duties_immediately);
    RUN_TEST(test_TC_G5_002_update_applies_exact_initial_duties_constants);

    /* GROUP 6 */
    RUN_TEST(test_TC_G6_001_update_clamps_out_of_range_values_and_calls_per_channel);
    RUN_TEST(test_TC_G6_002_multiple_updates_accumulate_call_count_and_overwrite_values);

    /* GROUP 7 */
    RUN_TEST(test_TC_G7_001_isr_toggle_without_init);
    RUN_TEST(test_TC_G7_002_isr_toggle_after_init);

    /* GROUP 8 */
    RUN_TEST(test_TC_G8_001_isr_two_calls_increment_by_two);
    RUN_TEST(test_TC_G8_002_isr_many_calls_increment_correctly);

    return UNITY_END();
}
