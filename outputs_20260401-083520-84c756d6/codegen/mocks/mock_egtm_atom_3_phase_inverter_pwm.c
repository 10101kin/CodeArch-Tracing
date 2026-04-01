/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P MODULE_P00 = {0};
Ifx_P MODULE_P01 = {0};
Ifx_P MODULE_P02 = {0};
Ifx_P MODULE_P03 = {0};
Ifx_P MODULE_P04 = {0};
Ifx_P MODULE_P10 = {0};
Ifx_P MODULE_P13 = {0};
Ifx_P MODULE_P14 = {0};
Ifx_P MODULE_P15 = {0};
Ifx_P MODULE_P16 = {0};
Ifx_P MODULE_P20 = {0};
Ifx_P MODULE_P21 = {0};
Ifx_P MODULE_P22 = {0};
Ifx_P MODULE_P23 = {0};
Ifx_P MODULE_P25 = {0};
Ifx_P MODULE_P30 = {0};
Ifx_P MODULE_P31 = {0};
Ifx_P MODULE_P32 = {0};
Ifx_P MODULE_P33 = {0};
Ifx_P MODULE_P34 = {0};
Ifx_P MODULE_P35 = {0};
Ifx_P MODULE_P40 = {0};

/* Spy globals */
int      mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int      mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

int      mock_IfxEgtm_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_isEnabled_returnValue = FALSE;
int      mock_IfxEgtm_enable_callCount = 0;

int      mock_IfxPort_setPinModeOutput_callCount = 0;
int      mock_IfxPort_togglePin_callCount = 0;
uint32   mock_togglePin_callCount = 0;

int      mock_IfxEgtm_Pwm_initConfig_callCount = 0;
uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

int      mock_IfxEgtm_Pwm_init_callCount = 0;
uint32   mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;

int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int      mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
boolean  mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

int      mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int      mock_IfxCpu_enableInterrupts_callCount = 0;

/* Internal captured channel count for bounded copies */
static uint32 _captured_numChannels = 0;

/* Stub bodies */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    /* Default to 100 MHz if test hasn't set a value */
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    else
    {
        return 100000000.0f;
    }
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

void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int priority)
{
    (void)handler; (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

void IfxCpu_enableInterrupts(void)
{
    mock_IfxCpu_enableInterrupts_callCount++;
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

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *duties)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = duties ? duties[i] : 0.0f;
    }
}

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}

/* Mock control API impls */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0U;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;

    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount = 0;

    _captured_numChannels = 0U;
}

/* Getters */
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }

int  mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int  mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }

int  mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int  mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }

int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int  mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void) { return mock_IfxEgtm_Trigger_trigToAdc_callCount; }

int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int  mock_IfxCpu_enableInterrupts_getCallCount(void) { return mock_IfxCpu_enableInterrupts_callCount; }
