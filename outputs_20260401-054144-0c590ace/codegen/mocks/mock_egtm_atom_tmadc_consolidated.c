/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxPort.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxAdc.h"
#include "IfxPort_Pinmap.h"

/* MODULE instances definitions */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_ADC  MODULE_ADC  = {0};
Ifx_P    MODULE_P00  = {0};
Ifx_P    MODULE_P01  = {0};
Ifx_P    MODULE_P02  = {0};
Ifx_P    MODULE_P03  = {0};
Ifx_P    MODULE_P04  = {0};
Ifx_P    MODULE_P10  = {0};
Ifx_P    MODULE_P13  = {0};
Ifx_P    MODULE_P14  = {0};
Ifx_P    MODULE_P15  = {0};
Ifx_P    MODULE_P16  = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P21  = {0};
Ifx_P    MODULE_P22  = {0};
Ifx_P    MODULE_P23  = {0};
Ifx_P    MODULE_P25  = {0};
Ifx_P    MODULE_P30  = {0};
Ifx_P    MODULE_P31  = {0};
Ifx_P    MODULE_P32  = {0};
Ifx_P    MODULE_P33  = {0};
Ifx_P    MODULE_P34  = {0};
Ifx_P    MODULE_P35  = {0};
Ifx_P    MODULE_P40  = {0};

Ifx_ASCLIN MODULE_ASCLIN0 = {0}; Ifx_ASCLIN MODULE_ASCLIN1 = {0}; Ifx_ASCLIN MODULE_ASCLIN2 = {0}; Ifx_ASCLIN MODULE_ASCLIN3 = {0}; Ifx_ASCLIN MODULE_ASCLIN4 = {0}; Ifx_ASCLIN MODULE_ASCLIN5 = {0}; Ifx_ASCLIN MODULE_ASCLIN6 = {0}; Ifx_ASCLIN MODULE_ASCLIN7 = {0}; Ifx_ASCLIN MODULE_ASCLIN8 = {0}; Ifx_ASCLIN MODULE_ASCLIN9 = {0}; Ifx_ASCLIN MODULE_ASCLIN10 = {0}; Ifx_ASCLIN MODULE_ASCLIN11 = {0}; Ifx_ASCLIN MODULE_ASCLIN12 = {0}; Ifx_ASCLIN MODULE_ASCLIN13 = {0}; Ifx_ASCLIN MODULE_ASCLIN14 = {0}; Ifx_ASCLIN MODULE_ASCLIN15 = {0}; Ifx_ASCLIN MODULE_ASCLIN16 = {0}; Ifx_ASCLIN MODULE_ASCLIN17 = {0}; Ifx_ASCLIN MODULE_ASCLIN18 = {0}; Ifx_ASCLIN MODULE_ASCLIN19 = {0}; Ifx_ASCLIN MODULE_ASCLIN20 = {0}; Ifx_ASCLIN MODULE_ASCLIN21 = {0}; Ifx_ASCLIN MODULE_ASCLIN22 = {0}; Ifx_ASCLIN MODULE_ASCLIN23 = {0}; Ifx_ASCLIN MODULE_ASCLIN24 = {0}; Ifx_ASCLIN MODULE_ASCLIN25 = {0}; Ifx_ASCLIN MODULE_ASCLIN26 = {0}; Ifx_ASCLIN MODULE_ASCLIN27 = {0};
Ifx_BCU MODULE_TBCU = {0}; Ifx_BCU MODULE_CSBCU = {0}; Ifx_BCU MODULE_SBCU = {0}; Ifx_BCU MODULE_COMBCU = {0};
Ifx_CANXL MODULE_CANXL0 = {0}; Ifx_CANXL MODULE_CANXL0_RAM = {0};
Ifx_CAN MODULE_CAN0 = {0}; Ifx_CAN MODULE_CAN0_RAM = {0}; Ifx_CAN MODULE_CAN1 = {0}; Ifx_CAN MODULE_CAN1_RAM = {0}; Ifx_CAN MODULE_CAN2 = {0}; Ifx_CAN MODULE_CAN2_RAM = {0}; Ifx_CAN MODULE_CAN3 = {0}; Ifx_CAN MODULE_CAN3_RAM = {0}; Ifx_CAN MODULE_CAN4 = {0}; Ifx_CAN MODULE_CAN4_RAM = {0};
Ifx_CBS MODULE_CBS = {0};
Ifx_CLOCK MODULE_CLOCK = {0};
Ifx_CPU_CFI MODULE_CPU_CFI0 = {0}; Ifx_CPU_CFI MODULE_CPU_CFI1 = {0}; Ifx_CPU_CFI MODULE_CPU_CFI2 = {0}; Ifx_CPU_CFI MODULE_CPU_CFI3 = {0}; Ifx_CPU_CFI MODULE_CPU_CFI4 = {0}; Ifx_CPU_CFI MODULE_CPU_CFI5 = {0}; Ifx_CPU_CFI MODULE_CPU_CFICS = {0};
Ifx_CPU MODULE_CPU0 = {0}; Ifx_CPU MODULE_CPU1 = {0}; Ifx_CPU MODULE_CPU2 = {0}; Ifx_CPU MODULE_CPU3 = {0}; Ifx_CPU MODULE_CPU4 = {0}; Ifx_CPU MODULE_CPU5 = {0}; Ifx_CPU MODULE_CPUCS = {0};
Ifx_DMA MODULE_DMA0 = {0}; Ifx_DMA MODULE_DMA1 = {0};
Ifx_DMU MODULE_DMU = {0};
Ifx_DOM MODULE_DOM0 = {0}; Ifx_DOM MODULE_DOM1 = {0}; Ifx_DOM MODULE_DOM3 = {0}; Ifx_DOM MODULE_DOM6 = {0}; Ifx_DOM MODULE_DOM7 = {0}; Ifx_DOM MODULE_DOM4 = {0}; Ifx_DOM MODULE_DOM2 = {0}; Ifx_DOM MODULE_DOM5 = {0};
Ifx_DRE MODULE_DRE = {0};
Ifx_ERAY MODULE_ERAY0 = {0}; Ifx_ERAY MODULE_ERAY1 = {0};
Ifx_FCE MODULE_FCE = {0};
Ifx_FSI MODULE_FSI_CSRM = {0}; Ifx_FSI MODULE_FSI_HOST = {0};
Ifx_GETH MODULE_GETH0 = {0};
Ifx_HSCT MODULE_HSCT0 = {0}; Ifx_HSCT MODULE_HSCT1 = {0};
Ifx_HSPHY MODULE_HSPHY = {0}; Ifx_HSPHY MODULE_HSPHY_CRPARA = {0};
Ifx_HSSL MODULE_HSSL0 = {0}; Ifx_HSSL MODULE_HSSL1 = {0};
Ifx_I2C MODULE_I2C0 = {0}; Ifx_I2C MODULE_I2C1 = {0}; Ifx_I2C MODULE_I2C2 = {0};
Ifx_INT MODULE_INT = {0};
Ifx_LETH MODULE_LETH0 = {0};
Ifx_LLI MODULE_LLI0 = {0};
Ifx_LMU MODULE_LMU0 = {0}; Ifx_LMU MODULE_LMU1 = {0}; Ifx_LMU MODULE_LMU2 = {0}; Ifx_LMU MODULE_LMU3 = {0}; Ifx_LMU MODULE_LMU4 = {0}; Ifx_LMU MODULE_LMU5 = {0}; Ifx_LMU MODULE_LMU6 = {0}; Ifx_LMU MODULE_LMU7 = {0}; Ifx_LMU MODULE_LMU8 = {0}; Ifx_LMU MODULE_LMU9 = {0};
Ifx_MCDS MODULE_MCDS2P = {0}; Ifx_MCDS MODULE_MCDS4P = {0};
Ifx_MSC MODULE_MSC0 = {0};
Ifx_PCIE_DSP MODULE_PCIE0_DSP = {0}; Ifx_PCIE_DSP MODULE_PCIE0_DSP_SRI = {0}; Ifx_PCIE_DSP MODULE_PCIE1_DSP = {0}; Ifx_PCIE_DSP MODULE_PCIE1_DSP_SRI = {0};
Ifx_PCIE_USP MODULE_PCIE0_USP = {0}; Ifx_PCIE_USP MODULE_PCIE0_USP_SRI = {0}; Ifx_PCIE_USP MODULE_PCIE1_USP = {0}; Ifx_PCIE_USP MODULE_PCIE1_USP_SRI = {0};
Ifx_PFRWB MODULE_PFRWB0A = {0}; Ifx_PFRWB MODULE_PFRWB0B = {0}; Ifx_PFRWB MODULE_PFRWB1A = {0}; Ifx_PFRWB MODULE_PFRWB1B = {0}; Ifx_PFRWB MODULE_PFRWB2A = {0}; Ifx_PFRWB MODULE_PFRWB2B = {0}; Ifx_PFRWB MODULE_PFRWB3A = {0}; Ifx_PFRWB MODULE_PFRWB3B = {0}; Ifx_PFRWB MODULE_PFRWB4A = {0}; Ifx_PFRWB MODULE_PFRWB4B = {0}; Ifx_PFRWB MODULE_PFRWB5A = {0}; Ifx_PFRWB MODULE_PFRWB5B = {0}; Ifx_PFRWB MODULE_PFRWBCS = {0};
Ifx_PMS MODULE_PMS = {0};
Ifx_PPU MODULE_PPU = {0}; Ifx_PPU MODULE_PPU_STUDMI = {0}; Ifx_PPU MODULE_PPU_DEBUG = {0}; Ifx_PPU MODULE_PPU_SM = {0}; Ifx_PPU MODULE_PPU_APU = {0}; Ifx_PPU MODULE_PPU_CSMAP = {0}; Ifx_PPU MODULE_PPU_VMEMAP = {0};
Ifx_PSI5S MODULE_PSI5S0 = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI MODULE_QSPI0 = {0}; Ifx_QSPI MODULE_QSPI1 = {0}; Ifx_QSPI MODULE_QSPI2 = {0}; Ifx_QSPI MODULE_QSPI3 = {0}; Ifx_QSPI MODULE_QSPI4 = {0}; Ifx_QSPI MODULE_QSPI5 = {0}; Ifx_QSPI MODULE_QSPI6 = {0}; Ifx_QSPI MODULE_QSPI7 = {0};
Ifx_SCU MODULE_SCU = {0};
Ifx_SDMMC MODULE_SDMMC0 = {0};
Ifx_SENT MODULE_SENT0 = {0}; Ifx_SENT MODULE_SENT1 = {0};
Ifx_SMM MODULE_SMM = {0};
Ifx_SMU MODULE_SMU = {0}; Ifx_SMU MODULE_SMUSTDBY = {0};
Ifx_SRC MODULE_SRC = {0};
Ifx_TRIF MODULE_TRIF = {0};
Ifx_TRI MODULE_TRI = {0};
Ifx_VMT MODULE_VMT0 = {0}; Ifx_VMT MODULE_VMT1 = {0}; Ifx_VMT MODULE_VMT2 = {0}; Ifx_VMT MODULE_VMT3 = {0}; Ifx_VMT MODULE_VMT4 = {0}; Ifx_VMT MODULE_VMT5 = {0}; Ifx_VMT MODULE_VMT6 = {0};
Ifx_VTMON MODULE_VTMON = {0};
Ifx_WTU MODULE_WTU = {0};
Ifx_XSPI MODULE_XSPI0 = {0};
Ifx_PROT MODULE_PROT = {0};
Ifx_ACCEN MODULE_ACCEN = {0};

/* Pin symbols allocation */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT  = {0};

/* Spy state */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;
int mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
int mock_IfxEgtm_Atom_Timer_run_callCount = 0;

int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;

int mock_IfxEgtm_Cmu_enable_callCount = 0;
int mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int mock_IfxEgtm_Cmu_setClkFrequency_f32_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_f32_callCount = 0;

int mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

int mock_IfxAdc_Tmadc_initModule_callCount = 0;
int mock_IfxAdc_Tmadc_initModuleConfig_callCount = 0;

int mock_IfxAdc_enableModule_callCount = 0;

int mock_IfxEgtm_enable_callCount = 0;
int mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int mock_IfxEgtm_PinMap_setAtomTout_callCount = 0;

int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_initConfig_callCount = 0;

uint32 mock_togglePin_callCount = 0;

/* Internal capture of numChannels for bounded copies */
static uint32 _captured_numChannels = 0;

/* Stubs */
boolean IfxEgtm_Atom_Timer_setFrequency(IfxEgtm_Atom_Timer *driver, float32 frequency)
{
    (void)driver; (void)frequency;
    mock_IfxEgtm_Atom_Timer_setFrequency_callCount++;
    return mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
}
void IfxEgtm_Atom_Timer_setTrigger(IfxEgtm_Atom_Timer *driver, uint32 triggerPoint)
{
    (void)driver; (void)triggerPoint;
    mock_IfxEgtm_Atom_Timer_setTrigger_callCount++;
}
void IfxEgtm_Atom_Timer_run(IfxEgtm_Atom_Timer *driver)
{
    (void)driver;
    mock_IfxEgtm_Atom_Timer_run_callCount++;
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* sensible default */
}
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count)
{
    (void)egtm; (void)clkIndex; (void)count;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}
/* Mandatory alt CMU API variants */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_enable_callCount++; }
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++; return mock_IfxEgtm_Cmu_isEnabled_returnValue; }
void IfxEgtm_Cmu_setClkFrequency_f32(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency) { (void)module; (void)clk; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_f32_callCount++; }
void IfxEgtm_Cmu_setGclkFrequency_f32(Ifx_EGTM *module, float32 frequency) { (void)module; (void)frequency; mock_IfxEgtm_Cmu_setGclkFrequency_f32_callCount++; }

boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}

void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config)
{
    (void)tmadc; (void)config;
    mock_IfxAdc_Tmadc_initModule_callCount++;
}
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc)
{
    (void)config; (void)adc;
    mock_IfxAdc_Tmadc_initModuleConfig_callCount++;
}

void IfxAdc_enableModule(Ifx_ADC *modSFR)
{
    (void)modSFR;
    mock_IfxAdc_enableModule_callCount++;
}

void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_enable_callCount++;
}
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm; mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}

void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config; (void)outputMode; (void)padDriver;
    mock_IfxEgtm_PinMap_setAtomTout_callCount++;
}

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

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_togglePin_callCount++;
}

/* Getters */
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void){ return mock_IfxEgtm_Atom_Timer_setFrequency_callCount; }
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void){ return mock_IfxEgtm_Atom_Timer_setTrigger_callCount; }
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void){ return mock_IfxEgtm_Atom_Timer_run_callCount; }

int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void){ return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void){ return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void){ return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_f32_getCallCount(void){ return mock_IfxEgtm_Cmu_setClkFrequency_f32_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_f32_getCallCount(void){ return mock_IfxEgtm_Cmu_setGclkFrequency_f32_callCount; }

int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void){ return mock_IfxEgtm_Trigger_trigToAdc_callCount; }

int mock_IfxAdc_Tmadc_initModule_getCallCount(void){ return mock_IfxAdc_Tmadc_initModule_callCount; }
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void){ return mock_IfxAdc_Tmadc_initModuleConfig_callCount; }

int mock_IfxAdc_enableModule_getCallCount(void){ return mock_IfxAdc_enableModule_callCount; }

int mock_IfxEgtm_enable_getCallCount(void){ return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void){ return mock_IfxEgtm_isEnabled_callCount; }

int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void){ return mock_IfxEgtm_PinMap_setAtomTout_callCount; }

int mock_IfxEgtm_Pwm_init_getCallCount(void){ return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void){ return mock_IfxEgtm_Pwm_initConfig_callCount; }

uint32 mock_togglePin_getCallCount(void){ return mock_togglePin_callCount; }

/* Reset */
void mock_egtm_atom_tmadc_consolidated_reset(void)
{
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
    for (uint32 i=0U;i<MOCK_MAX_CHANNELS;i++){
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[i] = 0.0f;
    }

    mock_IfxEgtm_Atom_Timer_setFrequency_callCount = 0;
    mock_IfxEgtm_Atom_Timer_setFrequency_returnValue = FALSE;
    mock_IfxEgtm_Atom_Timer_setTrigger_callCount = 0;
    mock_IfxEgtm_Atom_Timer_run_callCount = 0;

    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;

    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_setClkFrequency_f32_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_f32_callCount = 0;

    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;

    mock_IfxAdc_Tmadc_initModule_callCount = 0;
    mock_IfxAdc_Tmadc_initModuleConfig_callCount = 0;

    mock_IfxAdc_enableModule_callCount = 0;

    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_PinMap_setAtomTout_callCount = 0;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;

    mock_togglePin_callCount = 0U;

    _captured_numChannels = 0U;
}
