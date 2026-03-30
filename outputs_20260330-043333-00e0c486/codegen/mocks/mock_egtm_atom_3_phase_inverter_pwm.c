/* mock_egtm_atom_3_phase_inverter_pwm.c - Spy state + stub bodies + MODULE instances */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* MODULE_* instance definitions */
Ifx_EGTM MODULE_EGTM = { 0u };
Ifx_P    MODULE_P20  = { 0u };
Ifx_P    MODULE_P21  = { 0u };

/* Spy globals: counters and return-value controls */
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_enable_callCount = 0;

int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

int     mock_IfxPort_togglePin_callCount = 0;
int     mock_togglePin_callCount = 0;
int     mock_IfxPort_setPinModeOutput_callCount = 0;

/* Spy capture state */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Internal bounded-copy helper: number of channels captured from last init */
static uint32 _captured_numChannels = 0u;

/* ====== Stub bodies ====== */

/* IfxEgtm base enable/status */
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

/* CMU functions */
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
    /* Provide a reasonable default if test didn't override */
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* 100 MHz default */
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

/* PWM driver */
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
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty != NULL_PTR)
    {
        uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
                    ? _captured_numChannels
                    : (uint32)MOCK_MAX_CHANNELS;
        for (uint32 i = 0u; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
        for (uint32 i = n; i < (uint32)MOCK_MAX_CHANNELS; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        }
    }
}

/* PORT driver */
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

/* ====== Spy getters ====== */
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }

int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }

/* ====== Reset API ====== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0u; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    _captured_numChannels = 0u;
}
