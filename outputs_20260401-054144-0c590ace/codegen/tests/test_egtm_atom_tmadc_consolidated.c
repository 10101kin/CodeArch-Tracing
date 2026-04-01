#include "unity.h"
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "egtm_atom_tmadc_consolidated.h"

/* Extern declarations for non-header production symbols (ISR / update) */
extern void updateEgtmAtomDuty(void);
extern void resultISR(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)
#define UT_NUM_CHANNELS                  (3)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)
#define UT_DEAD_TIME_S                   (1e-6f)
#define UT_STEP_PERCENT                  (0.01f)
#define UT_CMU_MODULE_FREQ_HZ            (100000000u)

void setUp(void)   { mock_egtm_atom_tmadc_consolidated_reset(); }
void tearDown(void) {}

/* =====================
 * GROUP 1 - Initialization / enable guard
 * ===================== */
void test_TC_G1_001_init_enables_egtm_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = FALSE;  /* Force enable path */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = UT_CMU_MODULE_FREQ_HZ;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once during init (enable guard)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must be called when EGTM is disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "Module frequency must be read for CMU configuration when enabling EGTM");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Cmu_enableClocks_getCallCount(), "CMU clocks must be enabled when EGTM was disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(), "ATOM timer frequency must be configured once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(), "ATOM timer trigger must be set once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Atom_Timer_run_getCallCount(), "ATOM timer must be started once");

    TEST_ASSERT_TRUE_MESSAGE((mock_IfxEgtm_PinMap_setAtomTout_getCallCount() >= 1u), "PinMap: At least one AtomTout mapping must be applied");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Trigger_trigToAdc_getCallCount(), "ADC trigger routing via IfxEgtm_Trigger_trigToAdc must be configured once");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxAdc_enableModule_getCallCount(), "TMADC: IfxAdc_enableModule must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(), "TMADC: initModuleConfig must be called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxAdc_Tmadc_initModule_getCallCount(), "TMADC: initModule must be called once");

    TEST_ASSERT_TRUE_MESSAGE((mock_IfxCpu_Irq_installInterruptHandler_getCallCount() >= 1u), "Interrupt handler must be installed for TMADC result ISR");
}

void test_TC_G1_002_init_skips_enable_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* Skip enable path */
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_isEnabled_getCallCount(), "IfxEgtm_isEnabled must be called once during init (enable guard)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0u, mock_IfxEgtm_enable_getCallCount(), "IfxEgtm_enable must NOT be called when EGTM is already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called once even if already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called once even if already enabled");
}

/* =====================
 * GROUP 2 - Configuration values
 * ===================== */
void test_TC_G2_001_pwm_init_parameters_match_expected_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;   /* Keep path simple */
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Pwm_init_getCallCount(), "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(UT_NUM_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "PWM init: number of logical channels must be 3 (complementary pairs compressed)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM init: frequency must be 20 kHz");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(), "ATOM trigger timer frequency must be configured to match PWM period");
}

void test_TC_G2_002_deadtime_is_configured_to_expected_value_for_all_phases(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    /* Dead-time is configured to 1e-6 s for each logical phase (rising/falling) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[0], "DT rising[0] must be 1e-6 s");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[0], "DT falling[0] must be 1e-6 s");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[1], "DT rising[1] must be 1e-6 s");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[1], "DT falling[1] must be 1e-6 s");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[2], "DT rising[2] must be 1e-6 s");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_DEAD_TIME_S, mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[2], "DT falling[2] must be 1e-6 s");
}

/* =====================
 * GROUP 3 - Runtime update logic
 * ===================== */
void test_TC_G3_001_update_increments_all_duties_by_step_without_wrap(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtomDuty();

    /* Assert - duties are in PERCENT (0..100) */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_U_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must increment by step in percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_V_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must increment by step in percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, (UT_INIT_DUTY_W_PERCENT + UT_STEP_PERCENT), mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must increment by step in percent");
}

void test_TC_G3_002_update_wraps_phase_W_when_reaching_100_percent_and_other_phases_continue(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Calculate updates to drive Phase W (initial 75%) to wrap boundary */
    /* Wrap semantic: if (duty + step) >= 100 -> duty resets to 0 then adds step => duty = step */
    unsigned int updatesToWrapW = (unsigned int)((100.0f - UT_INIT_DUTY_W_PERCENT) / UT_STEP_PERCENT + 0.5f); /* ceil for integer math */
    if ((float)updatesToWrapW * UT_STEP_PERCENT + UT_INIT_DUTY_W_PERCENT < 100.0f)
    {
        updatesToWrapW += 1u; /* ensure we reach or exceed 100 */
    }

    /* Act - perform the computed number of updates */
    for (unsigned int i = 0; i < updatesToWrapW; ++i)
    {
        updateEgtmAtomDuty();
    }

    /* Expected after updatesToWrapW:
     * U: 25% + N*step = 25 + 25 = 50
     * V: 50% + N*step = 50 + 25 = 75
     * W: wrapped to step (0.01%) per wrap rule (>= 100 triggers wrap)
     */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must advance without wrap to 50%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 75.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Phase V duty must advance without wrap to 75%");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_STEP_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Phase W duty must wrap to step after reaching 100%");
}

/* =====================
 * GROUP 4 - ISR / interrupt behavior
 * ===================== */
void test_TC_G4_001_resultISR_toggles_led_once_per_call(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1u, mock_togglePin_callCount, "resultISR must toggle LED exactly once per invocation");
}

void test_TC_G4_002_resultISR_toggle_accumulates_over_multiple_calls(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = TRUE;
    initEgtmAtom3phInv();

    /* Act */
    resultISR();
    resultISR();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2u, mock_togglePin_callCount, "resultISR must toggle LED on every call (cumulative)");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_egtm_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_enable_when_already_enabled);

    RUN_TEST(test_TC_G2_001_pwm_init_parameters_match_expected_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_deadtime_is_configured_to_expected_value_for_all_phases);

    RUN_TEST(test_TC_G3_001_update_increments_all_duties_by_step_without_wrap);
    RUN_TEST(test_TC_G3_002_update_wraps_phase_W_when_reaching_100_percent_and_other_phases_continue);

    RUN_TEST(test_TC_G4_001_resultISR_toggles_led_once_per_call);
    RUN_TEST(test_TC_G4_002_resultISR_toggle_accumulates_over_multiple_calls);

    return UNITY_END();
}
