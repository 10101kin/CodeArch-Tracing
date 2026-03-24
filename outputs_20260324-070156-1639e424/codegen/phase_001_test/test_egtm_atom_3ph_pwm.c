#include "unity.h"
#include "egtm_atom_3ph_pwm.h"
#include "IfxAdc.h"
#include "IfxPort.h"

void setUp(void)
{
    // Reset all involved driver mocks (void functions; no arguments)
    IfxAdc_Mock_Reset();
    IfxAdc_Tmadc_Mock_Reset();
    IfxPort_Mock_Reset();
}

void tearDown(void) {}

// 1) Verify that init uses the TC4xx-targeted APIs for TMADC and Port
void test_initEgtmAtom3phInv_CallsExpectedDriverAPIs(void)
{
    // Act
    initEgtmAtom3phInv();   // exact signature: void initEgtmAtom3phInv(void)

    // Assert — ADC TMADC enable/config/init/channel/run sequence
    TEST_ASSERT_TRUE(IfxAdc_Mock_GetCallCount_enableModule() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initModule() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_initChannel() > 0);
    TEST_ASSERT_TRUE(IfxAdc_Tmadc_Mock_GetCallCount_runModule() > 0);

    // Assert — LED GPIO configuration and initial state
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0);
}
