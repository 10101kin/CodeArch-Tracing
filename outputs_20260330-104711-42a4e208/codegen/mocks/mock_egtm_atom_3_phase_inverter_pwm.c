/*
 * mock_egtm_atom_3_phase_inverter_pwm.c
 * Spy state + stub bodies + MODULE_* definitions
 */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"

/* Spy state definitions */
int      mock_IfxEgtm_Pwm_init_callCount = 0;
int      mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
uint32   mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
float32  mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int      mock_IfxEgtm_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_isEnabled_returnValue = FALSE;
int      mock_IfxEgtm_enable_callCount = 0;

int      mock_IfxEgtm_Cmu_enable_callCount = 0;
int      mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE; /* Fixes previous build error */
int      mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int      mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

int      mock_IfxPort_setPinModeOutput_callCount = 0;
int      mock_IfxPort_togglePin_callCount = 0;
uint32   mock_togglePin_callCount = 0u; /* legacy alias */

int      mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int      mock_IfxCpu_enableInterrupts_callCount = 0;

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P21  = {0};

/* Captured number of channels for bounded copy */
static uint32 _captured_numChannels = 0u;

/* Pin symbol definitions (appear in PWM header as extern) */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT = {0u};

/* Getters */
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxCpu_enableInterrupts_getCallCount(void) { return mock_IfxCpu_enableInterrupts_callCount; }

/* Reset */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0u;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_enable_callCount = 0;

    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_enableInterrupts_callCount = 0;
}

/* ========================= STUB BODIES ========================= */

/* IfxCpu helpers */
void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int priority)
{
    (void)handler; (void)priority; mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}
void IfxCpu_enableInterrupts(void)
{
    mock_IfxCpu_enableInterrupts_callCount++;
}

/* EGTM base */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}
void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_enable_callCount++;
}

/* EGTM CMU */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module)
{
    (void)module; mock_IfxEgtm_Cmu_enable_callCount++;
}
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module)
{
    (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++;
    return mock_IfxEgtm_Cmu_isEnabled_returnValue; /* Fixes previous build error */
}
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask; mock_IfxEgtm_Cmu_enableClocks_callCount++;
}
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    /* Default non-zero fallback for tests unless overridden */
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* 100 MHz default */
}
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm)
{
    (void)egtm; return 0.0f;
}
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled)
{
    (void)egtm; (void)clkIndex; (void)assumeEnabled; return 0.0f;
}
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{
    (void)egtm; (void)frequency; mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)egtm; (void)clkIndex; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

/* IfxPort mandatory APIs */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return FALSE; }
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{ (void)port; (void)pinIndex; (void)action; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; mock_IfxPort_togglePin_callCount++; mock_togglePin_callCount++; }
void IfxPort_setPinLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_LvdsMode lvdsMode)
{ (void)port; (void)pinIndex; (void)lvdsMode; }
IfxPort_LvdsMode IfxPort_getPinLVDS(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return IfxPort_LvdsMode_high; }
boolean IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return FALSE; }
boolean IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return FALSE; }
void IfxPort_resetESR(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setESR(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver padDriver)
{ (void)port; (void)pinIndex; (void)padDriver; }
void IfxPort_setPinControllerSelection(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_resetPinControllerSelection(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_modifyPinControllerSelection(Ifx_P *port, uint8 pinIndex, boolean mode)
{ (void)port; (void)pinIndex; (void)mode; }
uint32 IfxPort_getGroupState(Ifx_P *port, uint8 pinIndex, uint16 mask)
{ (void)port; (void)pinIndex; (void)mask; return 0u; }
void IfxPort_setGroupState(Ifx_P *port, uint8 pinIndex, uint16 mask, uint16 data)
{ (void)port; (void)pinIndex; (void)mask; (void)data; }
IfxPort_Index IfxPort_getIndex(Ifx_P *port)
{ (void)port; return IfxPort_Index_0; }
void IfxPort_setGroupModeInput(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_InputMode mode)
{ (void)port; (void)pinIndex; (void)mask; (void)mode; }
void IfxPort_setGroupModeOutput(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mask; (void)mode; (void)index; }
void IfxPort_setGroupPadDriver(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_PadDriver padDriver)
{ (void)port; (void)pinIndex; (void)mask; (void)padDriver; }
void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds)
{ (void)port; (void)pinIndex; (void)pinMode; (void)lvds; }
void IfxPort_initProtConfig(IfxPort_ProtConfig *config)
{ (void)config; }
void IfxPort_initProt(Ifx_P *port, IfxPort_ProtConfig *config)
{ (void)port; (void)config; }
void IfxPort_setApuGroupSelection(Ifx_P *port, uint8 pinIndex, uint8 grpNum)
{ (void)port; (void)pinIndex; (void)grpNum; }
void IfxPort_setPinWakeUpEnable(Ifx_P *port, uint8 pinIndex, boolean enable)
{ (void)port; (void)pinIndex; (void)enable; }
void IfxPort_clearPinWakeUpStatus(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinWakeUpStatusEnable(Ifx_P *port, uint8 pinIndex, boolean enable)
{ (void)port; (void)pinIndex; (void)enable; }
boolean IfxPort_getPinWakeUpStatus(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return FALSE; }
uint32 IfxPort_getRawPinWakeUpStatus(Ifx_P *port)
{ (void)port; return 0u; }
void IfxPort_configureBandGapTrim(Ifx_P *port, IfxPort_BandgapTrimConfig bandgapTrim)
{ (void)port; (void)bandgapTrim; }
void IfxPort_initApuConfig(IfxPort_ApuConfig *config)
{ (void)config; }
void IfxPort_initApu(Ifx_P *port, IfxPort_ApuConfig *config)
{ (void)port; (void)config; }
void IfxPort_initApuGroups(Ifx_P *port, IfxPort_ApuGroupConfig *config)
{ (void)port; (void)config; }
void IfxPort_configureESR(Ifx_P *port, uint8 pinIndex, IfxPort_EsrLevel esrLevel, IfxPort_EsrPadCfg esrPadCfg)
{ (void)port; (void)pinIndex; (void)esrLevel; (void)esrPadCfg; }
void IfxPort_configureAccessToPorts(IfxApApu_ApuConfig *apConfig)
{ (void)apConfig; }
void IfxPort_resetModule(Ifx_P *port)
{ (void)port; }
void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex)
{ (void)port; (void)pinIndex; (void)modex; }
void IfxPort_configureAccessToPort(Ifx_P *port, IfxPort_PadAccessGroup group, void *apConfig)
{ (void)port; (void)group; (void)apConfig; }
void IfxPort_configureLDO(Ifx_P *port, IfxPort_BlankingTimerConfig timerConfig)
{ (void)port; (void)timerConfig; }

/* IfxEgtm_Pwm primary driver */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    (void)egtmSFR;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    mock_IfxEgtm_Pwm_init_callCount++;
    (void)pwm; (void)channels;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    (void)pwm;
    if (requestDuty != NULL_PTR)
    {
        uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}
