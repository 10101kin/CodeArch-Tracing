/*
 * mock_egtm_atom_3_phase_inverter_pwm.c
 * Spy state definitions + stub bodies + MODULE_* instances
 */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/*====================================================*/
/* Spy globals initialization                          */
/*====================================================*/
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxEgtm_enable_callCount = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
int     mock_IfxEgtm_Cmu_selectClkInput_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
int     mock_IfxPort_setPinModeOutput_callCount = 0;
uint32  mock_togglePin_callCount = 0u;

boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/*====================================================*/
/* MODULE_* instances                                  */
/*====================================================*/
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P MODULE_P00 = {0};
Ifx_ADC MODULE_ADC = {0};
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
Ifx_ASCLIN24 MODULE_ASCLIN24 = {0};
Ifx_ASCLIN25 MODULE_ASCLIN25 = {0};
Ifx_ASCLIN26 MODULE_ASCLIN26 = {0};
Ifx_ASCLIN27 MODULE_ASCLIN27 = {0};
Ifx_TBCU MODULE_TBCU = {0};
Ifx_CSBCU MODULE_CSBCU = {0};
Ifx_SBCU MODULE_SBCU = {0};
Ifx_COMBCU MODULE_COMBCU = {0};
Ifx_CANXL0 MODULE_CANXL0 = {0};
Ifx_CANXL0_RAM MODULE_CANXL0_RAM = {0};
Ifx_CAN0 MODULE_CAN0 = {0};
Ifx_CAN0_RAM MODULE_CAN0_RAM = {0};
Ifx_CAN1 MODULE_CAN1 = {0};
Ifx_CAN1_RAM MODULE_CAN1_RAM = {0};
Ifx_CAN2 MODULE_CAN2 = {0};
Ifx_CAN2_RAM MODULE_CAN2_RAM = {0};
Ifx_CAN3 MODULE_CAN3 = {0};
Ifx_CAN3_RAM MODULE_CAN3_RAM = {0};
Ifx_CAN4 MODULE_CAN4 = {0};
Ifx_CAN4_RAM MODULE_CAN4_RAM = {0};
Ifx_CBS MODULE_CBS = {0};
Ifx_CLOCK MODULE_CLOCK = {0};
Ifx_CPU_CFI0 MODULE_CPU_CFI0 = {0};
Ifx_CPU_CFI1 MODULE_CPU_CFI1 = {0};
Ifx_CPU_CFI2 MODULE_CPU_CFI2 = {0};
Ifx_CPU_CFI3 MODULE_CPU_CFI3 = {0};
Ifx_CPU_CFI4 MODULE_CPU_CFI4 = {0};
Ifx_CPU_CFI5 MODULE_CPU_CFI5 = {0};
Ifx_CPU_CFICS MODULE_CPU_CFICS = {0};
Ifx_CPU0 MODULE_CPU0 = {0};
Ifx_CPU1 MODULE_CPU1 = {0};
Ifx_CPU2 MODULE_CPU2 = {0};
Ifx_CPU3 MODULE_CPU3 = {0};
Ifx_CPU4 MODULE_CPU4 = {0};
Ifx_CPU5 MODULE_CPU5 = {0};
Ifx_CPUCS MODULE_CPUCS = {0};
Ifx_DMA0 MODULE_DMA0 = {0};
Ifx_DMA1 MODULE_DMA1 = {0};
Ifx_DMU MODULE_DMU = {0};
Ifx_DOM0 MODULE_DOM0 = {0};
Ifx_DOM1 MODULE_DOM1 = {0};
Ifx_DOM3 MODULE_DOM3 = {0};
Ifx_DOM6 MODULE_DOM6 = {0};
Ifx_DOM7 MODULE_DOM7 = {0};
Ifx_DOM4 MODULE_DOM4 = {0};
Ifx_DOM2 MODULE_DOM2 = {0};
Ifx_DOM5 MODULE_DOM5 = {0};
Ifx_DRE MODULE_DRE = {0};
Ifx_ERAY0 MODULE_ERAY0 = {0};
Ifx_ERAY1 MODULE_ERAY1 = {0};
Ifx_FCE MODULE_FCE = {0};
Ifx_FSI_CSRM MODULE_FSI_CSRM = {0};
Ifx_FSI_HOST MODULE_FSI_HOST = {0};
Ifx_GETH0 MODULE_GETH0 = {0};
Ifx_HSCT0 MODULE_HSCT0 = {0};
Ifx_HSCT1 MODULE_HSCT1 = {0};
Ifx_HSPHY MODULE_HSPHY = {0};
Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA = {0};
Ifx_HSSL0 MODULE_HSSL0 = {0};
Ifx_HSSL1 MODULE_HSSL1 = {0};
Ifx_I2C0 MODULE_I2C0 = {0};
Ifx_I2C1 MODULE_I2C1 = {0};
Ifx_I2C2 MODULE_I2C2 = {0};
Ifx_INT MODULE_INT = {0};
Ifx_LETH0 MODULE_LETH0 = {0};
Ifx_LLI0 MODULE_LLI0 = {0};
Ifx_LMU0 MODULE_LMU0 = {0};
Ifx_LMU1 MODULE_LMU1 = {0};
Ifx_LMU2 MODULE_LMU2 = {0};
Ifx_LMU3 MODULE_LMU3 = {0};
Ifx_LMU4 MODULE_LMU4 = {0};
Ifx_LMU5 MODULE_LMU5 = {0};
Ifx_LMU6 MODULE_LMU6 = {0};
Ifx_LMU7 MODULE_LMU7 = {0};
Ifx_LMU8 MODULE_LMU8 = {0};
Ifx_LMU9 MODULE_LMU9 = {0};
Ifx_MCDS2P MODULE_MCDS2P = {0};
Ifx_MCDS4P MODULE_MCDS4P = {0};
Ifx_MSC0 MODULE_MSC0 = {0};
Ifx_PCIE0_DSP MODULE_PCIE0_DSP = {0};
Ifx_PCIE0_DSP_SRI MODULE_PCIE0_DSP_SRI = {0};
Ifx_PCIE1_DSP MODULE_PCIE1_DSP = {0};
Ifx_PCIE1_DSP_SRI MODULE_PCIE1_DSP_SRI = {0};
Ifx_PCIE0_USP MODULE_PCIE0_USP = {0};
Ifx_PCIE0_USP_SRI MODULE_PCIE0_USP_SRI = {0};
Ifx_PCIE1_USP MODULE_PCIE1_USP = {0};
Ifx_PCIE1_USP_SRI MODULE_PCIE1_USP_SRI = {0};
Ifx_PFRWB0A MODULE_PFRWB0A = {0};
Ifx_PFRWB0B MODULE_PFRWB0B = {0};
Ifx_PFRWB1A MODULE_PFRWB1A = {0};
Ifx_PFRWB1B MODULE_PFRWB1B = {0};
Ifx_PFRWB2A MODULE_PFRWB2A = {0};
Ifx_PFRWB2B MODULE_PFRWB2B = {0};
Ifx_PFRWB3A MODULE_PFRWB3A = {0};
Ifx_PFRWB3B MODULE_PFRWB3B = {0};
Ifx_PFRWB4A MODULE_PFRWB4A = {0};
Ifx_PFRWB4B MODULE_PFRWB4B = {0};
Ifx_PFRWB5A MODULE_PFRWB5A = {0};
Ifx_PFRWB5B MODULE_PFRWB5B = {0};
Ifx_PFRWBCS MODULE_PFRWBCS = {0};
Ifx_PMS MODULE_PMS = {0};
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
Ifx_P25 MODULE_P25 = {0};
Ifx_P30 MODULE_P30 = {0};
Ifx_P31 MODULE_P31 = {0};
Ifx_P32 MODULE_P32 = {0};
Ifx_P MODULE_P33 = {0};
Ifx_P MODULE_P34 = {0};
Ifx_P35 MODULE_P35 = {0};
Ifx_P MODULE_P40 = {0};
Ifx_P MODULE_P41 = {0};
Ifx_PPU MODULE_PPU = {0};
Ifx_PPU_STUDMI MODULE_PPU_STUDMI = {0};
Ifx_PPU_DEBUG MODULE_PPU_DEBUG = {0};
Ifx_PPU_SM MODULE_PPU_SM = {0};
Ifx_PPU_APU MODULE_PPU_APU = {0};
Ifx_PPU_CSMAP MODULE_PPU_CSMAP = {0};
Ifx_PPU_VMEMAP MODULE_PPU_VMEMAP = {0};
Ifx_PSI5S0 MODULE_PSI5S0 = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI0 MODULE_QSPI0 = {0};
Ifx_QSPI1 MODULE_QSPI1 = {0};
Ifx_QSPI2 MODULE_QSPI2 = {0};
Ifx_QSPI3 MODULE_QSPI3 = {0};
Ifx_QSPI4 MODULE_QSPI4 = {0};
Ifx_QSPI5 MODULE_QSPI5 = {0};
Ifx_QSPI6 MODULE_QSPI6 = {0};
Ifx_QSPI7 MODULE_QSPI7 = {0};
Ifx_SCU MODULE_SCU = {0};
Ifx_SDMMC0 MODULE_SDMMC0 = {0};
Ifx_SENT0 MODULE_SENT0 = {0};
Ifx_SENT1 MODULE_SENT1 = {0};
Ifx_SMM MODULE_SMM = {0};
Ifx_SMU MODULE_SMU = {0};
Ifx_SMUSTDBY MODULE_SMUSTDBY = {0};
Ifx_SRC MODULE_SRC = {0};
Ifx_TRIF MODULE_TRIF = {0};
Ifx_TRI MODULE_TRI = {0};
Ifx_VMT0 MODULE_VMT0 = {0};
Ifx_VMT1 MODULE_VMT1 = {0};
Ifx_VMT2 MODULE_VMT2 = {0};
Ifx_VMT3 MODULE_VMT3 = {0};
Ifx_VMT4 MODULE_VMT4 = {0};
Ifx_VMT5 MODULE_VMT5 = {0};
Ifx_VMT6 MODULE_VMT6 = {0};
Ifx_VTMON MODULE_VTMON = {0};
Ifx_WTU MODULE_WTU = {0};
Ifx_XSPI0 MODULE_XSPI0 = {0};

/*====================================================*/
/* Stub bodies (never access struct members)           */
/*====================================================*/
/* IfxEgtm_Pwm stubs */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    if (requestDuty != NULL_PTR)
    {
        for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

/* Provide no-op stubs for the rest of declared PWM APIs (never used in tests, return defaults) */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), sint32 vectabNum, Ifx_Priority priority) { (void)isr; (void)vectabNum; (void)priority; }
void IfxCpu_enableInterrupts(void) { }
void IfxEgtm_Pwm_updateFrequency(IfxEgtm_Pwm *pwm, float32 freq) { (void)pwm; (void)freq; }
void IfxEgtm_Pwm_updateChannelPhase(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxEgtm_Pwm_updateChannelPhaseImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase) { (void)pwm; (void)ch; (void)phase; }
void IfxEgtm_Pwm_updateChannelsPhase(IfxEgtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; }
void IfxEgtm_Pwm_updateChannelsPhaseImmediate(IfxEgtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; }
void IfxEgtm_Pwm_updateChannelDuty(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxEgtm_Pwm_updateChannelDutyImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty) { (void)pwm; (void)ch; (void)duty; }
void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *duties) { (void)pwm; (void)duties; }
void IfxEgtm_Pwm_startSyncedGroups(IfxEgtm_Pwm *pwm, uint32 groupMask) { (void)pwm; (void)groupMask; }
void IfxEgtm_Pwm_updateSyncedGroupsFrequency(IfxEgtm_Pwm *pwm, uint32 groupMask, float32 freq) { (void)pwm; (void)groupMask; (void)freq; }
void IfxEgtm_Pwm_setChannelPolarity(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, Ifx_ActiveState pol) { (void)pwm; (void)ch; (void)pol; }
void IfxEgtm_Pwm_updateChannelDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall) { (void)pwm; (void)ch; (void)rise; (void)fall; }
void IfxEgtm_Pwm_updateChannelDeadTimeImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall)
{ (void)pwm; (void)ch; mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[0] = rise; mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[0] = fall; }
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm, uint32 chMask) { (void)pwm; (void)chMask; }
void IfxEgtm_Pwm_stopSyncedChannels(IfxEgtm_Pwm *pwm, uint32 chMask) { (void)pwm; (void)chMask; }
void IfxEgtm_Pwm_stopSyncedGroups(IfxEgtm_Pwm *pwm, uint32 groupMask) { (void)pwm; (void)groupMask; }
void IfxEgtm_Pwm_updateFrequencyImmediate(IfxEgtm_Pwm *pwm, float32 freq) { (void)pwm; (void)freq; }
void IfxEgtm_Pwm_updateChannelPulse(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 pulse) { (void)pwm; (void)ch; (void)pulse; }
void IfxEgtm_Pwm_updateChannelPulseImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 pulse) { (void)pwm; (void)ch; (void)pulse; }
void IfxEgtm_Pwm_updateChannelsPulse(IfxEgtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; }
void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, float32 *rise, float32 *fall) { (void)pwm; (void)rise; (void)fall; }
void IfxEgtm_Pwm_updateChannelsPhaseImmediate(IfxEgtm_Pwm *pwm, float32 *phases) { (void)pwm; (void)phases; }
void IfxEgtm_Pwm_updateChannelsDutyImmediateAlias(IfxEgtm_Pwm *pwm, float32 *duties) { (void)pwm; (void)duties; }
void IfxEgtm_Pwm_updateChannelsPulseImmediate(IfxEgtm_Pwm *pwm, float32 *pulses) { (void)pwm; (void)pulses; }
void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm *pwm) { (void)pwm; }
IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch) { (void)pwm; (void)ch; return IfxEgtm_Pwm_ChannelState_running; }
void IfxEgtm_Pwm_stopChannelOutputs(IfxEgtm_Pwm *pwm, uint32 chMask) { (void)pwm; (void)chMask; }
void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm, uint32 chMask) { (void)pwm; (void)chMask; }
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *cfg) { (void)cfg; }

/* IfxEgtm (module) stubs */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_isEnabled_callCount++; return mock_IfxEgtm_isEnabled_returnValue; }
void IfxEgtm_enable(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_enable_callCount++; }
void IfxEgtm_disable(Ifx_EGTM *egtm) { (void)egtm; }
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode) { (void)egtm; (void)mode; }
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm) { (void)egtm; return 0.0f; }
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm) { (void)egtm; return 0.0f; }
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, uint32 ticks) { (void)egtm; (void)ticks; return 0.0f; }
uint32  IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds) { (void)egtm; (void)seconds; return 0u; }
void IfxEgtm_initProtSe(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_initCtrlProt(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_initClApu(Ifx_EGTM *egtm, IfxEgtm_ClApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, IfxEgtm_CtrlApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initWrapApu(Ifx_EGTM *egtm, IfxEgtm_WrapApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIdx, IfxEgtm_ClusterClockDiv div) { (void)egtm; (void)clusterIdx; (void)div; }
void IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 clusterIdx) { (void)egtm; (void)clusterIdx; }
void IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
void IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 ch, boolean enable) { (void)egtm; (void)ch; (void)enable; }
void IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode) { (void)egtm; (void)mode; }
void IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, IfxEgtm_MscOut *msc) { (void)egtm; (void)msc; }
void IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg) { (void)cfg; }
void IfxEgtm_initAp(Ifx_EGTM *egtm, IfxEgtm_ApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_configureAccessToEgtms(void) { }
float32 IfxClock_geteGtmFrequency(void) { return 0.0f; }

/* IfxEgtm_Cmu stubs */
void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal) { (void)egtm; (void)clkIndex; (void)useGlobal; mock_IfxEgtm_Cmu_selectClkInput_callCount++; }
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask) { (void)egtm; (void)clkMask; mock_IfxEgtm_Cmu_enableClocks_callCount++; }
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++; return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }
void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator) { (void)egtm; (void)numerator; (void)denominator; mock_IfxEgtm_Cmu_setGclkDivider_callCount++; }
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm) { (void)egtm; return 0.0f; }
boolean IfxEgtm_Cmu_isClkClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk) { (void)egtm; (void)clk; return FALSE; }
boolean IfxEgtm_Cmu_isEclkClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk eclk) { (void)egtm; (void)eclk; return FALSE; }
boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Fxclk fx) { (void)egtm; (void)fx; return FALSE; }
void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, uint32 val) { (void)egtm; (void)clk; (void)val; }
void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk eclk, uint32 div) { (void)egtm; (void)eclk; (void)div; }
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 freq) { (void)egtm; (void)freq; }
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled) { (void)egtm; (void)clkIndex; (void)assumeEnabled; return 0.0f; }
float32 IfxEgtm_Cmu_getEclkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk eclk) { (void)egtm; (void)eclk; return 0.0f; }
float32 IfxEgtm_Cmu_getFxClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Fxclk fx) { (void)egtm; (void)fx; return 0.0f; }
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency) { (void)egtm; (void)clkIndex; (void)frequency; }
void IfxEgtm_Cmu_setEclkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk eclkIndex, float32 frequency) { (void)egtm; (void)eclkIndex; (void)frequency; }

/* IfxPort stubs */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{ (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_togglePin_callCount++; }

/*====================================================*/
/* Spy API getters and reset                           */
/*====================================================*/
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void)              { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int  mock_IfxEgtm_Pwm_init_getCallCount(void)                    { return mock_IfxEgtm_Pwm_init_callCount; }
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int  mock_IfxEgtm_enable_getCallCount(void)                      { return mock_IfxEgtm_enable_callCount; }
int  mock_IfxEgtm_isEnabled_getCallCount(void)                   { return mock_IfxEgtm_isEnabled_callCount; }
int  mock_IfxEgtm_Cmu_selectClkInput_getCallCount(void)          { return mock_IfxEgtm_Cmu_selectClkInput_callCount; }
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void)            { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void)      { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void)          { return mock_IfxEgtm_Cmu_setGclkDivider_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void)            { return mock_IfxPort_setPinModeOutput_callCount; }

void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_selectClkInput_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
}
