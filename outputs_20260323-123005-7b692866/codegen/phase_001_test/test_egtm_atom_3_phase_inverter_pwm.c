#include "unity.h"
#include "egtm_atom_3_phase_inverter_pwm.h"

// Link-time mock control headers (only use functions from the AVAILABLE MOCK CONTROL FUNCTIONS list)
#include "IfxCpu_Irq.h"
#include "IfxStm_Timer.h"
#include "IfxGtm_Tom_Pwm.h"
#include "IfxVmt.h"

// Structured configuration values from requirements (used where capture is available)
#define TIMING_PWM_FREQUENCY_HZ        (20000U)
#define TIMING_DEADTIME_US             (1.0f)
#define TIMING_SYNC_START              (1)
#define TIMING_SYNC_UPDATE             (1)
#define CLOCK_EGTM_FXCLK0_HZ           (100000000U)
#define CLOCK_DTM_CLK0_HZ              (100000000U)
#define POLARITY_HS_ACTIVE_HIGH        (1)
#define POLARITY_LS_ACTIVE_LOW         (1)
#define POLARITY_CENTER_ALIGNED        (1)
#define ROUTING_USE_TOUTSEL            (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ (300U)
#define CLOCK_MODULE_FREQUENCY_SETTINGS_ENABLED (1)

// From user requirement: single period-event ISR on CPU0 with priority 20
#define EGTM_ATOM_ISR_PRIORITY         (20U)

void setUp(void)
{
    // Reset all available mocks before each test (no arguments)
    IfxCpu_Irq_Mock_Reset();
    IfxStm_Timer_Mock_Reset();
    IfxGtm_Tom_Pwm_Mock_Reset();
    IfxVmt_Mock_Reset();
}

void tearDown(void) {}

// 1) Init should call target IRQ API and must not call legacy CCU6/TOM init/start paths
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Act
    initEgtmAtom3phInv();

    // Assert target-side IRQ installation is invoked
    TEST_ASSERT_TRUE(IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler() > 0);

    // Migration safety: legacy CCU6 driver calls MUST NOT be used on TC4xx eGTM path
    TEST_ASSERT_EQUAL_UINT32(0, IfxVmt_Mock_GetCallCount_IfxCcu6_enableModule());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_initConfig());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_init());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_initConfig());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_init());
    TEST_ASSERT_EQUAL_UINT32(0, IfxGtm_Tom_Pwm_Mock_GetCallCount_IfxCcu6_PwmHl_start());
}

// 2) Init should configure expected values where capturable: verify ISR priority, and duty array is sane
void test_initEgtmAtom3phInv_SetsExpectedConfigValues(void)
{
    // Act
    initEgtmAtom3phInv();

    // Verify ISR priority value from user requirement (priority 20 on CPU0)
    TEST_ASSERT_TRUE(IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler() > 0);
    TEST_ASSERT_EQUAL_UINT32(EGTM_ATOM_ISR_PRIORITY, IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_prio());

    // Driver state is extern-visible; verify duty array is initialized to a valid range [0,100]
    // (Exact initial percentages depend on production design; sanity-check they are within bounds.)
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[0] >= 0.0f && g_egtmAtom3phInv.dutyCycles[0] <= 100.0f);
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[1] >= 0.0f && g_egtmAtom3phInv.dutyCycles[1] <= 100.0f);
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[2] >= 0.0f && g_egtmAtom3phInv.dutyCycles[2] <= 100.0f);
}

// 3) Single update applies one immediate step coherently across all three channels
void test_updateEgtmAtom3phInvDuty_SingleCall_UpdatesValues(void)
{
    // Arrange
    initEgtmAtom3phInv();

    // Start from known values well below wrap to focus on increment behavior
    g_egtmAtom3phInv.dutyCycles[0] = 10.0f;
    g_egtmAtom3phInv.dutyCycles[1] = 20.0f;
    g_egtmAtom3phInv.dutyCycles[2] = 30.0f;

    float before0 = g_egtmAtom3phInv.dutyCycles[0];
    float before1 = g_egtmAtom3phInv.dutyCycles[1];
    float before2 = g_egtmAtom3phInv.dutyCycles[2];

    // Act
    updateEgtmAtom3phInvDuty();

    // Assert: each channel increased by the same positive STEP, preserving per-phase independence
    float after0 = g_egtmAtom3phInv.dutyCycles[0];
    float after1 = g_egtmAtom3phInv.dutyCycles[1];
    float after2 = g_egtmAtom3phInv.dutyCycles[2];

    float step0 = after0 - before0;
    float step1 = after1 - before1;
    float step2 = after2 - before2;

    TEST_ASSERT_TRUE(step0 > 0.0f);
    TEST_ASSERT_TRUE(step1 > 0.0f);
    TEST_ASSERT_TRUE(step2 > 0.0f);

    // Coherent update: equal step applied to all channels (within small float tolerance)
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, step0, step1);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, step0, step2);

    // Values stay within 0..100 after a single step
    TEST_ASSERT_TRUE(after0 >= 0.0f && after0 <= 100.0f);
    TEST_ASSERT_TRUE(after1 >= 0.0f && after1 <= 100.0f);
    TEST_ASSERT_TRUE(after2 >= 0.0f && after2 <= 100.0f);
}

// 4) Multiple updates progress linearly by N*STEP (no gating/timing)
void test_updateEgtmAtom3phInvDuty_MultipleCalls_ProgressesCorrectly(void)
{
    // Arrange
    initEgtmAtom3phInv();

    g_egtmAtom3phInv.dutyCycles[0] = 5.0f;
    g_egtmAtom3phInv.dutyCycles[1] = 15.0f;
    g_egtmAtom3phInv.dutyCycles[2] = 25.0f;

    float start0 = g_egtmAtom3phInv.dutyCycles[0];
    float start1 = g_egtmAtom3phInv.dutyCycles[1];
    float start2 = g_egtmAtom3phInv.dutyCycles[2];

    // Act: two consecutive updates
    updateEgtmAtom3phInvDuty();
    float after1_0 = g_egtmAtom3phInv.dutyCycles[0];
    float after1_1 = g_egtmAtom3phInv.dutyCycles[1];
    float after1_2 = g_egtmAtom3phInv.dutyCycles[2];

    updateEgtmAtom3phInvDuty();
    float after2_0 = g_egtmAtom3phInv.dutyCycles[0];
    float after2_1 = g_egtmAtom3phInv.dutyCycles[1];
    float after2_2 = g_egtmAtom3phInv.dutyCycles[2];

    // Assert linear progression: delta after two calls ≈ 2x delta after first call
    float d1_0 = after1_0 - start0;
    float d1_1 = after1_1 - start1;
    float d1_2 = after1_2 - start2;

    float d2_0 = after2_0 - start0;
    float d2_1 = after2_1 - start1;
    float d2_2 = after2_2 - start2;

    TEST_ASSERT_TRUE(d1_0 > 0.0f && d1_1 > 0.0f && d1_2 > 0.0f);

    TEST_ASSERT_FLOAT_WITHIN(0.0002f, 2.0f * d1_0, d2_0);
    TEST_ASSERT_FLOAT_WITHIN(0.0002f, 2.0f * d1_1, d2_1);
    TEST_ASSERT_FLOAT_WITHIN(0.0002f, 2.0f * d1_2, d2_2);
}

// 5) Boundary/edge: values near or at 100% wrap to 0 before applying STEP
void test_updateEgtmAtom3phInvDuty_BoundaryWrapAround(void)
{
    // Arrange
    initEgtmAtom3phInv();

    // Force edge case: start at 100% so (duty + STEP) >= 100 triggers wrap then increment
    g_egtmAtom3phInv.dutyCycles[0] = 100.0f;
    g_egtmAtom3phInv.dutyCycles[1] = 100.0f;
    g_egtmAtom3phInv.dutyCycles[2] = 100.0f;

    // Act
    updateEgtmAtom3phInvDuty();

    // Assert: wrapped to low range (>0 because STEP added) and definitely lower than 100
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[0] > 0.0f && g_egtmAtom3phInv.dutyCycles[0] < 100.0f);
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[1] > 0.0f && g_egtmAtom3phInv.dutyCycles[1] < 100.0f);
    TEST_ASSERT_TRUE(g_egtmAtom3phInv.dutyCycles[2] > 0.0f && g_egtmAtom3phInv.dutyCycles[2] < 100.0f);

    // Coherent step still equal across channels after wrap
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, g_egtmAtom3phInv.dutyCycles[0], g_egtmAtom3phInv.dutyCycles[1]);
    TEST_ASSERT_FLOAT_WITHIN(0.0001f, g_egtmAtom3phInv.dutyCycles[0], g_egtmAtom3phInv.dutyCycles[2]);
}

// 6) Update must not re-run initialization paths (no extra IRQ installs; no legacy CCU6 calls)
void test_updateEgtmAtom3phInvDuty_DoesNotReInit(void)
{
    // Arrange
    initEgtmAtom3phInv();
    uint32 irqInstallsAfterInit = IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler();

    // Act
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();
    updateEgtmAtom3phInvDuty();

    // Assert: no additional IRQ handler installations occurred
    TEST_ASSERT_EQUAL_UINT32(irqInstallsAfterInit, IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler());

    // Migration safety: legacy CCU6 calls must remain unused during runtime as well
    TEST_ASSERT_EQUAL_UINT32(0, IfxVmt_Mock_GetCallCount_IfxCcu6_enableModule());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_initConfig());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_init());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_initConfig());
    TEST_ASSERT_EQUAL_UINT32(0, IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_init());
    TEST_ASSERT_EQUAL_UINT32(0, IfxGtm_Tom_Pwm_Mock_GetCallCount_IfxCcu6_PwmHl_start());
}
