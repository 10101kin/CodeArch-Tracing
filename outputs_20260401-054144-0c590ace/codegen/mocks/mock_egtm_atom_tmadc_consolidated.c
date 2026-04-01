#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxAdc_Tmadc.h"
#include "IfxAdc.h"
#include "IfxCpu.h"
#include "IfxPort_Pinmap.h"

/* MODULE_* instance definitions */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_ADC  MODULE_ADC  = {0};
Ifx_P    MODULE_P02  = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P33  = {0};

/* Pin symbol storage */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT  = {0};

/* Spy storage */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS]     = {0};
float32 mock_IfxEgtm_Pwm_update_lastDtRising[MOCK_MAX_CHANNELS]   = {0};
float32 mock_IfxEgtm_Pwm_update_lastDtFalling[MOCK_MAX_CHANNELS]  = {0};
uint32  mock_togglePin_callCount = 0U;

/* Internal capture for bounded copies */
static uint32 _captured_numChannels = 0U;

/* Call counters */
int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_enable_callCount = 0;
int mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
int mock_IfxEgtm_enable_callCount = 0;
int mock_IfxEgtm_isEnabled_callCount = 0;
int mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
int mock_IfxAdc_Tmadc_initModule_callCount = 0;
int mock_IfxAdc_Tmadc_initModuleConfig_callCount = 0;
int mock_IfxAdc_enableModule_callCount = 0;
int mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
int mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
int mock_IfxEgtm_Atom_Timer_run_callCount = 0;
int mock_IfxEgtm_PinMap_setAtomTout_callCount = 0;
int mock_IfxCpu_enableInterrupts_callCount = 0;
int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;

/* Return controls */
boolean mock_IfxEgtm_Pwm_init_returnValue = FALSE;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue   = 0.0f;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;
boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;

/* Stubs */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}

float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getGclkFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
    }
    return 100000000.0f;
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{
    (void)egtm; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)egtm; (void)clkIndex; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

void IfxEgtm_Cmu_enable(Ifx_EGTM *module)
{
    (void)module;
    mock_IfxEgtm_Cmu_enable_callCount++;
}

boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module)
{
    (void)module;
    mock_IfxEgtm_Cmu_isEnabled_callCount++;
    return mock_IfxEgtm_Cmu_isEnabled_returnValue;
}

void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_enable_callCount++;
}

boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}

void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config)
{
    (void)tmadc; (void)config;
    mock_IfxAdc_Tmadc_initModule_callCount++;
}

void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc)
{
    (void)config; (void)adc;
    mock_IfxAdc_Tmadc_initModuleConfig_callCount++;
}

void IfxAdc_enableModule(Ifx_ADC *modSFR)
{
    (void)modSFR;
    mock_IfxAdc_enableModule_callCount++;
}

boolean IfxEgtm_Atom_Timer_setFrequency(IfxEgtm_Atom_Timer *driver, float32 frequency)
{
    (void)driver; (void)frequency;
    mock_IfxEgtm_Atom_Timer_setFrequency_callCount++;
    return mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
}

void IfxEgtm_Atom_Timer_setTrigger(IfxEgtm_Atom_Timer *driver, uint32 triggerPoint)
{
    (void)driver; (void)triggerPoint;
    mock_IfxEgtm_Atom_Timer_setTrigger_callCount++;
}

void IfxEgtm_Atom_Timer_run(IfxEgtm_Atom_Timer *driver)
{
    (void)driver;
    mock_IfxEgtm_Atom_Timer_run_callCount++;
}

void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config; (void)outputMode; (void)padDriver;
    mock_IfxEgtm_PinMap_setAtomTout_callCount++;
}

void IfxCpu_enableInterrupts(void)
{
    mock_IfxCpu_enableInterrupts_callCount++;
}

void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), Ifx_Priority priority)
{
    (void)isr; (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

/* IfxPort basic stubs (no counters required) */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver)
{ (void)port; (void)pinIndex; (void)driver; }
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, uint8 altIdx)
{ (void)port; (void)pinIndex; (void)altIdx; }

/* Getters */
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void) { return mock_IfxEgtm_Trigger_trigToAdc_callCount; }
int mock_IfxAdc_Tmadc_initModule_getCallCount(void) { return mock_IfxAdc_Tmadc_initModule_callCount; }
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void) { return mock_IfxAdc_Tmadc_initModuleConfig_callCount; }
int mock_IfxAdc_enableModule_getCallCount(void) { return mock_IfxAdc_enableModule_callCount; }
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_setFrequency_callCount; }
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_setTrigger_callCount; }
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_run_callCount; }
int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void) { return mock_IfxEgtm_PinMap_setAtomTout_callCount; }
int mock_IfxCpu_enableInterrupts_getCallCount(void) { return mock_IfxCpu_enableInterrupts_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }

/* Reset */
void mock_egtm_atom_tmadc_consolidated_reset(void)
{
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
    for (uint32 i = 0U; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_update_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_update_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_update_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0U;
    mock_togglePin_callCount = 0U;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxAdc_Tmadc_initModule_callCount = 0;
    mock_IfxAdc_Tmadc_initModuleConfig_callCount = 0;
    mock_IfxAdc_enableModule_callCount = 0;
    mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
    mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
    mock_IfxEgtm_Atom_Timer_run_callCount = 0;
    mock_IfxEgtm_PinMap_setAtomTout_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount = 0;
    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;

    mock_IfxEgtm_Pwm_init_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue   = 0.0f;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;
}
