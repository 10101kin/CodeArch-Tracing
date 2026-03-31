#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* Spy state definitions */
int mock_IfxGtm_isEnabled_callCount = 0;
boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
int mock_IfxGtm_enable_callCount = 0;
int mock_IfxGtm_isModuleSuspended_callCount = 0;
boolean mock_IfxGtm_isModuleSuspended_returnValue = FALSE;
int mock_IfxGtm_setSuspendMode_callCount = 0;
int mock_IfxGtm_disable_callCount = 0;
int mock_IfxGtm_getSysClkFrequency_callCount = 0;
float32 mock_IfxGtm_getSysClkFrequency_returnValue = 0.0f;
int mock_IfxGtm_getClusterFrequency_callCount = 0;
float32 mock_IfxGtm_getClusterFrequency_returnValue = 0.0f;

int mock_IfxGtm_Cmu_enable_callCount = 0;
int mock_IfxGtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
int mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int mock_IfxGtm_Cmu_getClkFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getClkFrequency_returnValue = 0.0f;
int mock_IfxGtm_Cmu_getEclkFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getEclkFrequency_returnValue = 0.0f;
int mock_IfxGtm_Cmu_getFxClkFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = 0.0f;
int mock_IfxGtm_Cmu_getGclkFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getGclkFrequency_returnValue = 0.0f;
int mock_IfxGtm_Cmu_isClkClockEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isClkClockEnabled_returnValue = FALSE;
int mock_IfxGtm_Cmu_isEclkClockEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue = FALSE;
int mock_IfxGtm_Cmu_isFxClockEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = FALSE;
int mock_IfxGtm_Cmu_selectClkInput_callCount = 0;
int mock_IfxGtm_Cmu_setEclkFrequency_callCount = 0;

int mock_IfxGtm_Pwm_init_callCount = 0;
int mock_IfxGtm_Pwm_initConfig_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateFrequency_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
int mock_IfxGtm_Pwm_setChannelPolarity_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelPhase_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelPhaseImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelDuty_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelDutyImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount = 0;
int mock_IfxGtm_Pwm_initChannelConfig_callCount = 0;
int mock_IfxGtm_Pwm_startSyncedChannels_callCount = 0;
int mock_IfxGtm_Pwm_stopSyncedChannels_callCount = 0;
int mock_IfxGtm_Pwm_startSyncedGroups_callCount = 0;
int mock_IfxGtm_Pwm_stopSyncedGroups_callCount = 0;
int mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_callCount = 0;
int mock_IfxGtm_Pwm_updateFrequencyImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelPulse_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelPulseImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsPhase_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsPulse_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsPulseImmediate_callCount = 0;
int mock_IfxGtm_Pwm_interruptHandler_callCount = 0;
int mock_IfxGtm_Pwm_getChannelState_callCount = 0;
int mock_IfxGtm_Pwm_stopChannelOutputs_callCount = 0;
int mock_IfxGtm_Pwm_startChannelOutputs_callCount = 0;
int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

uint32 mock_IfxGtm_Pwm_getChannelState_returnValue = 0u;

int mock_togglePin_callCount = 0;

uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Internal captured channel count for bounded copying */
static uint32 _captured_numChannels = 0u;

/* MODULE_* instance definitions */
Ifx_ASCLIN MODULE_ASCLIN0 = {0};
Ifx_ASCLIN MODULE_ASCLIN1 = {0};
Ifx_ASCLIN MODULE_ASCLIN2 = {0};
Ifx_ASCLIN MODULE_ASCLIN3 = {0};
Ifx_ASCLIN MODULE_ASCLIN4 = {0};
Ifx_ASCLIN MODULE_ASCLIN5 = {0};
Ifx_ASCLIN MODULE_ASCLIN6 = {0};
Ifx_ASCLIN MODULE_ASCLIN7 = {0};
Ifx_ASCLIN MODULE_ASCLIN8 = {0};
Ifx_ASCLIN MODULE_ASCLIN9 = {0};
Ifx_ASCLIN MODULE_ASCLIN10 = {0};
Ifx_ASCLIN MODULE_ASCLIN11 = {0};
Ifx_ASCLIN MODULE_ASCLIN12 = {0};
Ifx_ASCLIN MODULE_ASCLIN13 = {0};
Ifx_ASCLIN MODULE_ASCLIN14 = {0};
Ifx_ASCLIN MODULE_ASCLIN15 = {0};
Ifx_ASCLIN MODULE_ASCLIN16 = {0};
Ifx_ASCLIN MODULE_ASCLIN17 = {0};
Ifx_ASCLIN MODULE_ASCLIN18 = {0};
Ifx_ASCLIN MODULE_ASCLIN19 = {0};
Ifx_ASCLIN MODULE_ASCLIN20 = {0};
Ifx_ASCLIN MODULE_ASCLIN21 = {0};
Ifx_ASCLIN MODULE_ASCLIN22 = {0};
Ifx_ASCLIN MODULE_ASCLIN23 = {0};
Ifx_CAN MODULE_CAN0 = {0};
Ifx_CAN MODULE_CAN1 = {0};
Ifx_CAN MODULE_CAN2 = {0};
Ifx_CBS MODULE_CBS = {0};
Ifx_CCU6 MODULE_CCU60 = {0};
Ifx_CCU6 MODULE_CCU61 = {0};
Ifx_CONVCTRL MODULE_CONVCTRL = {0};
Ifx_CPU MODULE_CPU0 = {0};
Ifx_CPU MODULE_CPU1 = {0};
Ifx_CPU MODULE_CPU2 = {0};
Ifx_CPU MODULE_CPU3 = {0};
Ifx_DAM MODULE_DAM0 = {0};
Ifx_DMA MODULE_DMA = {0};
Ifx_DMU MODULE_DMU = {0};
Ifx_DOM MODULE_DOM0 = {0};
Ifx_EDSADC MODULE_EDSADC = {0};
Ifx_ERAY MODULE_ERAY0 = {0};
Ifx_ERAY MODULE_ERAY1 = {0};
Ifx_EVADC MODULE_EVADC = {0};
Ifx_FCE MODULE_FCE = {0};
Ifx_FSI MODULE_FSI = {0};
Ifx_GETH MODULE_GETH = {0};
Ifx_GPT120 MODULE_GPT120 = {0};
Ifx_GTM MODULE_GTM = {0};
Ifx_HSCT MODULE_HSCT0 = {0};
Ifx_HSSL MODULE_HSSL0 = {0};
Ifx_I2C MODULE_I2C0 = {0};
Ifx_I2C MODULE_I2C1 = {0};
Ifx_INT MODULE_INT = {0};
Ifx_IOM MODULE_IOM = {0};
Ifx_LMU MODULE_LMU0 = {0};
Ifx_MINIMCDS MODULE_MINIMCDS = {0};
Ifx_MSC MODULE_MSC0 = {0};
Ifx_MSC MODULE_MSC1 = {0};
Ifx_MSC MODULE_MSC2 = {0};
Ifx_MTU MODULE_MTU = {0};
Ifx_PFI MODULE_PFI0 = {0};
Ifx_PFI MODULE_PFI1 = {0};
Ifx_PFI MODULE_PFI2 = {0};
Ifx_PFI MODULE_PFI3 = {0};
Ifx_PMS MODULE_PMS = {0};
Ifx_PMU MODULE_PMU = {0};
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
Ifx_PSI5S MODULE_PSI5S = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI MODULE_QSPI0 = {0};
Ifx_QSPI MODULE_QSPI1 = {0};
Ifx_QSPI MODULE_QSPI2 = {0};
Ifx_QSPI MODULE_QSPI3 = {0};
Ifx_QSPI MODULE_QSPI4 = {0};
Ifx_SBCU MODULE_SBCU = {0};
Ifx_SCU MODULE_SCU = {0};
Ifx_SENT MODULE_SENT = {0};
Ifx_SMU MODULE_SMU = {0};
Ifx_SRC MODULE_SRC = {0};
Ifx_STM MODULE_STM0 = {0};
Ifx_STM MODULE_STM1 = {0};
Ifx_STM MODULE_STM2 = {0};
Ifx_STM MODULE_STM3 = {0};

/* Required pin symbol instances */
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1N_TOUT14_P00_5_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2N_TOUT15_P00_6_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3N_TOUT16_P00_7_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_5_TOUT11_P00_2_OUT = {0};

/* ---- Stubs: IfxGtm ---- */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_isEnabled_callCount++; return mock_IfxGtm_isEnabled_returnValue; }
void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_isModuleSuspended_callCount++; return mock_IfxGtm_isModuleSuspended_returnValue; }
void IfxGtm_setSuspendMode(Ifx_GTM *gtm, uint32 mode) { (void)gtm; (void)mode; mock_IfxGtm_setSuspendMode_callCount++; }
void IfxGtm_disable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_disable_callCount++; }
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_getSysClkFrequency_callCount++; return (mock_IfxGtm_getSysClkFrequency_returnValue != 0.0f) ? mock_IfxGtm_getSysClkFrequency_returnValue : 100000000.0f; }
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, uint32 cluster) { (void)gtm; (void)cluster; mock_IfxGtm_getClusterFrequency_callCount++; return (mock_IfxGtm_getClusterFrequency_returnValue != 0.0f) ? mock_IfxGtm_getClusterFrequency_returnValue : 100000000.0f; }

/* ---- Stubs: IfxGtm_Cmu ---- */
void IfxGtm_Cmu_enable(Ifx_GTM *module) { (void)module; mock_IfxGtm_Cmu_enable_callCount++; }
boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module) { (void)module; mock_IfxGtm_Cmu_isEnabled_callCount++; return mock_IfxGtm_Cmu_isEnabled_returnValue; }
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *module) { (void)module; mock_IfxGtm_Cmu_getModuleFrequency_callCount++; return (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *module, float32 frequency) { (void)module; (void)frequency; mock_IfxGtm_Cmu_setGclkFrequency_callCount++; }
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *module, IfxGtm_Cmu_Clk clk, float32 frequency) { (void)module; (void)clk; (void)frequency; mock_IfxGtm_Cmu_setClkFrequency_callCount++; }
void IfxGtm_Cmu_enableClocks(Ifx_GTM *module, uint32 mask) { (void)module; (void)mask; mock_IfxGtm_Cmu_enableClocks_callCount++; }
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, boolean assumeEnabled) { (void)gtm; (void)clkIndex; (void)assumeEnabled; mock_IfxGtm_Cmu_getClkFrequency_callCount++; return (mock_IfxGtm_Cmu_getClkFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getClkFrequency_returnValue : 100000000.0f; }
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclkIndex, boolean assumeEnabled) { (void)gtm; (void)eclkIndex; (void)assumeEnabled; mock_IfxGtm_Cmu_getEclkFrequency_callCount++; return (mock_IfxGtm_Cmu_getEclkFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getEclkFrequency_returnValue : 100000000.0f; }
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxIndex, boolean assumeEnabled) { (void)gtm; (void)fxIndex; (void)assumeEnabled; mock_IfxGtm_Cmu_getFxClkFrequency_callCount++; return (mock_IfxGtm_Cmu_getFxClkFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getFxClkFrequency_returnValue : 100000000.0f; }
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_Cmu_getGclkFrequency_callCount++; return (mock_IfxGtm_Cmu_getGclkFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getGclkFrequency_returnValue : 100000000.0f; }
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex) { (void)gtm; (void)clkIndex; mock_IfxGtm_Cmu_isClkClockEnabled_callCount++; return mock_IfxGtm_Cmu_isClkClockEnabled_returnValue; }
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclkIndex) { (void)gtm; (void)eclkIndex; mock_IfxGtm_Cmu_isEclkClockEnabled_callCount++; return mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue; }
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxIndex) { (void)gtm; (void)fxIndex; mock_IfxGtm_Cmu_isFxClockEnabled_callCount++; return mock_IfxGtm_Cmu_isFxClockEnabled_returnValue; }
void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, uint32 clkSel) { (void)gtm; (void)clkIndex; (void)clkSel; mock_IfxGtm_Cmu_selectClkInput_callCount++; }
void IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclkIndex, float32 frequency) { (void)gtm; (void)eclkIndex; (void)frequency; mock_IfxGtm_Cmu_setEclkFrequency_callCount++; }

/* ---- Stubs: IfxGtm_Pwm ---- */
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)gtmSFR; mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency = config->frequency;
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm; (void)channels; mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm; mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR) {
        for (uint32 i = 0; i < n; ++i) {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; mock_IfxGtm_Pwm_updateFrequency_callCount++; }
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *duties) { (void)pwm; (void)duties; mock_IfxGtm_Pwm_updateChannelsDuty_callCount++; }
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, uint32 channelIndex, Ifx_ActiveState polarity) { (void)pwm; (void)channelIndex; (void)polarity; mock_IfxGtm_Pwm_setChannelPolarity_callCount++; }
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 phase) { (void)pwm; (void)channelIndex; (void)phase; mock_IfxGtm_Pwm_updateChannelPhase_callCount++; }
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 phase) { (void)pwm; (void)channelIndex; (void)phase; mock_IfxGtm_Pwm_updateChannelPhaseImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 duty) { (void)pwm; (void)channelIndex; (void)duty; mock_IfxGtm_Pwm_updateChannelDuty_callCount++; }
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 duty) { (void)pwm; (void)channelIndex; (void)duty; mock_IfxGtm_Pwm_updateChannelDutyImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 dtRising, float32 dtFalling) {
    (void)pwm; (void)channelIndex; mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i) {
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = dtRising;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = dtFalling;
    }
}
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *chCfg, Ifx_GTM *gtm) { (void)chCfg; (void)gtm; mock_IfxGtm_Pwm_initChannelConfig_callCount++; }
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; mock_IfxGtm_Pwm_startSyncedChannels_callCount++; }
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; mock_IfxGtm_Pwm_stopSyncedChannels_callCount++; }
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; mock_IfxGtm_Pwm_startSyncedGroups_callCount++; }
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask) { (void)pwm; (void)mask; mock_IfxGtm_Pwm_stopSyncedGroups_callCount++; }
void IfxGtm_Pwm_updateSyncedGroupsFrequency(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_callCount++; }
void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 frequency) { (void)pwm; (void)frequency; mock_IfxGtm_Pwm_updateFrequencyImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelPulse(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 pulse) { (void)pwm; (void)channelIndex; (void)pulse; mock_IfxGtm_Pwm_updateChannelPulse_callCount++; }
void IfxGtm_Pwm_updateChannelPulseImmediate(IfxGtm_Pwm *pwm, uint32 channelIndex, float32 pulse) { (void)pwm; (void)channelIndex; (void)pulse; mock_IfxGtm_Pwm_updateChannelPulseImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelsPhase(IfxGtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; mock_IfxGtm_Pwm_updateChannelsPhase_callCount++; }
void IfxGtm_Pwm_updateChannelsPulse(IfxGtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; mock_IfxGtm_Pwm_updateChannelsPulse_callCount++; }
void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, float32 *rise, float32 *fall) { (void)pwm; (void)rise; (void)fall; mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelsPhaseImmediate(IfxGtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_callCount++; }
void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; mock_IfxGtm_Pwm_updateChannelsPulseImmediate_callCount++; }
void IfxGtm_Pwm_interruptHandler(IfxGtm_Pwm *pwm) { (void)pwm; mock_IfxGtm_Pwm_interruptHandler_callCount++; }
uint32 IfxGtm_Pwm_getChannelState(IfxGtm_Pwm *pwm, uint32 channelIndex) { (void)pwm; (void)channelIndex; mock_IfxGtm_Pwm_getChannelState_callCount++; return mock_IfxGtm_Pwm_getChannelState_returnValue; }
void IfxGtm_Pwm_stopChannelOutputs(IfxGtm_Pwm *pwm) { (void)pwm; mock_IfxGtm_Pwm_stopChannelOutputs_callCount++; }
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm) { (void)pwm; mock_IfxGtm_Pwm_startChannelOutputs_callCount++; }

/* ---- Stubs: IfxCpu Irq ---- */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), Ifx_Priority priority) { (void)isr; (void)priority; mock_IfxCpu_Irq_installInterruptHandler_callCount++; }

/* ---- Getters ---- */
int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_isModuleSuspended_getCallCount(void) { return mock_IfxGtm_isModuleSuspended_callCount; }
int mock_IfxGtm_setSuspendMode_getCallCount(void) { return mock_IfxGtm_setSuspendMode_callCount; }
int mock_IfxGtm_disable_getCallCount(void) { return mock_IfxGtm_disable_callCount; }
int mock_IfxGtm_getSysClkFrequency_getCallCount(void) { return mock_IfxGtm_getSysClkFrequency_callCount; }
int mock_IfxGtm_getClusterFrequency_getCallCount(void) { return mock_IfxGtm_getClusterFrequency_callCount; }

int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Cmu_getClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getClkFrequency_callCount; }
int mock_IfxGtm_Cmu_getEclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getEclkFrequency_callCount; }
int mock_IfxGtm_Cmu_getFxClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getFxClkFrequency_callCount; }
int mock_IfxGtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_isClkClockEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isClkClockEnabled_callCount; }
int mock_IfxGtm_Cmu_isEclkClockEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEclkClockEnabled_callCount; }
int mock_IfxGtm_Cmu_isFxClockEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isFxClockEnabled_callCount; }
int mock_IfxGtm_Cmu_selectClkInput_getCallCount(void) { return mock_IfxGtm_Cmu_selectClkInput_callCount; }
int mock_IfxGtm_Cmu_setEclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setEclkFrequency_callCount; }

int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxGtm_Pwm_updateFrequency_getCallCount(void) { return mock_IfxGtm_Pwm_updateFrequency_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDuty_callCount; }
int mock_IfxGtm_Pwm_setChannelPolarity_getCallCount(void) { return mock_IfxGtm_Pwm_setChannelPolarity_callCount; }
int mock_IfxGtm_Pwm_updateChannelPhase_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelPhase_callCount; }
int mock_IfxGtm_Pwm_updateChannelPhaseImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelPhaseImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelDuty_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelDuty_callCount; }
int mock_IfxGtm_Pwm_updateChannelDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelDutyImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount; }
int mock_IfxGtm_Pwm_initChannelConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initChannelConfig_callCount; }
int mock_IfxGtm_Pwm_startSyncedChannels_getCallCount(void) { return mock_IfxGtm_Pwm_startSyncedChannels_callCount; }
int mock_IfxGtm_Pwm_stopSyncedChannels_getCallCount(void) { return mock_IfxGtm_Pwm_stopSyncedChannels_callCount; }
int mock_IfxGtm_Pwm_startSyncedGroups_getCallCount(void) { return mock_IfxGtm_Pwm_startSyncedGroups_callCount; }
int mock_IfxGtm_Pwm_stopSyncedGroups_getCallCount(void) { return mock_IfxGtm_Pwm_stopSyncedGroups_callCount; }
int mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_getCallCount(void) { return mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_callCount; }
int mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateFrequencyImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelPulse_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelPulse_callCount; }
int mock_IfxGtm_Pwm_updateChannelPulseImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelPulseImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelsPhase_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsPhase_callCount; }
int mock_IfxGtm_Pwm_updateChannelsPulse_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsPulse_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_callCount; }
int mock_IfxGtm_Pwm_updateChannelsPulseImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsPulseImmediate_callCount; }
int mock_IfxGtm_Pwm_interruptHandler_getCallCount(void) { return mock_IfxGtm_Pwm_interruptHandler_callCount; }
int mock_IfxGtm_Pwm_getChannelState_getCallCount(void) { return mock_IfxGtm_Pwm_getChannelState_callCount; }
int mock_IfxGtm_Pwm_stopChannelOutputs_getCallCount(void) { return mock_IfxGtm_Pwm_stopChannelOutputs_callCount; }
int mock_IfxGtm_Pwm_startChannelOutputs_getCallCount(void) { return mock_IfxGtm_Pwm_startChannelOutputs_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }

/* ---- Reset ---- */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void) {
    /* Zero counters */
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_isModuleSuspended_callCount = 0;
    mock_IfxGtm_setSuspendMode_callCount = 0;
    mock_IfxGtm_disable_callCount = 0;
    mock_IfxGtm_getSysClkFrequency_callCount = 0;
    mock_IfxGtm_getClusterFrequency_callCount = 0;

    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_getClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getEclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getFxClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_isClkClockEnabled_callCount = 0;
    mock_IfxGtm_Cmu_isEclkClockEnabled_callCount = 0;
    mock_IfxGtm_Cmu_isFxClockEnabled_callCount = 0;
    mock_IfxGtm_Cmu_selectClkInput_callCount = 0;
    mock_IfxGtm_Cmu_setEclkFrequency_callCount = 0;

    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateFrequency_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
    mock_IfxGtm_Pwm_setChannelPolarity_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelPhase_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelPhaseImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelDuty_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelDutyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount = 0;
    mock_IfxGtm_Pwm_initChannelConfig_callCount = 0;
    mock_IfxGtm_Pwm_startSyncedChannels_callCount = 0;
    mock_IfxGtm_Pwm_stopSyncedChannels_callCount = 0;
    mock_IfxGtm_Pwm_startSyncedGroups_callCount = 0;
    mock_IfxGtm_Pwm_stopSyncedGroups_callCount = 0;
    mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_callCount = 0;
    mock_IfxGtm_Pwm_updateFrequencyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelPulse_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelPulseImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsPhase_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsPulse_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsPulseImmediate_callCount = 0;
    mock_IfxGtm_Pwm_interruptHandler_callCount = 0;
    mock_IfxGtm_Pwm_getChannelState_callCount = 0;
    mock_IfxGtm_Pwm_stopChannelOutputs_callCount = 0;
    mock_IfxGtm_Pwm_startChannelOutputs_callCount = 0;
    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

    mock_togglePin_callCount = 0;

    /* Reset return controls */
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_isModuleSuspended_returnValue = FALSE;
    mock_IfxGtm_getSysClkFrequency_returnValue = 0.0f;
    mock_IfxGtm_getClusterFrequency_returnValue = 0.0f;

    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f; /* default path returns 100MHz */
    mock_IfxGtm_Cmu_getClkFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_getEclkFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_isClkClockEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue = FALSE;

    mock_IfxGtm_Pwm_getChannelState_returnValue = 0u;

    /* Reset spy captured values */
    mock_IfxGtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i) {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0u;
}
