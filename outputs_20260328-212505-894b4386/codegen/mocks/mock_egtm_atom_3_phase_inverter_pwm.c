#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include <string.h>

/* Spy counters and return values */
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

int mock_IfxEgtm_isEnabled_callCount = 0;
int mock_IfxEgtm_enable_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_togglePin_callCount = 0;
uint32  mock_togglePin_callCount = 0; /* legacy alias */

/* captured number of channels for bounded copies */
static uint32 _captured_numChannels = 0;

/* MODULE_* instance definitions */
Ifx_EGTM MODULE_EGTM; Ifx_GTM MODULE_GTM; Ifx_P MODULE_P00; Ifx_P MODULE_P01; Ifx_P MODULE_P02; Ifx_P MODULE_P03; Ifx_P MODULE_P04; Ifx_P MODULE_P10; Ifx_P MODULE_P13; Ifx_P MODULE_P14; Ifx_P MODULE_P15; Ifx_P MODULE_P16; Ifx_P MODULE_P20; Ifx_P MODULE_P21; Ifx_P MODULE_P22; Ifx_P MODULE_P23; Ifx_P MODULE_P25; Ifx_P MODULE_P30; Ifx_P MODULE_P31; Ifx_P MODULE_P32; Ifx_P MODULE_P33; Ifx_P MODULE_P34; Ifx_P MODULE_P35; Ifx_P MODULE_P40; Ifx_P MODULE_P41;
Ifx_ADC MODULE_ADC;
Ifx_ASCLIN0 MODULE_ASCLIN0; Ifx_ASCLIN0 MODULE_ASCLIN1; Ifx_ASCLIN0 MODULE_ASCLIN2; Ifx_ASCLIN0 MODULE_ASCLIN3; Ifx_ASCLIN0 MODULE_ASCLIN4; Ifx_ASCLIN0 MODULE_ASCLIN5; Ifx_ASCLIN0 MODULE_ASCLIN6; Ifx_ASCLIN0 MODULE_ASCLIN7; Ifx_ASCLIN0 MODULE_ASCLIN8; Ifx_ASCLIN0 MODULE_ASCLIN9; Ifx_ASCLIN0 MODULE_ASCLIN10; Ifx_ASCLIN0 MODULE_ASCLIN11; Ifx_ASCLIN0 MODULE_ASCLIN12; Ifx_ASCLIN0 MODULE_ASCLIN13; Ifx_ASCLIN0 MODULE_ASCLIN14; Ifx_ASCLIN0 MODULE_ASCLIN15; Ifx_ASCLIN0 MODULE_ASCLIN16; Ifx_ASCLIN0 MODULE_ASCLIN17; Ifx_ASCLIN0 MODULE_ASCLIN18; Ifx_ASCLIN0 MODULE_ASCLIN19; Ifx_ASCLIN0 MODULE_ASCLIN20; Ifx_ASCLIN0 MODULE_ASCLIN21; Ifx_ASCLIN0 MODULE_ASCLIN22; Ifx_ASCLIN0 MODULE_ASCLIN23; Ifx_ASCLIN0 MODULE_ASCLIN24; Ifx_ASCLIN0 MODULE_ASCLIN25; Ifx_ASCLIN0 MODULE_ASCLIN26; Ifx_ASCLIN0 MODULE_ASCLIN27;
Ifx_TBCU MODULE_TBCU; Ifx_CSBCU MODULE_CSBCU; Ifx_SBCU MODULE_SBCU; Ifx_COMBCU MODULE_COMBCU;
Ifx_CANXL0 MODULE_CANXL0; Ifx_CANXL0_RAM MODULE_CANXL0_RAM;
Ifx_CAN0 MODULE_CAN0; Ifx_CAN0_RAM MODULE_CAN0_RAM; Ifx_CAN1 MODULE_CAN1; Ifx_CAN1_RAM MODULE_CAN1_RAM; Ifx_CAN2 MODULE_CAN2; Ifx_CAN2_RAM MODULE_CAN2_RAM; Ifx_CAN3 MODULE_CAN3; Ifx_CAN3_RAM MODULE_CAN3_RAM; Ifx_CAN4 MODULE_CAN4; Ifx_CAN4_RAM MODULE_CAN4_RAM;
Ifx_CBS MODULE_CBS; Ifx_CLOCK MODULE_CLOCK;
Ifx_CPU_CFI0 MODULE_CPU_CFI0; Ifx_CPU_CFI1 MODULE_CPU_CFI1; Ifx_CPU_CFI2 MODULE_CPU_CFI2; Ifx_CPU_CFI3 MODULE_CPU_CFI3; Ifx_CPU_CFI4 MODULE_CPU_CFI4; Ifx_CPU_CFI5 MODULE_CPU_CFI5; Ifx_CPU_CFICS MODULE_CPU_CFICS;
Ifx_CPU0 MODULE_CPU0; Ifx_CPU1 MODULE_CPU1; Ifx_CPU2 MODULE_CPU2; Ifx_CPU3 MODULE_CPU3; Ifx_CPU4 MODULE_CPU4; Ifx_CPU5 MODULE_CPU5; Ifx_CPUCS MODULE_CPUCS;
Ifx_DMA0 MODULE_DMA0; Ifx_DMA1 MODULE_DMA1; Ifx_DMU MODULE_DMU;
Ifx_DOM0 MODULE_DOM0; Ifx_DOM1 MODULE_DOM1; Ifx_DOM3 MODULE_DOM3; Ifx_DOM6 MODULE_DOM6; Ifx_DOM7 MODULE_DOM7; Ifx_DOM4 MODULE_DOM4; Ifx_DOM2 MODULE_DOM2; Ifx_DOM5 MODULE_DOM5;
Ifx_DRE MODULE_DRE;
Ifx_ERAY0 MODULE_ERAY0; Ifx_ERAY1 MODULE_ERAY1;
Ifx_FCE MODULE_FCE;
Ifx_FSI_CSRM MODULE_FSI_CSRM; Ifx_FSI_HOST MODULE_FSI_HOST;
Ifx_GETH0 MODULE_GETH0;
Ifx_HSCT0 MODULE_HSCT0; Ifx_HSCT1 MODULE_HSCT1;
Ifx_HSPHY MODULE_HSPHY; Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA;
Ifx_HSSL0 MODULE_HSSL0; Ifx_HSSL1 MODULE_HSSL1;
Ifx_I2C0 MODULE_I2C0; Ifx_I2C1 MODULE_I2C1; Ifx_I2C2 MODULE_I2C2;
Ifx_INT MODULE_INT;
Ifx_LETH0 MODULE_LETH0;
Ifx_LLI0 MODULE_LLI0;
Ifx_LMU0 MODULE_LMU0; Ifx_LMU1 MODULE_LMU1; Ifx_LMU2 MODULE_LMU2; Ifx_LMU3 MODULE_LMU3; Ifx_LMU4 MODULE_LMU4; Ifx_LMU5 MODULE_LMU5; Ifx_LMU6 MODULE_LMU6; Ifx_LMU7 MODULE_LMU7; Ifx_LMU8 MODULE_LMU8; Ifx_LMU9 MODULE_LMU9;
Ifx_MCDS2P MODULE_MCDS2P; Ifx_MCDS4P MODULE_MCDS4P;
Ifx_MSC0 MODULE_MSC0;
Ifx_PCIE0_DSP MODULE_PCIE0_DSP; Ifx_PCIE0_DSP_SRI MODULE_PCIE0_DSP_SRI; Ifx_PCIE1_DSP MODULE_PCIE1_DSP; Ifx_PCIE1_DSP_SRI MODULE_PCIE1_DSP_SRI;
Ifx_PCIE0_USP MODULE_PCIE0_USP; Ifx_PCIE0_USP_SRI MODULE_PCIE0_USP_SRI; Ifx_PCIE1_USP MODULE_PCIE1_USP; Ifx_PCIE1_USP_SRI MODULE_PCIE1_USP_SRI;
Ifx_PFRWB0A MODULE_PFRWB0A; Ifx_PFRWB0B MODULE_PFRWB0B; Ifx_PFRWB1A MODULE_PFRWB1A; Ifx_PFRWB1B MODULE_PFRWB1B; Ifx_PFRWB2A MODULE_PFRWB2A; Ifx_PFRWB2B MODULE_PFRWB2B; Ifx_PFRWB3A MODULE_PFRWB3A; Ifx_PFRWB3B MODULE_PFRWB3B; Ifx_PFRWB4A MODULE_PFRWB4A; Ifx_PFRWB4B MODULE_PFRWB4B; Ifx_PFRWB5A MODULE_PFRWB5A; Ifx_PFRWB5B MODULE_PFRWB5B; Ifx_PFRWBCS MODULE_PFRWBCS;
Ifx_PMS MODULE_PMS;
Ifx_PPU MODULE_PPU; Ifx_PPU_STUDMI MODULE_PPU_STUDMI; Ifx_PPU_DEBUG MODULE_PPU_DEBUG; Ifx_PPU_SM MODULE_PPU_SM; Ifx_PPU_APU MODULE_PPU_APU; Ifx_PPU_CSMAP MODULE_PPU_CSMAP; Ifx_PPU_VMEMAP MODULE_PPU_VMEMAP;
Ifx_PSI5S0 MODULE_PSI5S0; Ifx_PSI5 MODULE_PSI5;
Ifx_QSPI0 MODULE_QSPI0; Ifx_QSPI1 MODULE_QSPI1; Ifx_QSPI2 MODULE_QSPI2; Ifx_QSPI3 MODULE_QSPI3; Ifx_QSPI4 MODULE_QSPI4; Ifx_QSPI5 MODULE_QSPI5; Ifx_QSPI6 MODULE_QSPI6; Ifx_QSPI7 MODULE_QSPI7;
Ifx_SCU MODULE_SCU; Ifx_SDMMC0 MODULE_SDMMC0; Ifx_SENT0 MODULE_SENT0; Ifx_SENT1 MODULE_SENT1; Ifx_SMM MODULE_SMM; Ifx_SMU MODULE_SMU; Ifx_SMUSTDBY MODULE_SMUSTDBY; Ifx_SRC MODULE_SRC; Ifx_TRIF MODULE_TRIF; Ifx_TRI MODULE_TRI; Ifx_VMT0 MODULE_VMT0; Ifx_VMT1 MODULE_VMT1; Ifx_VMT2 MODULE_VMT2; Ifx_VMT3 MODULE_VMT3; Ifx_VMT4 MODULE_VMT4; Ifx_VMT5 MODULE_VMT5; Ifx_VMT6 MODULE_VMT6; Ifx_VTMON MODULE_VTMON; Ifx_WTU MODULE_WTU; Ifx_XSPI0 MODULE_XSPI0;

/* Stubs */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount++;
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

void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, uint32 numerator, uint32 denominator)
{
    (void)egtm; (void)clkIndex; (void)numerator; (void)denominator;
    mock_IfxEgtm_Cmu_setEclkDivider_callCount++;
}

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

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS)
             ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

/* Getters */
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void)      { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void)    { return mock_IfxEgtm_Cmu_setGclkDivider_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void){ return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(void)    { return mock_IfxEgtm_Cmu_setEclkDivider_callCount; }

int mock_IfxEgtm_isEnabled_getCallCount(void)             { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void)                { return mock_IfxEgtm_enable_callCount; }

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void)        { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void)              { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }

int mock_IfxPort_setPinModeOutput_getCallCount(void)      { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_togglePin_getCallCount(void)             { return mock_IfxPort_togglePin_callCount; }

/* Reset */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setEclkDivider_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0u;
}
