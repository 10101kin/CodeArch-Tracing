/*
 * mock_egtm_atom_3_phase_inverter_pwm.c
 * Spy state + stub bodies + MODULE_* definitions
 */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0u};
Ifx_P    MODULE_P03  = {0u};
Ifx_P    MODULE_P20  = {0u};
Ifx_P    MODULE_P21  = {0u};

/* Spy counters and return controls */
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount                  = 0;
int     mock_IfxEgtm_Pwm_init_callCount                        = 0;
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels                  = 0u;
float32 mock_IfxEgtm_Pwm_init_lastFrequency                    = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels            = 0u;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency              = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0.0f};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS]  = {0.0f};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0.0f};

int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_togglePin_callCount        = 0;
uint32  mock_togglePin_callCount                = 0u;

int     mock_IfxEgtm_isEnabled_callCount        = 0;
int     mock_IfxEgtm_enable_callCount           = 0;
boolean mock_IfxEgtm_isEnabled_returnValue      = FALSE;

int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount        = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount     = 0;
int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount    = 0;
int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount    = 0;
int     mock_IfxEgtm_Cmu_getClkFrequency_callCount     = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue   = 0.0f;
float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue    = 0.0f;

int     mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int     mock_IfxCpu_enableInterrupts_callCount            = 0;

/* Internal captured channels bound for safe duty copy */
static uint32 _captured_numChannels = 0u;

/* ===== Stub bodies ===== */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty == NULL_PTR) {
        return;
    }
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
             ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0u; i < n; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
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

float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled)
{
    (void)egtm; (void)clkIndex; (void)assumeEnabled;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getClkFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getClkFrequency_returnValue;
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

void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int vectabNum, int priority)
{
    (void)handler; (void)vectabNum; (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

void IfxCpu_enableInterrupts(void)
{
    mock_IfxCpu_enableInterrupts_callCount++;
}

/* ===== Getters ===== */
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void)                  { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void)                        { return mock_IfxEgtm_Pwm_init_callCount; }

int mock_IfxPort_setPinModeOutput_getCallCount(void)                { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void)                       { return mock_IfxPort_togglePin_callCount; }

int mock_IfxEgtm_isEnabled_getCallCount(void)                       { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void)                          { return mock_IfxEgtm_enable_callCount; }

int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void)          { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void)                { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void)             { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void)            { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void)            { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void)             { return mock_IfxEgtm_Cmu_getClkFrequency_callCount; }

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void)      { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxCpu_enableInterrupts_getCallCount(void)                 { return mock_IfxCpu_enableInterrupts_callCount; }

/* ===== Reset ===== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount                  = 0;
    mock_IfxEgtm_Pwm_init_callCount                        = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels                  = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency                    = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels            = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency              = 0.0f;
    for (uint32 i = 0u; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i]  = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0u;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount        = 0;
    mock_togglePin_callCount                = 0u;

    mock_IfxEgtm_isEnabled_callCount        = 0;
    mock_IfxEgtm_enable_callCount           = 0;
    mock_IfxEgtm_isEnabled_returnValue      = FALSE;

    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount        = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount     = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount    = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount    = 0;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount     = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue   = 0.0f;
    mock_IfxEgtm_Cmu_getClkFrequency_returnValue    = 0.0f;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount            = 0;
}
