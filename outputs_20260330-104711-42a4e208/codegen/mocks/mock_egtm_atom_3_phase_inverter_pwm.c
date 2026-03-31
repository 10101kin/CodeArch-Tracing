/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* MODULE_* instances */
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

/* Spy state */
int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int mock_IfxEgtm_isEnabled_callCount = 0;
int mock_IfxEgtm_enable_callCount = 0;
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
uint32 mock_togglePin_callCount = 0U;

boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;

boolean      mock_IfxPort_getPinState_returnValue = FALSE;
uint32       mock_IfxPort_getGroupState_returnValue = 0U;
boolean      mock_IfxPort_disableEmergencyStop_returnValue = FALSE;
boolean      mock_IfxPort_enableEmergencyStop_returnValue = FALSE;
unsigned int mock_IfxPort_getRawPinWakeUpStatus_returnValue = 0U;
boolean      mock_IfxPort_getPinWakeUpStatus_returnValue = FALSE;
unsigned int mock_IfxPort_getIndex_returnValue = 0U;
unsigned int mock_IfxPort_getPinLVDS_returnValue = 0U;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Bounded copy helper state */
static uint32 _captured_numChannels = 0U;

/* Getters */
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
uint32 mock_togglePin_getCallCount(void) { return mock_togglePin_callCount; }

/* Reset */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_togglePin_callCount = 0U;

    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_getClkFrequency_returnValue = 0.0f;

    mock_IfxPort_getPinState_returnValue = FALSE;
    mock_IfxPort_getGroupState_returnValue = 0U;
    mock_IfxPort_disableEmergencyStop_returnValue = FALSE;
    mock_IfxPort_enableEmergencyStop_returnValue = FALSE;
    mock_IfxPort_getRawPinWakeUpStatus_returnValue = 0U;
    mock_IfxPort_getPinWakeUpStatus_returnValue = FALSE;
    mock_IfxPort_getIndex_returnValue = 0U;
    mock_IfxPort_getPinLVDS_returnValue = 0U;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0U;
}

/* ========================= IfxEgtm_Pwm stubs ========================= */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0U && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

/* Also declared in IfxEgtm_Pwm.h (CPU helpers) */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int vectabNum, int priority)
{
    (void)isr; (void)vectabNum; (void)priority;
}
void IfxCpu_enableInterrupts(void)
{
}

/* ========================= IfxEgtm stubs ========================= */
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

/* ========================= IfxEgtm_Cmu stubs ========================= */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module)
{
    (void)module;
}

boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module)
{
    (void)module;
    return mock_IfxEgtm_Cmu_isEnabled_returnValue;
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

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

/* ========================= IfxPort stubs ========================= */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return mock_IfxPort_getPinState_returnValue; }
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; }
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{ (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mode; (void)index; }
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{ (void)port; (void)pinIndex; (void)action; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; mock_togglePin_callCount++; }
void IfxPort_setPinLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_LvdsMode lvdsMode)
{ (void)port; (void)pinIndex; (void)lvdsMode; }
IfxPort_LvdsMode IfxPort_getPinLVDS(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return (IfxPort_LvdsMode)mock_IfxPort_getPinLVDS_returnValue; }
boolean IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return mock_IfxPort_disableEmergencyStop_returnValue; }
boolean IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{ (void)port; (void)pinIndex; return mock_IfxPort_enableEmergencyStop_returnValue; }
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
{ (void)port; (void)pinIndex; (void)mask; return mock_IfxPort_getGroupState_returnValue; }
void IfxPort_setGroupState(Ifx_P *port, uint8 pinIndex, uint16 mask, uint16 data)
{ (void)port; (void)pinIndex; (void)mask; (void)data; }
unsigned int IfxPort_getIndex(Ifx_P *port)
{ (void)port; return mock_IfxPort_getIndex_returnValue; }
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
{ (void)port; (void)pinIndex; return mock_IfxPort_getPinWakeUpStatus_returnValue; }
uint32 IfxPort_getRawPinWakeUpStatus(Ifx_P *port)
{ (void)port; return mock_IfxPort_getRawPinWakeUpStatus_returnValue; }
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

