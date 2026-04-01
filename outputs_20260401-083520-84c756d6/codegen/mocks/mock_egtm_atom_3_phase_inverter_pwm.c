#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P33  = {0};

/* Pin symbol instances */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT = {0};

/* Spy counters and return controls */
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_enable_callCount = 0;

int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

int     mock_IfxPort_setPinModeOutput_callCount = 0;
uint32  mock_togglePin_callCount = 0;

int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
int     mock_IfxEgtm_Pwm_init_callCount = 0;
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;

int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
/* Backward-compatible aliases (singular) */
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

/* Internal captured numChannels for bounded copies */
static uint32 _captured_numChannels = 0;

/* Getters */
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount; }
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void) { return mock_IfxEgtm_Trigger_trigToAdc_callCount; }

/* Reset all spies and controls */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;

    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;

    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

    _captured_numChannels = 0;
}

/* Stub bodies */
/* IfxEgtm */
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

/* IfxEgtm_Cmu mandatory functions */
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

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *module)
{
    (void)module;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    /* default 100 MHz if test didn't preset a value */
    return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
           ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue
           : 100000000.0f;
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *module, float32 frequency)
{
    (void)module; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency)
{
    (void)module; (void)clk; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *module, uint32 mask)
{
    (void)module; (void)mask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

/* IfxPort */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_togglePin_callCount++;
}

/* IfxEgtm_Pwm */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *duties)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= MOCK_MAX_CHANNELS)
               ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (duties != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = duties[i];
        }
    }
}

void IfxEgtm_Pwm_updateChannelsDeadTimeImmediate(IfxEgtm_Pwm *pwm, float32 *dtRising, float32 *dtFalling)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= MOCK_MAX_CHANNELS)
               ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (dtRising != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = dtRising[i];
            mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = dtRising[i];
        }
    }
    if (dtFalling != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = dtFalling[i];
            mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = dtFalling[i];
        }
    }
}

/* IfxEgtm_Trigger */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}
