/* mock_gtm_tom_3_phase_inverter_pwm.c - Spy state + stub bodies + MODULE_* definitions */
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include <string.h>

/* Spy counters and return-values (definitions) */
int      mock_IfxGtm_enable_callCount = 0;
int      mock_IfxGtm_isEnabled_callCount = 0;
boolean  mock_IfxGtm_isEnabled_returnValue = FALSE;

int      mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int      mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
float32  mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int      mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;

int      mock_IfxGtm_Pwm_initConfig_callCount = 0;
int      mock_IfxGtm_Pwm_init_callCount = 0;
int      mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int      mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;

int      mock_IfxPort_setPinModeOutput_callCount = 0;
uint32   mock_togglePin_callCount = 0;

/* Value-capture spy fields (definitions) */
uint32   mock_IfxGtm_Pwm_init_lastNumChannels = 0;
float32  mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32   mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
float32  mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
float32  mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* MODULE_* instance definitions */
Ifx_GTM    MODULE_GTM;
Ifx_P      MODULE_P00;
Ifx_ASCLIN0 MODULE_ASCLIN0;
Ifx_ASCLIN1 MODULE_ASCLIN1;
Ifx_ASCLIN2 MODULE_ASCLIN2;
Ifx_ASCLIN3 MODULE_ASCLIN3;
Ifx_ASCLIN4 MODULE_ASCLIN4;
Ifx_ASCLIN5 MODULE_ASCLIN5;
Ifx_ASCLIN6 MODULE_ASCLIN6;
Ifx_ASCLIN7 MODULE_ASCLIN7;
Ifx_ASCLIN8 MODULE_ASCLIN8;
Ifx_ASCLIN9 MODULE_ASCLIN9;
Ifx_ASCLIN10 MODULE_ASCLIN10;
Ifx_ASCLIN11 MODULE_ASCLIN11;
Ifx_ASCLIN12 MODULE_ASCLIN12;
Ifx_ASCLIN13 MODULE_ASCLIN13;
Ifx_ASCLIN14 MODULE_ASCLIN14;
Ifx_ASCLIN15 MODULE_ASCLIN15;
Ifx_ASCLIN16 MODULE_ASCLIN16;
Ifx_ASCLIN17 MODULE_ASCLIN17;
Ifx_ASCLIN18 MODULE_ASCLIN18;
Ifx_ASCLIN19 MODULE_ASCLIN19;
Ifx_ASCLIN20 MODULE_ASCLIN20;
Ifx_ASCLIN21 MODULE_ASCLIN21;
Ifx_ASCLIN22 MODULE_ASCLIN22;
Ifx_ASCLIN23 MODULE_ASCLIN23;
Ifx_CAN0   MODULE_CAN0;
Ifx_CAN1   MODULE_CAN1;
Ifx_CAN2   MODULE_CAN2;
Ifx_CBS    MODULE_CBS;
Ifx_CCU60  MODULE_CCU60;
Ifx_CCU61  MODULE_CCU61;
Ifx_CONVCTRL MODULE_CONVCTRL;
Ifx_CPU0   MODULE_CPU0;
Ifx_CPU1   MODULE_CPU1;
Ifx_CPU2   MODULE_CPU2;
Ifx_CPU3   MODULE_CPU3;
Ifx_DAM0   MODULE_DAM0;
Ifx_DMA    MODULE_DMA;
Ifx_DMU    MODULE_DMU;
Ifx_DOM0   MODULE_DOM0;
Ifx_EDSADC MODULE_EDSADC;
Ifx_ERAY0  MODULE_ERAY0;
Ifx_ERAY1  MODULE_ERAY1;
Ifx_EVADC  MODULE_EVADC;
Ifx_FCE    MODULE_FCE;
Ifx_FSI    MODULE_FSI;
Ifx_GETH   MODULE_GETH;
Ifx_GPT120 MODULE_GPT120;
Ifx_HSCT0  MODULE_HSCT0;
Ifx_HSSL0  MODULE_HSSL0;
Ifx_I2C0   MODULE_I2C0;
Ifx_I2C1   MODULE_I2C1;
Ifx_INT    MODULE_INT;
Ifx_IOM    MODULE_IOM;
Ifx_LMU0   MODULE_LMU0;
Ifx_MINIMCDS MODULE_MINIMCDS;
Ifx_MSC0   MODULE_MSC0;
Ifx_MSC1   MODULE_MSC1;
Ifx_MSC2   MODULE_MSC2;
Ifx_MTU    MODULE_MTU;
Ifx_PFI0   MODULE_PFI0;
Ifx_PFI1   MODULE_PFI1;
Ifx_PFI2   MODULE_PFI2;
Ifx_PFI3   MODULE_PFI3;
Ifx_PMS    MODULE_PMS;
Ifx_PMU    MODULE_PMU;
Ifx_P      MODULE_P01;
Ifx_P      MODULE_P02;
Ifx_P      MODULE_P10;
Ifx_P      MODULE_P11;
Ifx_P      MODULE_P12;
Ifx_P      MODULE_P13;
Ifx_P      MODULE_P14;
Ifx_P      MODULE_P15;
Ifx_P      MODULE_P20;
Ifx_P      MODULE_P21;
Ifx_P      MODULE_P22;
Ifx_P      MODULE_P23;
Ifx_P      MODULE_P24;
Ifx_P      MODULE_P25;
Ifx_P      MODULE_P26;
Ifx_P      MODULE_P30;
Ifx_P      MODULE_P31;
Ifx_P      MODULE_P32;
Ifx_P      MODULE_P33;
Ifx_P      MODULE_P34;
Ifx_P      MODULE_P40;
Ifx_P      MODULE_P41;
Ifx_PSI5S  MODULE_PSI5S;
Ifx_PSI5   MODULE_PSI5;
Ifx_QSPI0  MODULE_QSPI0;
Ifx_QSPI1  MODULE_QSPI1;
Ifx_QSPI2  MODULE_QSPI2;
Ifx_QSPI3  MODULE_QSPI3;
Ifx_QSPI4  MODULE_QSPI4;
Ifx_SBCU   MODULE_SBCU;
Ifx_SCU    MODULE_SCU;
Ifx_SENT   MODULE_SENT;
Ifx_SMU    MODULE_SMU;
Ifx_SRC    MODULE_SRC;
Ifx_STM0   MODULE_STM0;
Ifx_STM1   MODULE_STM1;
Ifx_STM2   MODULE_STM2;
Ifx_STM3   MODULE_STM3;

/* IfxGtm.h stubs */
void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }
boolean IfxGtm_isEnabled(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_isEnabled_callCount++; return mock_IfxGtm_isEnabled_returnValue; }
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm) { (void)gtm; return FALSE; }
void IfxGtm_setSuspendMode(Ifx_GTM *gtm, IfxGtm_SuspendMode mode) { (void)gtm; (void)mode; }
void IfxGtm_disable(Ifx_GTM *gtm) { (void)gtm; }
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, IfxGtm_Cluster cluster) { (void)gtm; (void)cluster; return 0.0f; }

/* IfxGtm_Cmu.h stubs */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) { (void)gtm; (void)clkMask; mock_IfxGtm_Cmu_enableClocks_callCount++; }
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return 0.0f; }
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return 0.0f; }
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return 0.0f; }
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_Cmu_getModuleFrequency_callCount++; if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f) return mock_IfxGtm_Cmu_getModuleFrequency_returnValue; return 100000000.0f; }
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return FALSE; }
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return FALSE; }
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return FALSE; }
void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, uint32 input) { (void)gtm; (void)clk; (void)input; }
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq) { (void)gtm; (void)clk; (void)freq; }
void IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk, float32 freq) { (void)gtm; (void)eclk; (void)freq; }
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency) { (void)gtm; (void)frequency; mock_IfxGtm_Cmu_setGclkFrequency_callCount++; }

/* IfxGtm_Pwm.h stubs */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), uint32 vectabNum, Ifx_Priority priority) { (void)isr; (void)vectabNum; (void)priority; }

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency   = config->frequency;
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm; (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency   = config->frequency;
    }
}

void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; }
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty) { (void)pwm; (void)requestDuty; }
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, Ifx_ActiveState polarity) { (void)pwm; (void)ch; (void)polarity; }
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 dtRising, float32 dtFalling) { (void)pwm; (void)ch; (void)dtRising; (void)dtFalling; }
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *chCfg, Ifx_GTM *gtmSFR) { (void)chCfg; (void)gtmSFR; }
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }
void IfxGtm_Pwm_updateSyncedGroupsFrequency(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; }
void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; }
void IfxGtm_Pwm_updateChannelPulse(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 pulseTicks) { (void)pwm; (void)ch; (void)pulseTicks; }
void IfxGtm_Pwm_updateChannelPulseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 pulseTicks) { (void)pwm; (void)ch; (void)pulseTicks; }
void IfxGtm_Pwm_updateChannelsPhase(IfxGtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; }
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty != NULL_PTR) {
        unsigned i;
        for (i = 0; i < MOCK_MAX_CHANNELS; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}
void IfxGtm_Pwm_updateChannelsPulse(IfxGtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; }
void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, float32 *dtRising, float32 *dtFalling) {
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount++;
    if (dtRising != NULL_PTR) {
        unsigned i; for (i = 0; i < MOCK_MAX_CHANNELS; ++i) { mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = dtRising[i]; }
    }
    if (dtFalling != NULL_PTR) {
        unsigned i; for (i = 0; i < MOCK_MAX_CHANNELS; ++i) { mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = dtFalling[i]; }
    }
}
void IfxGtm_Pwm_updateChannelsPhaseImmediate(IfxGtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; }
void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; }
void IfxGtm_Pwm_interruptHandler(IfxGtm_Pwm *pwm) { (void)pwm; }
IfxGtm_Pwm_ChannelState IfxGtm_Pwm_getChannelState(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch) { (void)pwm; (void)ch; return IfxGtm_Pwm_ChannelState_running; }
void IfxGtm_Pwm_stopChannelOutputs(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; }

/* IfxPort.h stubs */
IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; return IfxPort_State_notChanged; }
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) { (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State state) { (void)port; (void)pinIndex; (void)state; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_togglePin_callCount++; }
void IfxPort_disableEmergencyStop(Ifx_P *port, uint16 mask) { (void)port; (void)mask; }
void IfxPort_enableEmergencyStop(Ifx_P *port, uint16 mask) { (void)port; (void)mask; }
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_LvdsConfig *cfg) { (void)port; (void)pinIndex; (void)cfg; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver) { (void)port; (void)pinIndex; (void)driver; }
void IfxPort_setPinControllerSelection(Ifx_P *port, uint8 pinIndex, IfxPort_ControlledBy controller) { (void)port; (void)pinIndex; (void)controller; }
void IfxPort_resetPinControllerSelection(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
uint32 IfxPort_getGroupState(Ifx_P *port, uint16 mask) { (void)port; (void)mask; return 0U; }
void IfxPort_setGroupModeOutput(Ifx_P *port, uint16 mask, IfxPort_OutputMode mode, IfxPort_OutputIdx index) { (void)port; (void)mask; (void)mode; (void)index; }
void IfxPort_setGroupState(Ifx_P *port, uint16 mask, IfxPort_State state) { (void)port; (void)mask; (void)state; }
uint8 IfxPort_getIndex(Ifx_P *port) { (void)port; return 0U; }
void IfxPort_setGroupModeInput(Ifx_P *port, uint16 mask, IfxPort_InputMode mode) { (void)port; (void)mask; (void)mode; }
void IfxPort_setGroupPadDriver(Ifx_P *port, uint16 mask, IfxPort_PadDriver padDriver) { (void)port; (void)mask; (void)padDriver; }
void IfxPort_resetESR(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setESR(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_modifyPinControllerSelection(Ifx_P *port, uint16 mask, IfxPort_ControlledBy add, IfxPort_ControlledBy remove) { (void)port; (void)mask; (void)add; (void)remove; }

/* Watchdog stubs (declared in IfxPort.h list) */
void IfxScuWdt_clearCpuEndinit(uint16 password) { (void)password; }
void IfxScuWdt_setCpuEndinit(uint16 password) { (void)password; }

/* Mock control API implementations */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void) {
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;

    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    unsigned i;
    for (i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
}

int  mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int  mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int  mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int  mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int  mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int  mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int  mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }
