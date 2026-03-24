#include "unity.h"
#include "egtm_atom_3ph_pwm.h"

// Mock headers providing control functions for TC4xx target APIs
#include "IfxAdc_Mock.h"
#include "IfxAdc_Tmadc_Mock.h"
#include "IfxPort_Mock.h"

void setUp(void)
{
    // Reset all relevant driver mocks (void, no arguments)
    IfxAdc_Mock_Reset();
    IfxAdc_Tmadc_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Verify init calls the expected TC4xx driver APIs (ADC TMADC in polled mode, LED GPIO)
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Act
    initEgtmAtom3phInv();

    // Assert: ADC enable + TMADC configuration and run (polled)
    TEST_ASSERT_TRUE(IfxAdc_Mock_GetCallCount_enableModule() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModule() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannel() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_runModule() > 0u);

    // Assert: LED GPIO configured as push-pull output and initial state set
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0u);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0u);
}

// 2) Migration target API usage smoke test: ensure only TC4xx-target mocks are exercised
//    (Do not reference any TC3xx APIs; verify key TC4xx paths are taken.)
void test_initEgtmAtom3phInv_TargetAPIs_AreUsed_ForTc4xx(void)
{
    // Act
    initEgtmAtom3phInv();

    // Assert: TMADC (TC4xx) path was used and driven
    TEST_ASSERT_TRUE(IfxAdc_Mock_GetCallCount_enableModule() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModule() > 0u);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_runModule() > 0u);

    // Assert: LED GPIO configured using IfxPort (TC4xx-compatible) APIs
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0u);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0u);
}
