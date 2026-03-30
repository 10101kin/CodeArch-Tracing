/*
 * mock_egtm_atom_3_phase_inverter_pwm.c
 * Spy state + stub bodies + MODULE_* definitions
 */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P21  = {0};

/* Internal captured channel count for bounded duty copies */
static uint32 _captured_numChannels = 0;

/* Spy counters and return-value globals */
int      mock_IfxEgtm_Pwm_init_callCount = 0;
int      mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int      mock_IfxEgtm_enable_callCount = 0;
int      mock_IfxEgtm_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_isEnabled_returnValue = FALSE;

int      mock_IfxEgtm_Cmu_enable_callCount = 0;
int      mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean  mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int      mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int      mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

int      mock_IfxPort_setPinModeOutput_callCount = 0;
int      mock_togglePin_callCount = 0;
int      mock_IfxPort_getPinState_callCount = 0;
boolean  mock_IfxPort_getPinState_returnValue = FALSE;
int      mock_IfxPort_getPinLVDS_callCount = 0;
uint32   mock_IfxPort_getPinLVDS_returnValue = 0u;
int      mock_IfxPort_disableEmergencyStop_callCount = 0;
boolean  mock_IfxPort_disableEmergencyStop_returnValue = FALSE;
int      mock_IfxPort_enableEmergencyStop_callCount = 0;
boolean  mock_IfxPort_enableEmergencyStop_returnValue = FALSE;
int      mock_IfxPort_getGroupState_callCount = 0;
uint32   mock_IfxPort_getGroupState_returnValue = 0u;
int      mock_IfxPort_getIndex_callCount = 0;
uint32   mock_IfxPort_getIndex_returnValue = 0u;
int      mock_IfxPort_getPinWakeUpStatus_callCount = 0;
boolean  mock_IfxPort_getPinWakeUpStatus_returnValue = FALSE;
int      mock_IfxPort_getRawPinWakeUpStatus_callCount = 0;
uint32   mock_IfxPort_getRawPinWakeUpStatus_returnValue = 0u;

/* Value-capture globals */
uint32   mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Mock control: getters */
int  mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int  mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int  mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int  mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_togglePin_getCallCount(void) { return mock_togglePin_callCount; }
int  mock_IfxPort_getPinState_getCallCount(void) { return mock_IfxPort_getPinState_callCount; }
int  mock_IfxPort_getPinLVDS_getCallCount(void) { return mock_IfxPort_getPinLVDS_callCount; }
int  mock_IfxPort_disableEmergencyStop_getCallCount(void) { return mock_IfxPort_disableEmergencyStop_callCount; }
int  mock_IfxPort_enableEmergencyStop_getCallCount(void) { return mock_IfxPort_enableEmergencyStop_callCount; }
int  mock_IfxPort_getGroupState_getCallCount(void) { return mock_IfxPort_getGroupState_callCount; }
int  mock_IfxPort_getIndex_getCallCount(void) { return mock_IfxPort_getIndex_callCount; }
int  mock_IfxPort_getPinWakeUpStatus_getCallCount(void) { return mock_IfxPort_getPinWakeUpStatus_callCount; }
int  mock_IfxPort_getRawPinWakeUpStatus_getCallCount(void) { return mock_IfxPort_getRawPinWakeUpStatus_callCount; }

/* Reset function */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0;
    mock_IfxPort_getPinState_callCount = 0;
    mock_IfxPort_getPinState_returnValue = FALSE;
    mock_IfxPort_getPinLVDS_callCount = 0;
    mock_IfxPort_getPinLVDS_returnValue = 0u;
    mock_IfxPort_disableEmergencyStop_callCount = 0;
    mock_IfxPort_disableEmergencyStop_returnValue = FALSE;
    mock_IfxPort_enableEmergencyStop_callCount = 0;
    mock_IfxPort_enableEmergencyStop_returnValue = FALSE;
    mock_IfxPort_getGroupState_callCount = 0;
    mock_IfxPort_getGroupState_returnValue = 0u;
    mock_IfxPort_getIndex_callCount = 0;
    mock_IfxPort_getIndex_returnValue = 0u;
    mock_IfxPort_getPinWakeUpStatus_callCount = 0;
    mock_IfxPort_getPinWakeUpStatus_returnValue = FALSE;
    mock_IfxPort_getRawPinWakeUpStatus_callCount = 0;
    mock_IfxPort_getRawPinWakeUpStatus_returnValue = 0u;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;
}

/* ========================= Stubs: IfxEgtm ========================= */
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

/* ========================= Stubs: IfxEgtm_Cmu ========================= */
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

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    /* Allow test to override; else return 0.0f as default */
    return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
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

/* ========================= Stubs: IfxEgtm_Pwm ========================= */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency = config->frequency;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

/* ========================= Stubs: IfxPort ========================= */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_getPinState_callCount++;
    return mock_IfxPort_getPinState_returnValue;
}

void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode omode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)omode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    (void)port; (void)pinIndex; (void)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_togglePin_callCount++;
}

void IfxPort_setPinLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_LvdsMode lvdsMode)
{
    (void)port; (void)pinIndex; (void)lvdsMode;
}

uint32 IfxPort_getPinLVDS(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_getPinLVDS_callCount++;
    return mock_IfxPort_getPinLVDS_returnValue;
}

boolean IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_disableEmergencyStop_callCount++;
    return mock_IfxPort_disableEmergencyStop_returnValue;
}

boolean IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_enableEmergencyStop_callCount++;
    return mock_IfxPort_enableEmergencyStop_returnValue;
}

void IfxPort_resetESR(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setESR(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver padDriver)
{
    (void)port; (void)pinIndex; (void)padDriver;
}

void IfxPort_setPinControllerSelection(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_resetPinControllerSelection(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_modifyPinControllerSelection(Ifx_P *port, uint8 pinIndex, boolean mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

uint32 IfxPort_getGroupState(Ifx_P *port, uint8 pinIndex, uint16 mask)
{
    (void)port; (void)pinIndex; (void)mask;
    mock_IfxPort_getGroupState_callCount++;
    return mock_IfxPort_getGroupState_returnValue;
}

void IfxPort_setGroupModeOutput(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mask; (void)mode; (void)index;
}

void IfxPort_setGroupState(Ifx_P *port, uint8 pinIndex, uint16 mask, uint16 data)
{
    (void)port; (void)pinIndex; (void)mask; (void)data;
}

uint32 IfxPort_getIndex(Ifx_P *port)
{
    (void)port;
    mock_IfxPort_getIndex_callCount++;
    return mock_IfxPort_getIndex_returnValue;
}

void IfxPort_setGroupModeInput(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_InputMode mode)
{
    (void)port; (void)pinIndex; (void)mask; (void)mode;
}

void IfxPort_setGroupPadDriver(Ifx_P *port, uint8 pinIndex, uint16 mask, IfxPort_PadDriver padDriver)
{
    (void)port; (void)pinIndex; (void)mask; (void)padDriver;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds)
{
    (void)port; (void)pinIndex; (void)pinMode; (void)lvds;
}

void IfxPort_initProtConfig(IfxPort_ProtConfig *config)
{
    (void)config;
}

void IfxPort_initProt(Ifx_P *port, IfxPort_ProtConfig *config)
{
    (void)port; (void)config;
}

void IfxPort_setApuGroupSelection(Ifx_P *port, uint8 pinIndex, uint8 grpNum)
{
    (void)port; (void)pinIndex; (void)grpNum;
}

void IfxPort_setPinWakeUpEnable(Ifx_P *port, uint8 pinIndex, boolean enable)
{
    (void)port; (void)pinIndex; (void)enable;
}

void IfxPort_clearPinWakeUpStatus(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinWakeUpStatusEnable(Ifx_P *port, uint8 pinIndex, boolean enable)
{
    (void)port; (void)pinIndex; (void)enable;
}

boolean IfxPort_getPinWakeUpStatus(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_getPinWakeUpStatus_callCount++;
    return mock_IfxPort_getPinWakeUpStatus_returnValue;
}

uint32 IfxPort_getRawPinWakeUpStatus(Ifx_P *port)
{
    (void)port;
    mock_IfxPort_getRawPinWakeUpStatus_callCount++;
    return mock_IfxPort_getRawPinWakeUpStatus_returnValue;
}

void IfxPort_configureBandGapTrim(Ifx_P *port, IfxPort_BandgapTrimConfig bandgapTrim)
{
    (void)port; (void)bandgapTrim;
}

void IfxPort_initApuConfig(IfxPort_ApuConfig *config)
{
    (void)config;
}

void IfxPort_initApu(Ifx_P *port, IfxPort_ApuConfig *config)
{
    (void)port; (void)config;
}

void IfxPort_initApuGroups(Ifx_P *port, IfxPort_ApuGroupConfig *config)
{
    (void)port; (void)config;
}

void IfxPort_configureESR(Ifx_P *port, uint8 pinIndex, IfxPort_EsrLevel esrLevel, IfxPort_EsrPadCfg esrPadCfg)
{
    (void)port; (void)pinIndex; (void)esrLevel; (void)esrPadCfg;
}

void IfxPort_configureAccessToPorts(IfxApApu_ApuConfig *apConfig)
{
    (void)apConfig;
}

void IfxPort_resetModule(Ifx_P *port)
{
    (void)port;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex)
{
    (void)port; (void)pinIndex; (void)modex;
}

void IfxPort_configureAccessToPort(Ifx_P *port, IfxPort_PadAccessGroup group, void *apConfig)
{
    (void)port; (void)group; (void)apConfig;
}

void IfxPort_configureLDO(Ifx_P *port, IfxPort_BlankingTimerConfig timerConfig)
{
    (void)port; (void)timerConfig;
}
