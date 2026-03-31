#include "unity.h"
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_PHASE_CHANNELS            (3U)
#define UT_NUM_ADC_TRIG_CHANNELS         (1U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_ADC_TRIG_DUTY_PERCENT         (50.0f)
#define UT_PWM_FREQUENCY_HZ              (20000.0f)

void setUp(void)   { mock_egtm_atom_adc_tmadc_multiple_channels_reset(); }
void tearDown(void) {}

/* =============================== */
/* GROUP 1 - initEgtmAtom3phInv: initialization / enable guard */
/* =============================== */
void test_TC_G1_001_Init_WhenEgtmAlreadyEnabled_NoEnableCallAndClockConfigured(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "TC4xx API: IfxEgtm_Pwm_initConfig called once for inverter init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "TC4xx API: IfxEgtm_Pwm_init called once for inverter init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "TC4xx API: IfxEgtm_isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),         "TC4xx API: IfxEgtm_enable must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "TC4xx API: CMU module frequency read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "TC4xx API: CMU clocks enabled once");
}

void test_TC_G1_002_Init_WhenEgtmDisabled_EnablesModuleAndClocks(void)
{
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    initEgtmAtom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "TC4xx API: IfxEgtm_isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),         "TC4xx API: IfxEgtm_enable must be called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "TC4xx API: IfxEgtm_Pwm_initConfig called once for inverter init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "TC4xx API: IfxEgtm_Pwm_init called once for inverter init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "TC4xx API: CMU module frequency read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "TC4xx API: CMU clocks enabled once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(),       "GPIO: LED configured as output once after PWM init");
}

/* =============================== */
/* GROUP 2 - initEgtmAtom3phInv: configuration values */
/* =============================== */
void test_TC_G2_001_Init_Config_SetsNumChannelsAndFrequency(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtom3phInv();

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)UT_NUM_PHASE_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Inverter: numChannels must be 3 (complementary pairs handled internally)");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "Inverter: PWM frequency must be 20 kHz");
}

void test_TC_G2_002_Init_Config_DoesNotCallUpdateDuringInit(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter init: must not push duty updates during initialization");
}

/* =============================== */
/* GROUP 3 - initEgtmAtom3phInv: runtime update logic (init side-effects) */
/* =============================== */
void test_TC_G3_001_Init_DoesNotUpdateDuty(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtom3phInv();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter init: HAL duty update must not be called");
}

void test_TC_G3_002_Init_FollowedByUpdate_UsesInitialPatternValues(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 req[UT_NUM_PHASE_CHANNELS] = {UT_INIT_DUTY_U_PERCENT, UT_INIT_DUTY_V_PERCENT, UT_INIT_DUTY_W_PERCENT};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(req);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter update: HAL duty update called exactly once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_U_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Inverter update: U duty in percent passed without scaling");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_V_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "Inverter update: V duty in percent passed without scaling");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_INIT_DUTY_W_PERCENT, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "Inverter update: W duty in percent passed without scaling");
}

/* =============================== */
/* GROUP 4 - updateEgtmAtom3phInvDuty: initialization / enable guard */
/* =============================== */
void test_TC_G4_001_Update_AppliesRequestedDutiesOnce(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 req[UT_NUM_PHASE_CHANNELS] = {10.0f, 20.0f, 30.0f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(req);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter update: HAL duty update called once");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase duty forwarded as percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 20.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase duty forwarded as percent");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 30.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase duty forwarded as percent");
}

void test_TC_G4_002_Update_Twice_IncrementsCallCountAndOverwritesDuties(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 first[UT_NUM_PHASE_CHANNELS]  = {1.0f, 2.0f, 3.0f};
    float32 second[UT_NUM_PHASE_CHANNELS] = {90.0f, 50.0f, 10.0f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(first);
    updateEgtmAtom3phInvDuty(second);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter update: HAL duty update called once per invocation");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 90.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U phase duty overwritten by second call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V phase duty overwritten by second call");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 10.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W phase duty overwritten by second call");
}

/* =============================== */
/* GROUP 5 - updateEgtmAtom3phInvDuty: configuration values */
/* =============================== */
void test_TC_G5_001_Update_UsesThreeChannelsConfiguredAtInit(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 req[UT_NUM_PHASE_CHANNELS] = {33.0f, 66.0f, 99.0f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(req);

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)UT_NUM_PHASE_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "Inverter: update relies on 3 configured logical channels");
}

void test_TC_G5_002_Update_DoesNotReinitializePwm(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 req[UT_NUM_PHASE_CHANNELS] = {40.0f, 50.0f, 60.0f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(req);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "Inverter: initConfig must occur only during init, not update");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "Inverter: init must occur only during init, not update");
}

/* =============================== */
/* GROUP 6 - updateEgtmAtom3phInvDuty: runtime update logic */
/* =============================== */
void test_TC_G6_001_Update_PassesPercentValuesWithoutScaling(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 req[UT_NUM_PHASE_CHANNELS] = {33.33f, 66.66f, 99.99f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(req);

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 33.33f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "U duty percent forwarded 1:1");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 66.66f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1], "V duty percent forwarded 1:1");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, 99.99f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2], "W duty percent forwarded 1:1");
}

void test_TC_G6_002_Update_MultipleUpdatesCallOncePerInvocation(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;
    float32 a[UT_NUM_PHASE_CHANNELS] = {5.0f, 10.0f, 15.0f};
    float32 b[UT_NUM_PHASE_CHANNELS] = {15.0f, 10.0f, 5.0f};

    initEgtmAtom3phInv();
    updateEgtmAtom3phInvDuty(a);
    updateEgtmAtom3phInvDuty(b);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Inverter: HAL update called once per update invocation");
}

/* =============================== */
/* GROUP 7 - initEgtmAtomAdcTrigger: initialization / enable guard */
/* =============================== */
void test_TC_G7_001_InitAdcTrigger_WhenEgtmAlreadyEnabled_NoEnableCall(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "ADC Trigger: IfxEgtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "ADC Trigger: IfxEgtm_Pwm_init called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "ADC Trigger: IfxEgtm_isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),         "ADC Trigger: IfxEgtm_enable not called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "ADC Trigger: CMU module frequency read once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "ADC Trigger: CMU clocks enabled once");
}

void test_TC_G7_002_InitAdcTrigger_WhenEgtmDisabled_EnablesModule(void)
{
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "ADC Trigger: IfxEgtm_isEnabled checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),         "ADC Trigger: IfxEgtm_enable called when disabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "ADC Trigger: IfxEgtm_Pwm_initConfig called once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "ADC Trigger: IfxEgtm_Pwm_init called once");
}

/* =============================== */
/* GROUP 8 - initEgtmAtomAdcTrigger: configuration values */
/* =============================== */
void test_TC_G8_001_InitAdcTrigger_FrequencyIs20kHz(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(UT_FLOAT_EPSILON, UT_PWM_FREQUENCY_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "ADC Trigger: PWM frequency must be 20 kHz");
}

void test_TC_G8_002_InitAdcTrigger_NumChannelsIsOne(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_EQUAL_INT_MESSAGE((int)UT_NUM_ADC_TRIG_CHANNELS, (int)mock_IfxEgtm_Pwm_init_lastNumChannels, "ADC Trigger: numChannels must be 1");
}

/* =============================== */
/* GROUP 9 - initEgtmAtomAdcTrigger: runtime update logic (init side-effects) */
/* =============================== */
void test_TC_G9_001_InitAdcTrigger_DoesNotUpdateDuty(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "ADC Trigger init: must not call duty update during initialization");
}

void test_TC_G9_002_InitAdcTrigger_DoesNotConfigureLedPinMode(void)
{
    mock_IfxEgtm_isEnabled_returnValue = TRUE;

    initEgtmAtomAdcTrigger();

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxPort_setPinModeOutput_getCallCount(), "ADC Trigger init: should not configure LED GPIO");
}

int main(void)
{
    UNITY_BEGIN();

    /* Group 1 */
    RUN_TEST(test_TC_G1_001_Init_WhenEgtmAlreadyEnabled_NoEnableCallAndClockConfigured);
    RUN_TEST(test_TC_G1_002_Init_WhenEgtmDisabled_EnablesModuleAndClocks);

    /* Group 2 */
    RUN_TEST(test_TC_G2_001_Init_Config_SetsNumChannelsAndFrequency);
    RUN_TEST(test_TC_G2_002_Init_Config_DoesNotCallUpdateDuringInit);

    /* Group 3 */
    RUN_TEST(test_TC_G3_001_Init_DoesNotUpdateDuty);
    RUN_TEST(test_TC_G3_002_Init_FollowedByUpdate_UsesInitialPatternValues);

    /* Group 4 */
    RUN_TEST(test_TC_G4_001_Update_AppliesRequestedDutiesOnce);
    RUN_TEST(test_TC_G4_002_Update_Twice_IncrementsCallCountAndOverwritesDuties);

    /* Group 5 */
    RUN_TEST(test_TC_G5_001_Update_UsesThreeChannelsConfiguredAtInit);
    RUN_TEST(test_TC_G5_002_Update_DoesNotReinitializePwm);

    /* Group 6 */
    RUN_TEST(test_TC_G6_001_Update_PassesPercentValuesWithoutScaling);
    RUN_TEST(test_TC_G6_002_Update_MultipleUpdatesCallOncePerInvocation);

    /* Group 7 */
    RUN_TEST(test_TC_G7_001_InitAdcTrigger_WhenEgtmAlreadyEnabled_NoEnableCall);
    RUN_TEST(test_TC_G7_002_InitAdcTrigger_WhenEgtmDisabled_EnablesModule);

    /* Group 8 */
    RUN_TEST(test_TC_G8_001_InitAdcTrigger_FrequencyIs20kHz);
    RUN_TEST(test_TC_G8_002_InitAdcTrigger_NumChannelsIsOne);

    /* Group 9 */
    RUN_TEST(test_TC_G9_001_InitAdcTrigger_DoesNotUpdateDuty);
    RUN_TEST(test_TC_G9_002_InitAdcTrigger_DoesNotConfigureLedPinMode);

    return UNITY_END();
}
