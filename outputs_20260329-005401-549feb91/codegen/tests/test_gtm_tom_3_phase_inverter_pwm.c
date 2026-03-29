#include "unity.h"
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "gtm_tom_3_phase_inverter_pwm.h"

/* ========================= UT configuration macros ========================= */
#define UT_FLOAT_EPSILON                                 (1e-4f)

#define UT_PWM_MODE_CENTER_ALIGNED                       (1)
#define UT_NUM_PHASES                                    (3U)
#define UT_CHANNELS_PER_PHASE                            (2U)
#define UT_NUM_CHANNELS                                  (UT_NUM_PHASES * UT_CHANNELS_PER_PHASE)

#define UT_PWM_FREQUENCY_HZ                              (20000U)       /* 20 kHz switching frequency */

#define UT_GTM_SYSCLK_HZ                                 (300000000U)   /* 300 MHz system/CMU clock */
#define UT_FXCLK0_HZ                                     (100000000U)   /* 100 MHz FXCLK0 */

#define UT_DT_US                                         (0.5f)
#define UT_MIN_PULSE_US                                  (1.0f)
#define UT_DT_TICKS                                      (50U)          /* 0.5 us @ 100 MHz FXCLK0 */
#define UT_MIN_PULSE_TICKS                               (100U)         /* 1.0 us @ 100 MHz FXCLK0 */

#define UT_TIMEBASE_PERIOD_TICKS_CENTER_ALIGNED          (2500U)

/* ============================ Unity fixtures ============================== */
void setUp(void)   { mock_gtm_tom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* =============================== GROUP 1 ==================================
 * initGtmTomPwm: initialization / enable guard
 * ======================================================================== */
void test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 0;                 /* GTM disabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must query isEnabled exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_enable_getCallCount(), "GTM must be enabled when initially disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU module frequency should be read once when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK must be set exactly once inside enable guard");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK clocks must be enabled exactly once inside enable guard");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig should be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init should be called exactly once");
}

void test_TC_G1_002_init_skips_clocking_when_already_enabled(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;                 /* GTM already enabled */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_isEnabled_getCallCount(), "GTM enable guard must query isEnabled exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_enable_getCallCount(), "GTM enable must be skipped when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(), "CMU frequency read should be skipped when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(), "GCLK setup should be skipped when GTM already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxGtm_Cmu_enableClocks_getCallCount(), "FXCLK enable should be skipped when GTM already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_initConfig_getCallCount(), "IfxGtm_Pwm_initConfig must still execute once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_init_getCallCount(), "IfxGtm_Pwm_init must still execute once");
}

/* =============================== GROUP 2 ==================================
 * initGtmTomPwm: configuration values
 * ======================================================================== */
void test_TC_G2_001_config_sets_frequency_and_channel_count(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;                 /* focus on config, skip enable guard */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert: PWM switching frequency (not CMU clock) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_init_lastFrequency, "PWM driver init must request 20 kHz switching frequency");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_PWM_FREQUENCY_HZ, mock_IfxGtm_Pwm_initConfig_lastFrequency, "PWM initConfig must request 20 kHz switching frequency");

    /* Assert: number of configured channels (three complementary pairs => 6 channels) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_init_lastNumChannels, "Driver init must configure exactly 6 TOM channels (U/V/W high & low)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxGtm_Pwm_initConfig_lastNumChannels, "initConfig must reflect exactly 6 TOM channels");
}

void test_TC_G2_002_config_applies_deadtime_and_initial_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;                 /* focus on config, skip enable guard */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert: dead-time ticks applied to all channels (rising & falling) */
    for (uint32 i = 0; i < UT_NUM_CHANNELS; ++i)
    {
        char msgR[96];
        char msgF[96];
        (void)snprintf(msgR, sizeof(msgR), "Dead-time rising ticks mismatch at channel index %lu", (unsigned long)i);
        (void)snprintf(msgF, sizeof(msgF), "Dead-time falling ticks mismatch at channel index %lu", (unsigned long)i);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DT_TICKS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i], msgR);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_DT_TICKS, mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i], msgF);
    }

    /* Assert: initial duties include U/V/W = 25%/50%/75% and are normalized [0..1] */
    uint32 n = mock_IfxGtm_Pwm_init_lastNumChannels;
    uint32 found025 = 0, found050 = 0, found075 = 0;
    for (uint32 i = 0; i < n; ++i)
    {
        float d = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i];
        {
            char msgRange[96];
            (void)snprintf(msgRange, sizeof(msgRange), "Duty out of range [0..1] at index %lu", (unsigned long)i);
            TEST_ASSERT_MESSAGE((d >= 0.0f) && (d <= 1.0f), msgRange);
        }
        if (fabsf(d - 0.25f) <= UT_FLOAT_EPSILON) { found025++; }
        if (fabsf(d - 0.50f) <= UT_FLOAT_EPSILON) { found050++; }
        if (fabsf(d - 0.75f) <= UT_FLOAT_EPSILON) { found075++; }
    }
    TEST_ASSERT_MESSAGE(found025 >= 1, "Initial duty array must include 25% for phase U");
    TEST_ASSERT_MESSAGE(found050 >= 1, "Initial duty array must include 50% for phase V");
    TEST_ASSERT_MESSAGE(found075 >= 1, "Initial duty array must include 75% for phase W");
}

/* =============================== GROUP 3 ==================================
 * initGtmTomPwm: runtime update logic (within initialization sequence)
 * ======================================================================== */
void test_TC_G3_001_runtime_frequency_update_called_once_in_init(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;                 /* concentrate on update calls */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert: base/timebase frequency is applied immediately once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount(), "updateFrequencyImmediate must be called exactly once during init");
}

void test_TC_G3_002_runtime_duty_update_called_once_in_init_and_contains_phase_duties(void)
{
    /* Arrange */
    mock_IfxGtm_isEnabled_returnValue = 1;                 /* concentrate on update calls */
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = UT_GTM_SYSCLK_HZ;

    /* Act */
    initGtmTomPwm();

    /* Assert: multi-channel duty update issued exactly once */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "Duty update (immediate) must be called exactly once during init");

    /* And confirm representative three-phase duties are present */
    uint32 n = mock_IfxGtm_Pwm_init_lastNumChannels;
    uint32 found025 = 0, found050 = 0, found075 = 0;
    for (uint32 i = 0; i < n; ++i)
    {
        float d = mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i];
        if (fabsf(d - 0.25f) <= UT_FLOAT_EPSILON) { found025++; }
        if (fabsf(d - 0.50f) <= UT_FLOAT_EPSILON) { found050++; }
        if (fabsf(d - 0.75f) <= UT_FLOAT_EPSILON) { found075++; }
    }
    TEST_ASSERT_MESSAGE(found025 >= 1, "Initial duty update must include 25% duty (phase U)");
    TEST_ASSERT_MESSAGE(found050 >= 1, "Initial duty update must include 50% duty (phase V)");
    TEST_ASSERT_MESSAGE(found075 >= 1, "Initial duty update must include 75% duty (phase W)");
}

/* ================================= main =================================== */
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_GTM_and_clocks_when_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_clocking_when_already_enabled);

    RUN_TEST(test_TC_G2_001_config_sets_frequency_and_channel_count);
    RUN_TEST(test_TC_G2_002_config_applies_deadtime_and_initial_duties);

    RUN_TEST(test_TC_G3_001_runtime_frequency_update_called_once_in_init);
    RUN_TEST(test_TC_G3_002_runtime_duty_update_called_once_in_init_and_contains_phase_duties);

    return UNITY_END();
}
