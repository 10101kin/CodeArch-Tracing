#include "mock_gtm_tom_3_phase_inverter_pwm_2.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxGtm_Pwm.h"

/* Spy state definitions */
int     mock_IfxGtm_Pwm_initConfig_callCount = 0;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

int     mock_IfxGtm_Pwm_init_callCount = 0;
uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;

int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
float32 mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount = 0;
float32 mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxGtm_isEnabled_callCount = 0;
boolean mock_IfxGtm_isEnabled_returnValue = FALSE;

int     mock_IfxGtm_enable_callCount = 0;

int     mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

int     mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_enable_callCount = 0;
int     mock_IfxGtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_togglePin_callCount = 0;

/* Internal captured channel count bounded copy helper */
static uint32 _captured_numChannels = 0;

/* MODULE_* instance definitions */
Ifx_GTM MODULE_GTM = {0};

Ifx_P MODULE_P00 = {0};
Ifx_P MODULE_P01 = {0};
Ifx_P MODULE_P02 = {0};
Ifx_P MODULE_P10 = {0};
Ifx_P MODULE_P11 = {0};
Ifx_P MODULE_P12 = {0};
Ifx_P MODULE_P13 = {0};
Ifx_P MODULE_P14 = {0};
Ifx_P MODULE_P15 = {0};
Ifx_P MODULE_P20 = {0};
Ifx_P MODULE_P21 = {0};
Ifx_P MODULE_P22 = {0};
Ifx_P MODULE_P23 = {0};
Ifx_P MODULE_P24 = {0};
Ifx_P MODULE_P25 = {0};
Ifx_P MODULE_P26 = {0};
Ifx_P MODULE_P30 = {0};
Ifx_P MODULE_P31 = {0};
Ifx_P MODULE_P32 = {0};
Ifx_P MODULE_P33 = {0};
Ifx_P MODULE_P34 = {0};
Ifx_P MODULE_P40 = {0};
Ifx_P MODULE_P41 = {0};

/* Getter implementations */
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDuty_callCount; }
int mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount; }
int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }

/* Reset implementation */
void mock_gtm_tom_3_phase_inverter_pwm_2_reset(void)
{
    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_init_lastNumChannels = 0;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;

    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount = 0;

    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_enable_callCount = 0;

    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;

    _captured_numChannels = 0;
}

/* Stub bodies (do not access struct members except where allowed for init/initConfig) */

/* IfxGtm.h stubs */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_isEnabled_callCount++;
    return mock_IfxGtm_isEnabled_returnValue;
}

void IfxGtm_enable(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_enable_callCount++;
}

/* IfxGtm_Cmu.h stubs */
void IfxGtm_Cmu_enable(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_enable_callCount++;
}

boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_isEnabled_callCount++;
    return mock_IfxGtm_Cmu_isEnabled_returnValue;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f) {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm; (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)gtm; (void)clkIndex; (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency)
{
    (void)gtm; (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

/* IfxPort.h stubs */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
}

/* IfxGtm_Pwm.h stubs */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int priority)
{
    (void)isr; (void)priority;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *duties)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (duties != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = duties[i];
        }
    }
}

void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *duties)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDuty_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (duties != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[i] = duties[i];
        }
    }
}

void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch channel, IfxGtm_Pwm_DeadTime *deadTime)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount++;
    uint32 ch = (uint32)channel;
    if (ch < (uint32)MOCK_MAX_CHANNELS && deadTime != NULL_PTR) {
        mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[ch]  = deadTime->rising;
        mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[ch] = deadTime->falling;
    }
}
