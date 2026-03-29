#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"

/* MODULE_* instance definitions (define frequently used ones) */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P MODULE_P00 = {0};
Ifx_P MODULE_P02 = {0};
Ifx_P MODULE_P10 = {0};
Ifx_P MODULE_P13 = {0};
Ifx_P MODULE_P14 = {0};
Ifx_P MODULE_P15 = {0};
Ifx_P MODULE_P20 = {0};
Ifx_P MODULE_P21 = {0};
Ifx_P MODULE_P22 = {0};
Ifx_P MODULE_P23 = {0};
Ifx_P MODULE_P33 = {0};
Ifx_P MODULE_P34 = {0};
Ifx_P MODULE_P40 = {0};
Ifx_P MODULE_P41 = {0};

/* Spy globals */
int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int      mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int      mock_IfxEgtm_Pwm_init_callCount = 0;
uint32   mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int      mock_IfxPort_togglePin_callCount = 0;
int      mock_IfxPort_setPinModeOutput_callCount = 0;
uint32   mock_togglePin_callCount = 0; /* compatibility alias */

int      mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
int      mock_IfxEgtm_Cmu_setClkCount_callCount = 0;

int      mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int      mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

int      mock_IfxEgtm_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_isEnabled_returnValue = FALSE;
int      mock_IfxEgtm_enable_callCount = 0;

/* Internal capture of requested number of channels for bounded copies */
static uint32 _captured_numChannels = 0;

/* Stub implementations */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    /* bounded copy using last captured numChannels */
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS)
               ? _captured_numChannels : MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty ? requestDuty[i] : 0.0f;
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

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
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
    return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
}

void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount++;
}

void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count)
{
    (void)egtm; (void)clkIndex; (void)count;
    mock_IfxEgtm_Cmu_setClkCount_callCount++;
}

/* Additional stubs to satisfy previous API usage */
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount++;
    return mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
}

float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled)
{
    (void)egtm; (void)clkIndex; (void)assumeEnabled;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount++;
    return mock_IfxEgtm_Cmu_getClkFrequency_returnValue;
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

/* Getters */
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int  mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkDivider_callCount; }
int  mock_IfxEgtm_Cmu_setClkCount_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkCount_callCount; }
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int  mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getClkFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int  mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int  mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }

/* Reset function */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;

    mock_IfxPort_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_setClkCount_callCount = 0;

    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;
}
