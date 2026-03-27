/*
 * mock_egtm_atom_3_phase_inverter_pwm.c
 * Spy state + stub bodies + MODULE_* definitions
 */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ================= Spy state definitions ================= */
int     mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
int     mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxEgtm_enable_callCount = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxPort_setPinModeOutput_callCount = 0;
uint32  mock_togglePin_callCount = 0;

/* ================= MODULE_* instance definitions ================= */
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
Ifx_P MODULE_P41 = {0u};

Ifx_WTU MODULE_WTU = {0u};
Ifx_SRC MODULE_SRC = {0u};

/* ================= Pin symbol definitions (EGTM ATOM TOUTs) ================= */
/* Provide the symbols reported missing in previous build errors */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT33_P33_11_OUT = {0};

/* ================= Stub bodies ================= */
/* ---- IfxEgtm_Cmu ---- */
void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount++;
}

void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)clkIndex; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setEclkDivider_callCount++;
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

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

/* ---- IfxEgtm_Pwm ---- */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
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
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty != NULL_PTR)
    {
        /* Capture up to MOCK_MAX_CHANNELS values */
        for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

/* ---- IfxEgtm core ---- */
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

/* ---- IfxPort ---- */
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

/* ================= Spy getters ================= */
int mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void)      { return mock_IfxEgtm_Cmu_setGclkDivider_callCount; }
int mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(void)      { return mock_IfxEgtm_Cmu_setEclkDivider_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void)  { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void)        { return mock_IfxEgtm_Cmu_enableClocks_callCount; }

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void)          { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void)                { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int mock_IfxEgtm_enable_getCallCount(void)                  { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void)               { return mock_IfxEgtm_isEnabled_callCount; }

int mock_IfxPort_setPinModeOutput_getCallCount(void)        { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void)               { return (int)mock_togglePin_callCount; }

/* ================= Reset ================= */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0u;
}
