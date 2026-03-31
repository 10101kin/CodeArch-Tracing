#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"

/* ================= MODULE_* instances ================= */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P21  = {0};

/* ================= Spy state definitions ================= */
int     mock_IfxPort_togglePin_callCount = 0;
uint32  mock_togglePin_callCount         = 0;
int     mock_IfxPort_setPinModeOutput_callCount = 0;

int     mock_IfxEgtm_enable_callCount    = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Cmu_enable_callCount       = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount    = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue  = FALSE;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount  = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

int     mock_IfxEgtm_Pwm_init_callCount  = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;

float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_lastDtRising[MOCK_MAX_CHANNELS]      = {0};
float32 mock_IfxEgtm_Pwm_lastDtFalling[MOCK_MAX_CHANNELS]     = {0};

int     mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int     mock_IfxCpu_enableInterrupts_callCount            = 0;

/* Bounded copy count captured from last init */
static uint32 _captured_numChannels = 0;

/* =================== Stub bodies =================== */

/* IfxPort */
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

/* IfxEgtm enable/status */
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

/* IfxEgtm_Cmu */
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

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
         ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue
         : 100000000.0f; /* sensible default 100 MHz */
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{
    (void)egtm; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, float32 frequency)
{
    (void)egtm; (void)clk; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

/* IfxEgtm_Pwm primary driver */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty != NULL_PTR) {
        uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS)
                 ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_update_lastDuties[i] = requestDuty[i];
        }
        /* zero the rest to keep deterministic image */
        for (uint32 i = n; i < (uint32)MOCK_MAX_CHANNELS; ++i) {
            mock_IfxEgtm_Pwm_update_lastDuties[i] = 0.0f;
        }
    }
}

/* CPU helpers */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int vectabNum, Ifx_Priority priority)
{
    (void)isr; (void)vectabNum; (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

void IfxCpu_enableInterrupts(void)
{
    mock_IfxCpu_enableInterrupts_callCount++;
}

/* =================== Mock control (reset + getters) =================== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount         = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;

    mock_IfxEgtm_enable_callCount    = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Cmu_enable_callCount       = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount    = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue  = FALSE;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount  = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxEgtm_Pwm_init_callCount  = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;

    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_update_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_lastDtRising[i]      = 0.0f;
        mock_IfxEgtm_Pwm_lastDtFalling[i]     = 0.0f;
    }

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount            = 0;

    _captured_numChannels = 0;
}

/* Getters */
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_togglePin_getCallCount(void)         { return (int)mock_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }

int mock_IfxEgtm_enable_getCallCount(void)    { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }

int mock_IfxEgtm_Cmu_enable_getCallCount(void)          { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void)       { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void)    { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }

int mock_IfxEgtm_Pwm_init_getCallCount(void)                 { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void)           { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxCpu_enableInterrupts_getCallCount(void)            { return mock_IfxCpu_enableInterrupts_callCount; }
