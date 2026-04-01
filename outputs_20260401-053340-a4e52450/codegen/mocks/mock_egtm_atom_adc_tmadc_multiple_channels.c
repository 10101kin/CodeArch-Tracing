/* Spy state + stub bodies + MODULE_* instances */
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_ADC  MODULE_ADC  = {0};
Ifx_P    MODULE_P02  = {0};
Ifx_P    MODULE_P33  = {0};

/* Pin symbol storage (must exist) */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT = {0};

/* Spy state */
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_enable_callCount = 0;

int mock_IfxAdc_clearTmadcResultFlag_callCount = 0;
int mock_IfxAdc_enableModule_callCount = 0;
int     mock_IfxAdc_isTmadcResultAvailable_callCount = 0;
boolean mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

int mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

int mock_IfxPort_togglePin_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;
uint32 mock_togglePin_callCount = 0;

int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int mock_IfxAdc_Tmadc_runModule_callCount = 0;

/* Internal bounded-copy tracker */
static uint32 _captured_numChannels = 0;

/* Getters */
int mock_IfxEgtm_isEnabled_getCallCount(void){ return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void){ return mock_IfxEgtm_enable_callCount; }
int mock_IfxAdc_clearTmadcResultFlag_getCallCount(void){ return mock_IfxAdc_clearTmadcResultFlag_callCount; }
int mock_IfxAdc_enableModule_getCallCount(void){ return mock_IfxAdc_enableModule_callCount; }
int mock_IfxAdc_isTmadcResultAvailable_getCallCount(void){ return mock_IfxAdc_isTmadcResultAvailable_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void){ return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void){ return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void){ return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void){ return mock_IfxEgtm_Trigger_trigToAdc_callCount; }
int mock_IfxPort_togglePin_getCallCount(void){ return mock_IfxPort_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void){ return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void){ return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void){ return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void){ return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxAdc_Tmadc_runModule_getCallCount(void){ return mock_IfxAdc_Tmadc_runModule_callCount; }

/* Reset */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void)
{
    mock_IfxEgtm_isEnabled_callCount = 0; mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxAdc_clearTmadcResultFlag_callCount = 0;
    mock_IfxAdc_enableModule_callCount = 0;
    mock_IfxAdc_isTmadcResultAvailable_callCount = 0; mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;

    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0; mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0; mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0; mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0; mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

    mock_IfxPort_togglePin_callCount = 0; mock_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0; mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0; mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i=0;i<MOCK_MAX_CHANNELS;++i){
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    mock_IfxAdc_Tmadc_runModule_callCount = 0;

    _captured_numChannels = 0;
}

/* Stubs: IfxEgtm */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_isEnabled_callCount++; return mock_IfxEgtm_isEnabled_returnValue; }
void IfxEgtm_enable(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_enable_callCount++; }

/* Stubs: IfxAdc */
void IfxAdc_clearTmadcResultFlag(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum)
{ (void)tmadc; (void)resultRegNum; mock_IfxAdc_clearTmadcResultFlag_callCount++; }
void IfxAdc_enableModule(Ifx_ADC *modSFR)
{ (void)modSFR; mock_IfxAdc_enableModule_callCount++; }
boolean IfxAdc_isTmadcResultAvailable(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum)
{ (void)tmadc; (void)resultRegNum; mock_IfxAdc_isTmadcResultAvailable_callCount++; return mock_IfxAdc_isTmadcResultAvailable_returnValue; }

/* Stubs: IfxEgtm_Cmu */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{ (void)egtm; (void)clkMask; mock_IfxEgtm_Cmu_enableClocks_callCount++; }
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++; return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{ (void)egtm; (void)frequency; mock_IfxEgtm_Cmu_setGclkFrequency_callCount++; }
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_Cmu_getGclkFrequency_callCount++; return (mock_IfxEgtm_Cmu_getGclkFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getGclkFrequency_returnValue : 100000000.0f; }
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, float32 frequency)
{ (void)egtm; (void)clk; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_callCount++; }
void IfxEgtm_Cmu_enable(Ifx_EGTM *module)
{ (void)module; mock_IfxEgtm_Cmu_enable_callCount++; }
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module)
{ (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++; return mock_IfxEgtm_Cmu_isEnabled_returnValue; }

/* Stubs: IfxEgtm_Trigger */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{ (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal; mock_IfxEgtm_Trigger_trigToAdc_callCount++; return mock_IfxEgtm_Trigger_trigToAdc_returnValue; }

/* Stubs: IfxPort */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; mock_IfxPort_togglePin_callCount++; mock_togglePin_callCount++; }
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }

/* Stubs: IfxEgtm_Pwm */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR; mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR){
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels; mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR){
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm; mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR){
        for (uint32 i=0;i<n;++i){ mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i]; }
    }
}

/* Stubs: IfxAdc_Tmadc */
void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc)
{ (void)tmadc; mock_IfxAdc_Tmadc_runModule_callCount++; }
