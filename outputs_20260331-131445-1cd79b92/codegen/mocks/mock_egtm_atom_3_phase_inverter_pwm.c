/* Implementation of spies and stubs */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P02  = {0};
Ifx_P    MODULE_P33  = {0};

/* Pin symbol instances */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT  = {0};

/* Spy state */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS]  = {0};
float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0;

/* Internal capture of numChannels for bounded copies */
static uint32 _captured_numChannels = 0;

/* Counters and return-value controls */
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_enable_callCount = 0;

int     mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;

int     mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
int     mock_IfxEgtm_Atom_Timer_run_callCount = 0;

int     mock_IfxEgtm_Atom_Timer_getPeriod_callCount = 0;
uint32  mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 0u;

int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount  = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount     = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Pwm_updateChannelDutyImmediate_callCount = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_init_callCount = 0;

int     mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_togglePin_callCount = 0;

int     mock_IfxEgtm_PinMap_setAtomTout_callCount = 0;

/* Getters */
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_setFrequency_callCount; }
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_setTrigger_callCount; }
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_run_callCount; }
int mock_IfxEgtm_Atom_Timer_getPeriod_getCallCount(void) { return mock_IfxEgtm_Atom_Timer_getPeriod_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void) { return mock_IfxEgtm_Trigger_trigToAdc_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void) { return mock_IfxEgtm_PinMap_setAtomTout_callCount; }

/* Stubs - keep bodies minimal, no struct member access except config capture per rules */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}

void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_enable_callCount++;
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

uint32 IfxEgtm_Atom_Timer_getPeriod(IfxEgtm_Atom_Timer *driver)
{
    (void)driver;
    mock_IfxEgtm_Atom_Timer_getPeriod_callCount++;
    return mock_IfxEgtm_Atom_Timer_getPeriod_returnValue;
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

void IfxEgtm_Pwm_updateChannelDutyImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SyncChannelIndex configIndex, float32 requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelDutyImmediate_callCount++;
    /* bounded capture */
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    uint32 idx = (uint32)configIndex;
    if (idx < n)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[idx] = requestDuty;
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}

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

void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config; (void)outputMode; (void)padDriver;
    mock_IfxEgtm_PinMap_setAtomTout_callCount++;
}

void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;
    mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
    mock_IfxEgtm_Atom_Timer_run_callCount = 0;
    mock_IfxEgtm_Atom_Timer_getPeriod_callCount = 0;
    mock_IfxEgtm_Atom_Timer_getPeriod_returnValue = 0u;

    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Pwm_updateChannelDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;

    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_dt_lastDtRising[i] = 0.0f;
        mock_dt_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;
}
