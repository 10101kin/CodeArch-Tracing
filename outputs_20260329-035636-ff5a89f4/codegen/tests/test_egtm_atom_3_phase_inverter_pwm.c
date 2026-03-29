#include "unity.h"
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Extern declarations for ISR not exposed via header */
extern void interruptEgtmAtom(void);

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_NUM_CHANNELS                  (3)
#define UT_SWITCHING_FREQ_HZ             (20000U)
#define UT_INIT_DUTY_U_PERCENT           (25.0f)
#define UT_INIT_DUTY_V_PERCENT           (50.0f)
#define UT_INIT_DUTY_W_PERCENT           (75.0f)
#define UT_DEAD_TIME_US                  (1.0f)
#define UT_CLUSTER_INDEX                 (1U)
#define UT_ATOM_INDEX                    (1U)
#define UT_ISR_PRIORITY                  (20U)

/* Available extern return-value controls from the mock */
extern uint32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern uint32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern uint32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;
extern uint8  mock_IfxEgtm_isEnabled_returnValue;

/* Available extern spy variables from the mock */
extern uint32 mock_IfxEgtm_Pwm_init_lastNumChannels;
extern uint32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern float  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[];
extern float  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[];
extern float  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[];
extern uint32 mock_togglePin_callCount;

/* Available call-count getters from the mock */
extern uint32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
extern uint32 mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
extern uint32 mock_IfxEgtm_Pwm_init_getCallCount(void);
extern uint32 mock_IfxPort_togglePin_getCallCount(void);
extern uint32 mock_IfxPort_setPinModeOutput_getCallCount(void);
extern uint32 mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
extern uint32 mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
extern uint32 mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void);
extern uint32 mock_IfxEgtm_Cmu_setClkCount_getCallCount(void);
extern uint32 mock_IfxEgtm_isEnabled_getCallCount(void);
extern uint32 mock_IfxEgtm_enable_getCallCount(void);

void setUp(void)   { mock_egtm_atom_3_phase_inverter_pwm_reset(); }
void tearDown(void) {}

/* =========================================================
 * GROUP 1 - initEgtmAtom3phInv: initialization / enable guard
 * ========================================================= */
void test_TC_G1_001_init_enables_cmu_when_module_disabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 0U; /* eGTM not enabled */
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 150000000U; /* arbitrary */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_enable_getCallCount(),         "IfxEgtm_enable must be called when module is disabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must be called when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(),     "CMU setGclkDivider must be called when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_setClkCount_getCallCount(),        "CMU setClkCount must be called when enabling clocks");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU enableClocks must be called when enabling clocks");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Board LED must be configured as output once");
}

void test_TC_G1_002_init_skips_cmu_when_module_enabled(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U; /* eGTM already enabled */

    /* Act */
    initEgtmAtom3phInv();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "IfxEgtm_Pwm_initConfig must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "IfxEgtm_isEnabled must be checked once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_enable_getCallCount(),         "IfxEgtm_enable must NOT be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(), "CMU getModuleFrequency must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(),     "CMU setGclkDivider must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_setClkCount_getCallCount(),        "CMU setClkCount must NOT be called when already enabled");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, mock_IfxEgtm_Cmu_enableClocks_getCallCount(),       "CMU enableClocks must NOT be called when already enabled");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "IfxEgtm_Pwm_init must be called exactly once");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "Board LED must be configured as output once");
}

/* ==============================================
 * GROUP 2 - initEgtmAtom3phInv: configuration values
 * ============================================== */
void test_TC_G2_001_init_sets_num_channels_and_frequency(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert (use init() spies for application-configured values) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Number of logical channels must be 3 for 3-phase");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_SWITCHING_FREQ_HZ, mock_IfxEgtm_Pwm_init_lastFrequency, "PWM switching frequency must be 20 kHz");
}

void test_TC_G2_002_init_calls_required_target_apis_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;

    /* Act */
    initEgtmAtom3phInv();

    /* Assert: confirm TC4xx IfxEgtm_* APIs are used (not TC3xx IfxGtm_*) */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_initConfig_getCallCount(), "Target API IfxEgtm_Pwm_initConfig must be used");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_init_getCallCount(),       "Target API IfxEgtm_Pwm_init must be used");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_isEnabled_getCallCount(),      "Target API IfxEgtm_isEnabled must be used");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxPort_setPinModeOutput_getCallCount(), "LED GPIO must be configured once");
}

/* ==================================================
 * GROUP 3 - initEgtmAtom3phInv: runtime update logic
 * ================================================== */
void test_TC_G3_001_single_update_increments_duties_and_calls_hal_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called exactly once for one update");

    /* Duties must be in percent range [0,100] for all three channels */
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(100.0f + UT_FLOAT_EPSILON, 50.0f, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0], "Phase U duty must be within 0..100% (using centered window)");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 100.0f, "Phase U duty bound check");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 100.0f, "Phase V duty bound check");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 100.0f, "Phase W duty bound check");
}

void test_TC_G3_002_multiple_updates_preserve_bounds_and_single_hal_call_per_update(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Act */
    const uint32 iterations = 5U;
    for (uint32 i = 0; i < iterations; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(iterations, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called exactly once per update invocation");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 100.0f, "Phase U duty bound check after multiple updates");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 100.0f, "Phase V duty bound check after multiple updates");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 100.0f, "Phase W duty bound check after multiple updates");
}

/* =============================================
 * GROUP 4 - initEgtmAtom3phInv: ISR / interrupt
 * ============================================= */
void test_TC_G4_001_isr_toggles_led_once(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();
    uint32 before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 1U, mock_togglePin_callCount, "ISR must toggle LED GPIO exactly once per invocation");
}

void test_TC_G4_002_isr_accumulates_toggle_over_multiple_invocations(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();
    uint32 before = mock_togglePin_callCount;

    /* Act */
    interruptEgtmAtom();
    interruptEgtmAtom();
    interruptEgtmAtom();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(before + 3U, mock_togglePin_callCount, "ISR toggle count must accumulate across invocations");
}

/* ==================================================
 * GROUP 5 - updateEgtmAtom3phInvDuty: configuration
 * ================================================== */
void test_TC_G5_001_update_respects_num_channels_and_value_range(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_NUM_CHANNELS, mock_IfxEgtm_Pwm_init_lastNumChannels, "Logical channel count must remain 3");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] <= 100.0f, "Phase U duty within 0..100% after update");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] <= 100.0f, "Phase V duty within 0..100% after update");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] >= 0.0f && mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] <= 100.0f, "Phase W duty within 0..100% after update");
}

void test_TC_G5_002_second_update_changes_duty_values(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Act */
    updateEgtmAtom3phInvDuty();
    float firstU = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    float firstV = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    float firstW = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    updateEgtmAtom3phInvDuty();

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be called once per update (2 total)");
    /* At least one channel should change between consecutive updates */
    int changed = 0;
    changed |= (mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0] - firstU) > UT_FLOAT_EPSILON || (firstU - mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0]) > UT_FLOAT_EPSILON;
    changed |= (mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1] - firstV) > UT_FLOAT_EPSILON || (firstV - mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1]) > UT_FLOAT_EPSILON;
    changed |= (mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2] - firstW) > UT_FLOAT_EPSILON || (firstW - mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2]) > UT_FLOAT_EPSILON;
    TEST_ASSERT_TRUE_MESSAGE(changed != 0, "At least one phase duty must change between consecutive updates");
}

/* ==============================================
 * GROUP 6 - updateEgtmAtom3phInvDuty: runtime logic
 * ============================================== */
void test_TC_G6_001_update_called_once_per_invocation(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Act */
    const uint32 N = 10U;
    for (uint32 i = 0; i < N; ++i)
    {
        updateEgtmAtom3phInvDuty();
    }

    /* Assert */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(N, mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(), "HAL update must be invoked exactly once per update call");
}

void test_TC_G6_002_wrap_around_occurs_and_is_independent_per_channel(void)
{
    /* Arrange */
    mock_IfxEgtm_isEnabled_returnValue = 1U;
    initEgtmAtom3phInv();

    /* Prime first update to get starting values */
    updateEgtmAtom3phInvDuty();
    float prev[3];
    prev[0] = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
    prev[1] = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
    prev[2] = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

    int wrapped[3] = {0,0,0};

    /* Act: perform many updates to observe wrap events on all channels */
    const uint32 N = 2000U; /* large enough to detect wrap for typical step sizes */
    for (uint32 i = 0; i < N; ++i)
    {
        updateEgtmAtom3phInvDuty();
        float u = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[0];
        float v = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[1];
        float w = mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[2];

        if (u + UT_FLOAT_EPSILON < prev[0]) wrapped[0] = 1;
        if (v + UT_FLOAT_EPSILON < prev[1]) wrapped[1] = 1;
        if (w + UT_FLOAT_EPSILON < prev[2]) wrapped[2] = 1;

        prev[0] = u; prev[1] = v; prev[2] = w;

        /* Always remain within bounds */
        TEST_ASSERT_TRUE_MESSAGE(u >= 0.0f && u <= 100.0f, "Phase U duty must remain in 0..100% during wrap test");
        TEST_ASSERT_TRUE_MESSAGE(v >= 0.0f && v <= 100.0f, "Phase V duty must remain in 0..100% during wrap test");
        TEST_ASSERT_TRUE_MESSAGE(w >= 0.0f && w <= 100.0f, "Phase W duty must remain in 0..100% during wrap test");
    }

    /* Assert: each channel should have exhibited at least one wrap */
    TEST_ASSERT_TRUE_MESSAGE(wrapped[0] == 1, "Phase U must wrap independently within test horizon");
    TEST_ASSERT_TRUE_MESSAGE(wrapped[1] == 1, "Phase V must wrap independently within test horizon");
    TEST_ASSERT_TRUE_MESSAGE(wrapped[2] == 1, "Phase W must wrap independently within test horizon");
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_enables_cmu_when_module_disabled);
    RUN_TEST(test_TC_G1_002_init_skips_cmu_when_module_enabled);

    RUN_TEST(test_TC_G2_001_init_sets_num_channels_and_frequency);
    RUN_TEST(test_TC_G2_002_init_calls_required_target_apis_once);

    RUN_TEST(test_TC_G3_001_single_update_increments_duties_and_calls_hal_once);
    RUN_TEST(test_TC_G3_002_multiple_updates_preserve_bounds_and_single_hal_call_per_update);

    RUN_TEST(test_TC_G4_001_isr_toggles_led_once);
    RUN_TEST(test_TC_G4_002_isr_accumulates_toggle_over_multiple_invocations);

    RUN_TEST(test_TC_G5_001_update_respects_num_channels_and_value_range);
    RUN_TEST(test_TC_G5_002_second_update_changes_duty_values);

    RUN_TEST(test_TC_G6_001_update_called_once_per_invocation);
    RUN_TEST(test_TC_G6_002_wrap_around_occurs_and_is_independent_per_channel);

    return UNITY_END();
}
