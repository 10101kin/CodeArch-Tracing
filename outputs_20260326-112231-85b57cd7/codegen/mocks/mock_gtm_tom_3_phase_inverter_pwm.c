#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxGtm_Pwm.h"

/* Spy state definitions */
int     mock_IfxGtm_enable_callCount = 0;
int     mock_IfxGtm_isEnabled_callCount = 0;
int     mock_IfxGtm_Pwm_initConfig_callCount = 0;
int     mock_IfxGtm_Pwm_init_callCount = 0;
int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;
uint32  mock_togglePin_callCount = 0u;

boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* MODULE_* instance definitions */
Ifx_GTM MODULE_GTM = {0};
Ifx_P   MODULE_P00 = {0};
Ifx_ASCLIN0 MODULE_ASCLIN0 = {0};
Ifx_ASCLIN1 MODULE_ASCLIN1 = {0};
Ifx_ASCLIN2 MODULE_ASCLIN2 = {0};
Ifx_ASCLIN3 MODULE_ASCLIN3 = {0};
Ifx_ASCLIN4 MODULE_ASCLIN4 = {0};
Ifx_ASCLIN5 MODULE_ASCLIN5 = {0};
Ifx_ASCLIN6 MODULE_ASCLIN6 = {0};
Ifx_ASCLIN7 MODULE_ASCLIN7 = {0};
Ifx_ASCLIN8 MODULE_ASCLIN8 = {0};
Ifx_ASCLIN9 MODULE_ASCLIN9 = {0};
Ifx_ASCLIN10 MODULE_ASCLIN10 = {0};
Ifx_ASCLIN11 MODULE_ASCLIN11 = {0};
Ifx_ASCLIN12 MODULE_ASCLIN12 = {0};
Ifx_ASCLIN13 MODULE_ASCLIN13 = {0};
Ifx_ASCLIN14 MODULE_ASCLIN14 = {0};
Ifx_ASCLIN15 MODULE_ASCLIN15 = {0};
Ifx_ASCLIN16 MODULE_ASCLIN16 = {0};
Ifx_ASCLIN17 MODULE_ASCLIN17 = {0};
Ifx_ASCLIN18 MODULE_ASCLIN18 = {0};
Ifx_ASCLIN19 MODULE_ASCLIN19 = {0};
Ifx_ASCLIN20 MODULE_ASCLIN20 = {0};
Ifx_ASCLIN21 MODULE_ASCLIN21 = {0};
Ifx_ASCLIN22 MODULE_ASCLIN22 = {0};
Ifx_ASCLIN23 MODULE_ASCLIN23 = {0};
Ifx_CAN0 MODULE_CAN0 = {0};
Ifx_CAN1 MODULE_CAN1 = {0};
Ifx_CAN2 MODULE_CAN2 = {0};
Ifx_CBS MODULE_CBS = {0};
Ifx_CCU60 MODULE_CCU60 = {0};
Ifx_CCU61 MODULE_CCU61 = {0};
Ifx_CONVCTRL MODULE_CONVCTRL = {0};
Ifx_CPU0 MODULE_CPU0 = {0};
Ifx_CPU1 MODULE_CPU1 = {0};
Ifx_CPU2 MODULE_CPU2 = {0};
Ifx_CPU3 MODULE_CPU3 = {0};
Ifx_DAM0 MODULE_DAM0 = {0};
Ifx_DMA MODULE_DMA = {0};
Ifx_DMU MODULE_DMU = {0};
Ifx_DOM0 MODULE_DOM0 = {0};
Ifx_EDSADC MODULE_EDSADC = {0};
Ifx_ERAY0 MODULE_ERAY0 = {0};
Ifx_ERAY1 MODULE_ERAY1 = {0};
Ifx_EVADC MODULE_EVADC = {0};
Ifx_FCE MODULE_FCE = {0};
Ifx_FSI MODULE_FSI = {0};
Ifx_GETH MODULE_GETH = {0};
Ifx_GPT120 MODULE_GPT120 = {0};
Ifx_HSCT0 MODULE_HSCT0 = {0};
Ifx_HSSL0 MODULE_HSSL0 = {0};
Ifx_I2C0 MODULE_I2C0 = {0};
Ifx_I2C1 MODULE_I2C1 = {0};
Ifx_INT MODULE_INT = {0};
Ifx_IOM MODULE_IOM = {0};
Ifx_LMU0 MODULE_LMU0 = {0};
Ifx_MINIMCDS MODULE_MINIMCDS = {0};
Ifx_MSC0 MODULE_MSC0 = {0};
Ifx_MSC1 MODULE_MSC1 = {0};
Ifx_MSC2 MODULE_MSC2 = {0};
Ifx_MTU MODULE_MTU = {0};
Ifx_PFI0 MODULE_PFI0 = {0};
Ifx_PFI1 MODULE_PFI1 = {0};
Ifx_PFI2 MODULE_PFI2 = {0};
Ifx_PFI3 MODULE_PFI3 = {0};
Ifx_PMS MODULE_PMS = {0};
Ifx_PMU MODULE_PMU = {0};
Ifx_P   MODULE_P01 = {0};
Ifx_P   MODULE_P02 = {0};
Ifx_P   MODULE_P10 = {0};
Ifx_P   MODULE_P11 = {0};
Ifx_P   MODULE_P12 = {0};
Ifx_P   MODULE_P13 = {0};
Ifx_P   MODULE_P14 = {0};
Ifx_P   MODULE_P15 = {0};
Ifx_P   MODULE_P20 = {0};
Ifx_P   MODULE_P21 = {0};
Ifx_P   MODULE_P22 = {0};
Ifx_P   MODULE_P23 = {0};
Ifx_P   MODULE_P24 = {0};
Ifx_P   MODULE_P25 = {0};
Ifx_P   MODULE_P26 = {0};
Ifx_P   MODULE_P30 = {0};
Ifx_P   MODULE_P31 = {0};
Ifx_P   MODULE_P32 = {0};
Ifx_P   MODULE_P33 = {0};
Ifx_P   MODULE_P34 = {0};
Ifx_P   MODULE_P40 = {0};
Ifx_P   MODULE_P41 = {0};
Ifx_PSI5S MODULE_PSI5S = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI0 MODULE_QSPI0 = {0};
Ifx_QSPI1 MODULE_QSPI1 = {0};
Ifx_QSPI2 MODULE_QSPI2 = {0};
Ifx_QSPI3 MODULE_QSPI3 = {0};
Ifx_QSPI4 MODULE_QSPI4 = {0};
Ifx_SBCU MODULE_SBCU = {0};
Ifx_SCU MODULE_SCU = {0};
Ifx_SENT MODULE_SENT = {0};
Ifx_SMU MODULE_SMU = {0};
Ifx_SRC MODULE_SRC = {0};
Ifx_STM0 MODULE_STM0 = {0};
Ifx_STM1 MODULE_STM1 = {0};
Ifx_STM2 MODULE_STM2 = {0};
Ifx_STM3 MODULE_STM3 = {0};

/* Stub bodies */

/* IfxGtm.h */
void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }
boolean IfxGtm_isEnabled(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_isEnabled_callCount++; return mock_IfxGtm_isEnabled_returnValue; }
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm) { (void)gtm; return FALSE; }
void IfxGtm_setSuspendMode(Ifx_GTM *gtm, IfxGtm_SuspendMode mode) { (void)gtm; (void)mode; }
void IfxGtm_disable(Ifx_GTM *gtm) { (void)gtm; }
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, IfxGtm_Cluster cluster) { (void)gtm; (void)cluster; return 0.0f; }

/* IfxGtm_Cmu.h */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) { (void)gtm; (void)clkMask; mock_IfxGtm_Cmu_enableClocks_callCount++; }
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return 0.0f; }
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return 0.0f; }
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return 0.0f; }
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm) {
    (void)gtm;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f) {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return FALSE; }
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return FALSE; }
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return FALSE; }
void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; }
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq) { (void)gtm; (void)clk; (void)freq; }
void IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk, float32 freq) { (void)gtm; (void)eclk; (void)freq; }
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency) { (void)gtm; (void)frequency; }

/* IfxPort.h */
IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; return IfxPort_State_notChanged; }
void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) { (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State state) { (void)port; (void)pinIndex; (void)state; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_togglePin_callCount++; }
void IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_LvdsConfig *config) { (void)port; (void)pinIndex; (void)config; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver padDriver) { (void)port; (void)pinIndex; (void)padDriver; }
void IfxPort_setPinControllerSelection(Ifx_P *port, uint8 pinIndex, IfxPort_ControlledBy ctrl) { (void)port; (void)pinIndex; (void)ctrl; }
void IfxPort_resetPinControllerSelection(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }

/* IfxGtm_Pwm.h */
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency = config->frequency;
    }
}
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm; (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency = config->frequency;
    }
}
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = mock_IfxGtm_Pwm_init_lastNumChannels;
    if (n > MOCK_MAX_CHANNELS) n = MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
        for (uint32 j = n; j < MOCK_MAX_CHANNELS; ++j) {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[j] = 0.0f;
        }
    }
}
void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, float32 *dtRising, float32 *dtFalling) {
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount++;
    uint32 n = mock_IfxGtm_Pwm_init_lastNumChannels;
    if (n > MOCK_MAX_CHANNELS) n = MOCK_MAX_CHANNELS;
    if (dtRising != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = dtRising[i];
        }
        for (uint32 j = n; j < MOCK_MAX_CHANNELS; ++j) {
            mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[j] = 0.0f;
        }
    }
    if (dtFalling != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = dtFalling[i];
        }
        for (uint32 j = n; j < MOCK_MAX_CHANNELS; ++j) {
            mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[j] = 0.0f;
        }
    }
}
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *config) { (void)config; }
void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 freq) { (void)pwm; (void)freq; }
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty) { (void)pwm; (void)requestDuty; }
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, Ifx_ActiveState polarity) { (void)pwm; (void)ch; (void)polarity; }
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, IfxGtm_Pwm_DeadTime *dt) { (void)pwm; (void)ch; (void)dt; }
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm) { (void)pwm; }
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm) { (void)pwm; }
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm) { (void)pwm; }
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm) { (void)pwm; }
void IfxGtm_Pwm_updateSyncedGroupsFrequency(IfxGtm_Pwm *pwm, float32 freq) { (void)pwm; (void)freq; }
void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 freq) { (void)pwm; (void)freq; }
void IfxGtm_Pwm_updateChannelPulse(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 pulse) { (void)pwm; (void)ch; (void)pulse; }
void IfxGtm_Pwm_updateChannelPulseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 pulse) { (void)pwm; (void)ch; (void)pulse; }
void IfxGtm_Pwm_updateChannelsPhase(IfxGtm_Pwm *pwm, float32 *phase) { (void)pwm; (void)phase; }
void IfxGtm_Pwm_updateChannelsPulse(IfxGtm_Pwm *pwm, float32 *pulse) { (void)pwm; (void)pulse; }
void IfxGtm_Pwm_updateChannelsPhaseImmediate(IfxGtm_Pwm *pwm, float32 *phase) { (void)pwm; (void)phase; }
void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *pulse) { (void)pwm; (void)pulse; }
void IfxGtm_Pwm_interruptHandler(IfxGtm_Pwm *pwm) { (void)pwm; }
IfxGtm_Pwm_ChannelState IfxGtm_Pwm_getChannelState(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch) { (void)pwm; (void)ch; return IfxGtm_Pwm_ChannelState_running; }
void IfxGtm_Pwm_stopChannelOutputs(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch) { (void)pwm; (void)ch; }
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch) { (void)pwm; (void)ch; }

/* Mock control API definitions */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void) {
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
}

int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }
