#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort_Pinmap.h"

/* Spy counters and controls */
int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_togglePin_callCount = 0;
uint32  mock_togglePin_callCount = 0;

int     mock_IfxGtm_Pwm_initConfig_callCount = 0;
int     mock_IfxGtm_Pwm_init_callCount = 0;
int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

int     mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxGtm_isEnabled_callCount = 0;
int     mock_IfxGtm_enable_callCount = 0;
int     mock_IfxGtm_Cmu_enable_callCount = 0;
int     mock_IfxGtm_Cmu_isEnabled_callCount = 0;
int     mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

/* Spy capture storage */
uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0.0f};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0.0f};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0.0f};

/* Bounded duty array copy control */
static uint32 _captured_numChannels = 0;

/* MODULE instances */
Ifx_GTM MODULE_GTM = {0};
Ifx_P   MODULE_P02  = {0};
Ifx_P   MODULE_P13  = {0};

/* Required pin symbol definitions */
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0N_TOUT7_P02_7_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0_TOUT0_P02_0_OUT  = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_12_TOUT4_P02_4_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_13_TOUT5_P02_5_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1_TOUT1_P02_1_OUT  = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_5_TOUT2_P02_2_OUT  = {0};

/* Stubs */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0U && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i)
    {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
    }
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency)
{
    (void)gtm; (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* 100 MHz default */
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)gtm; (void)clkIndex; (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
}

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm; (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

boolean IfxGtm_isEnabled(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_isEnabled_callCount++;
    return mock_IfxGtm_isEnabled_returnValue;
}

void IfxGtm_enable(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_enable_callCount++;
}

void IfxGtm_Cmu_enable(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_enable_callCount++;
}

boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_isEnabled_callCount++;
    return mock_IfxGtm_Cmu_isEnabled_returnValue;
}

void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int priority)
{
    (void)isr; (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

/* Getters */
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }

/* Reset */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0U;

    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    _captured_numChannels = 0U;
}
