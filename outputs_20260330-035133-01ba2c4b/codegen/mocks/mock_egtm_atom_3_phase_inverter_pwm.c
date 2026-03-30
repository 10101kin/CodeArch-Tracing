/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* MODULE_* instance definitions */
Ifx_EGTM MODULE_EGTM = {0u};
Ifx_P MODULE_P00 = {0u};
Ifx_P MODULE_P01 = {0u};
Ifx_P MODULE_P02 = {0u};
Ifx_P MODULE_P03 = {0u};
Ifx_P MODULE_P04 = {0u};
Ifx_P MODULE_P10 = {0u};
Ifx_P MODULE_P13 = {0u};
Ifx_P MODULE_P14 = {0u};
Ifx_P MODULE_P15 = {0u};
Ifx_P MODULE_P16 = {0u};
Ifx_P MODULE_P20 = {0u};
Ifx_P MODULE_P21 = {0u};
Ifx_P MODULE_P22 = {0u};
Ifx_P MODULE_P23 = {0u};
Ifx_P MODULE_P25 = {0u};
Ifx_P MODULE_P30 = {0u};
Ifx_P MODULE_P31 = {0u};
Ifx_P MODULE_P32 = {0u};
Ifx_P MODULE_P33 = {0u};
Ifx_P MODULE_P34 = {0u};
Ifx_P MODULE_P35 = {0u};
Ifx_P MODULE_P40 = {0u};

/* Spy state definitions */
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Pwm_init_callCount = 0;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};

float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int mock_IfxPort_setPinModeOutput_callCount = 0;
int mock_IfxPort_togglePin_callCount = 0;
uint32 mock_togglePin_callCount = 0u;

int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_enable_callCount = 0;

int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

/* Internal capture for bounded duty copy */
static uint32 _captured_numChannels = 0u;

/* Stub bodies */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels
               : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
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
    (void)pwm;
    (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
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

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* sensible default: 100 MHz */
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, float32 frequency)
{
    (void)egtm; (void)clk; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{
    (void)egtm; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
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

/* Getters */
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }

int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }

int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }

int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }

/* Reset */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    {
        uint32 i;
        for (i = 0u; i < (uint32)MOCK_MAX_CHANNELS; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
            mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
            mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
        }
    }

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

    _captured_numChannels = 0u;
}
