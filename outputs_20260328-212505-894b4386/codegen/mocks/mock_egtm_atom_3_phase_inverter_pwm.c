#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include <string.h>

/* Spy counters and return controls */
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
int mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;

int mock_IfxEgtm_isEnabled_callCount = 0;
int mock_IfxEgtm_enable_callCount = 0;

int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int mock_IfxCpu_enableInterrupts_callCount = 0;

int mock_IfxPort_setPinModeOutput_callCount = 0;
int mock_IfxPort_togglePin_callCount = 0;
uint32 mock_togglePin_callCount = 0;

boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;

/* Spy capture fields */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Local state for bounded copy */
static uint32 _captured_numChannels = 0;

/* MODULE_* instance definitions (only those typically used by this module + requested ports) */
Ifx_EGTM MODULE_EGTM;
Ifx_P MODULE_P00; Ifx_P MODULE_P01; Ifx_P MODULE_P02; Ifx_P MODULE_P03; Ifx_P MODULE_P04;
Ifx_P MODULE_P10; Ifx_P MODULE_P13; Ifx_P MODULE_P14; Ifx_P MODULE_P15; Ifx_P MODULE_P16;
Ifx_P MODULE_P20; Ifx_P MODULE_P21; Ifx_P MODULE_P22; Ifx_P MODULE_P23; Ifx_P MODULE_P25;
Ifx_P MODULE_P30; Ifx_P MODULE_P31; Ifx_P MODULE_P32; Ifx_P MODULE_P33; Ifx_P MODULE_P34; Ifx_P MODULE_P35; Ifx_P MODULE_P40; Ifx_P MODULE_P41;

/* Also provide a few other module instances to avoid accidental link errors if referenced */
Ifx_ADC MODULE_ADC;
Ifx_ASCLIN0 MODULE_ASCLIN0; Ifx_ASCLIN1 MODULE_ASCLIN1; Ifx_ASCLIN2 MODULE_ASCLIN2; Ifx_ASCLIN3 MODULE_ASCLIN3; Ifx_ASCLIN4 MODULE_ASCLIN4; Ifx_ASCLIN5 MODULE_ASCLIN5; Ifx_ASCLIN6 MODULE_ASCLIN6; Ifx_ASCLIN7 MODULE_ASCLIN7; Ifx_ASCLIN8 MODULE_ASCLIN8; Ifx_ASCLIN9 MODULE_ASCLIN9; Ifx_ASCLIN10 MODULE_ASCLIN10; Ifx_ASCLIN11 MODULE_ASCLIN11; Ifx_ASCLIN12 MODULE_ASCLIN12; Ifx_ASCLIN13 MODULE_ASCLIN13; Ifx_ASCLIN14 MODULE_ASCLIN14; Ifx_ASCLIN15 MODULE_ASCLIN15; Ifx_ASCLIN16 MODULE_ASCLIN16; Ifx_ASCLIN17 MODULE_ASCLIN17; Ifx_ASCLIN18 MODULE_ASCLIN18; Ifx_ASCLIN19 MODULE_ASCLIN19; Ifx_ASCLIN20 MODULE_ASCLIN20; Ifx_ASCLIN21 MODULE_ASCLIN21; Ifx_ASCLIN22 MODULE_ASCLIN22; Ifx_ASCLIN23 MODULE_ASCLIN23; Ifx_ASCLIN24 MODULE_ASCLIN24; Ifx_ASCLIN25 MODULE_ASCLIN25; Ifx_ASCLIN26 MODULE_ASCLIN26; Ifx_ASCLIN27 MODULE_ASCLIN27;

/* Stub bodies (do not access struct members) */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{ (void)egtm; (void)clkMask; mock_IfxEgtm_Cmu_enableClocks_callCount++; }

void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{ (void)egtm; (void)numerator; (void)denominator; mock_IfxEgtm_Cmu_setGclkDivider_callCount++; }

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++; return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }

void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, uint32 numerator, uint32 denominator)
{ (void)egtm; (void)clkIndex; (void)numerator; (void)denominator; mock_IfxEgtm_Cmu_setEclkDivider_callCount++; }

float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_Cmu_getGclkFrequency_callCount++; return (mock_IfxEgtm_Cmu_getGclkFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getGclkFrequency_returnValue : 100000000.0f; }

float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled)
{ (void)egtm; (void)clkIndex; (void)assumeEnabled; mock_IfxEgtm_Cmu_getClkFrequency_callCount++; return (mock_IfxEgtm_Cmu_getClkFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getClkFrequency_returnValue : 100000000.0f; }

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency)
{ (void)egtm; (void)clkIndex; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_callCount++; }

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{ (void)egtm; (void)frequency; mock_IfxEgtm_Cmu_setGclkFrequency_callCount++; }

boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_isEnabled_callCount++; return mock_IfxEgtm_isEnabled_returnValue; }

void IfxEgtm_enable(Ifx_EGTM *egtm)
{ (void)egtm; mock_IfxEgtm_enable_callCount++; }

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

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int priority)
{ (void)handler; (void)priority; mock_IfxCpu_Irq_installInterruptHandler_callCount++; }

void IfxCpu_enableInterrupts(void)
{ mock_IfxCpu_enableInterrupts_callCount++; }

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; mock_IfxPort_togglePin_callCount++; mock_togglePin_callCount++; }

/* Getters */
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkDivider_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(void) { return mock_IfxEgtm_Cmu_setEclkDivider_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }

int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxCpu_enableInterrupts_getCallCount(void) { return mock_IfxCpu_enableInterrupts_callCount; }

int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }

/* Reset API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0;

    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;
}
