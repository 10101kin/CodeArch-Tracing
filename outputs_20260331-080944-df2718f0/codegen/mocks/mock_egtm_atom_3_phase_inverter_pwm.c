/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* =========================
 * Spy state definitions
 * ========================= */

int     mock_IfxEgtm_enable_callCount = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxEgtm_Pwm_setDeadtime_callCount = 0;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_setDeadtime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_setDeadtime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

uint32  mock_togglePin_callCount = 0u;

/* Bounded copy helper: captured numChannels from latest init */
static uint32 _captured_numChannels = 0u;

/* =========================
 * MODULE_* instance definitions
 * ========================= */

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

Ifx_ADC MODULE_ADC = {0};
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
Ifx_ASCLIN MODULE_ASCLIN24 = {0};
Ifx_ASCLIN MODULE_ASCLIN25 = {0};
Ifx_ASCLIN MODULE_ASCLIN26 = {0};
Ifx_ASCLIN MODULE_ASCLIN27 = {0};
Ifx_TBCU MODULE_TBCU = {0};
Ifx_CSBCU MODULE_CSBCU = {0};
Ifx_SBCU MODULE_SBCU = {0};
Ifx_COMBCU MODULE_COMBCU = {0};
Ifx_CANXL MODULE_CANXL0 = {0};
Ifx_CANXL_RAM MODULE_CANXL0_RAM = {0};
Ifx_CAN MODULE_CAN0 = {0};
Ifx_CAN_RAM MODULE_CAN0_RAM = {0};
Ifx_CAN MODULE_CAN1 = {0};
Ifx_CAN_RAM MODULE_CAN1_RAM = {0};
Ifx_CAN MODULE_CAN2 = {0};
Ifx_CAN_RAM MODULE_CAN2_RAM = {0};
Ifx_CAN MODULE_CAN3 = {0};
Ifx_CAN_RAM MODULE_CAN3_RAM = {0};
Ifx_CAN MODULE_CAN4 = {0};
Ifx_CAN_RAM MODULE_CAN4_RAM = {0};
Ifx_CBS MODULE_CBS = {0};
Ifx_CLOCK MODULE_CLOCK = {0};
Ifx_CPU_CFI MODULE_CPU_CFI0 = {0};
Ifx_CPU_CFI MODULE_CPU_CFI1 = {0};
Ifx_CPU_CFI MODULE_CPU_CFI2 = {0};
Ifx_CPU_CFI MODULE_CPU_CFI3 = {0};
Ifx_CPU_CFI MODULE_CPU_CFI4 = {0};
Ifx_CPU_CFI MODULE_CPU_CFI5 = {0};
Ifx_CPU_CFICS MODULE_CPU_CFICS = {0};
Ifx_CPU MODULE_CPU0 = {0};
Ifx_CPU MODULE_CPU1 = {0};
Ifx_CPU MODULE_CPU2 = {0};
Ifx_CPU MODULE_CPU3 = {0};
Ifx_CPU MODULE_CPU4 = {0};
Ifx_CPU MODULE_CPU5 = {0};
Ifx_CPUCS MODULE_CPUCS = {0};
Ifx_DMA MODULE_DMA0 = {0};
Ifx_DMA MODULE_DMA1 = {0};
Ifx_DMU MODULE_DMU = {0};
Ifx_DOM MODULE_DOM0 = {0};
Ifx_DOM MODULE_DOM1 = {0};
Ifx_DOM MODULE_DOM3 = {0};
Ifx_DOM MODULE_DOM6 = {0};
Ifx_DOM MODULE_DOM7 = {0};
Ifx_DOM MODULE_DOM4 = {0};
Ifx_DOM MODULE_DOM2 = {0};
Ifx_DOM MODULE_DOM5 = {0};
Ifx_DRE MODULE_DRE = {0};
Ifx_ERAY MODULE_ERAY0 = {0};
Ifx_ERAY MODULE_ERAY1 = {0};
Ifx_FCE MODULE_FCE = {0};
Ifx_FSI_CSRM MODULE_FSI_CSRM = {0};
Ifx_FSI_HOST MODULE_FSI_HOST = {0};
Ifx_GETH MODULE_GETH0 = {0};
Ifx_HSCT MODULE_HSCT0 = {0};
Ifx_HSCT MODULE_HSCT1 = {0};
Ifx_HSPHY MODULE_HSPHY = {0};
Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA = {0};
Ifx_HSSL MODULE_HSSL0 = {0};
Ifx_HSSL MODULE_HSSL1 = {0};
Ifx_I2C MODULE_I2C0 = {0};
Ifx_I2C MODULE_I2C1 = {0};
Ifx_I2C MODULE_I2C2 = {0};
Ifx_INT MODULE_INT = {0};
Ifx_LETH MODULE_LETH0 = {0};
Ifx_LLI MODULE_LLI0 = {0};
Ifx_LMU MODULE_LMU0 = {0};
Ifx_LMU MODULE_LMU1 = {0};
Ifx_LMU MODULE_LMU2 = {0};
Ifx_LMU MODULE_LMU3 = {0};
Ifx_LMU MODULE_LMU4 = {0};
Ifx_LMU MODULE_LMU5 = {0};
Ifx_LMU MODULE_LMU6 = {0};
Ifx_LMU MODULE_LMU7 = {0};
Ifx_LMU MODULE_LMU8 = {0};
Ifx_LMU MODULE_LMU9 = {0};
Ifx_MCDS2P MODULE_MCDS2P = {0};
Ifx_MCDS4P MODULE_MCDS4P = {0};
Ifx_MSC MODULE_MSC0 = {0};
Ifx_PCIE_DSP MODULE_PCIE0_DSP = {0};
Ifx_PCIE_DSP_SRI MODULE_PCIE0_DSP_SRI = {0};
Ifx_PCIE_DSP MODULE_PCIE1_DSP = {0};
Ifx_PCIE_DSP_SRI MODULE_PCIE1_DSP_SRI = {0};
Ifx_PCIE_USP MODULE_PCIE0_USP = {0};
Ifx_PCIE_USP_SRI MODULE_PCIE0_USP_SRI = {0};
Ifx_PCIE_USP MODULE_PCIE1_USP = {0};
Ifx_PCIE_USP_SRI MODULE_PCIE1_USP_SRI = {0};
Ifx_PFRWB MODULE_PFRWB0A = {0};
Ifx_PFRWB MODULE_PFRWB0B = {0};
Ifx_PFRWB MODULE_PFRWB1A = {0};
Ifx_PFRWB MODULE_PFRWB1B = {0};
Ifx_PFRWB MODULE_PFRWB2A = {0};
Ifx_PFRWB MODULE_PFRWB2B = {0};
Ifx_PFRWB MODULE_PFRWB3A = {0};
Ifx_PFRWB MODULE_PFRWB3B = {0};
Ifx_PFRWB MODULE_PFRWB4A = {0};
Ifx_PFRWB MODULE_PFRWB4B = {0};
Ifx_PFRWB MODULE_PFRWB5A = {0};
Ifx_PFRWB MODULE_PFRWB5B = {0};
Ifx_PFRWB MODULE_PFRWBCS = {0};
Ifx_PMS MODULE_PMS = {0};
Ifx_PSI5S MODULE_PSI5S0 = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI MODULE_QSPI0 = {0};
Ifx_QSPI MODULE_QSPI1 = {0};
Ifx_QSPI MODULE_QSPI2 = {0};
Ifx_QSPI MODULE_QSPI3 = {0};
Ifx_QSPI MODULE_QSPI4 = {0};
Ifx_QSPI MODULE_QSPI5 = {0};
Ifx_QSPI MODULE_QSPI6 = {0};
Ifx_QSPI MODULE_QSPI7 = {0};
Ifx_SCU MODULE_SCU = {0};
Ifx_SDMMC MODULE_SDMMC0 = {0};
Ifx_SENT MODULE_SENT0 = {0};
Ifx_SENT MODULE_SENT1 = {0};
Ifx_SMM MODULE_SMM = {0};
Ifx_SMU MODULE_SMU = {0};
Ifx_SMU_STDBY MODULE_SMUSTDBY = {0};
Ifx_SRC MODULE_SRC = {0};
Ifx_TRIF MODULE_TRIF = {0};
Ifx_TRI MODULE_TRI = {0};
Ifx_VMT MODULE_VMT0 = {0};
Ifx_VMT MODULE_VMT1 = {0};
Ifx_VMT MODULE_VMT2 = {0};
Ifx_VMT MODULE_VMT3 = {0};
Ifx_VMT MODULE_VMT4 = {0};
Ifx_VMT MODULE_VMT5 = {0};
Ifx_VMT MODULE_VMT6 = {0};
Ifx_VTMON MODULE_VTMON = {0};
Ifx_WTU MODULE_WTU = {0};
Ifx_XSPI MODULE_XSPI0 = {0};

/* =========================
 * Pin symbol instances (ToutMap)
 * ========================= */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT = {0};

/* =========================
 * Stub bodies
 * ========================= */

/* IfxEgtm */
void IfxEgtm_enable(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_enable_callCount++; }
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_isEnabled_callCount++; return mock_IfxEgtm_isEnabled_returnValue; }
void IfxEgtm_disable(Ifx_EGTM *egtm) { (void)egtm; }
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode) { (void)egtm; (void)mode; }
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm) { (void)egtm; return 0.0f; }
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; return 0.0f; }
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, float32 ticks) { (void)egtm; return ticks; }
float32 IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds) { (void)egtm; return seconds; }
void IfxEgtm_initProtSe(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_initCtrlProt(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_initClApu(Ifx_EGTM *egtm, const IfxEgtm_ClApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, const IfxEgtm_CtrlApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initWrapApu(Ifx_EGTM *egtm, const IfxEgtm_WrapApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster, IfxEgtm_ClusterClockDiv div) { (void)egtm; (void)cluster; (void)div; }
void IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; }
void IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
void IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 channel, boolean enable) { (void)egtm; (void)channel; (void)enable; }
void IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode) { (void)egtm; (void)mode; }
void IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, const IfxEgtm_MscOut *msc) { (void)egtm; (void)msc; }
void IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg) { (void)cfg; }
void IfxEgtm_initAp(Ifx_EGTM *egtm, const IfxEgtm_ApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_configureAccessToEgtms(void) { }
float32 IfxClock_geteGtmFrequency(void) { return 0.0f; }

/* IfxEgtm_Cmu */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *module, uint32 mask) { (void)module; (void)mask; mock_IfxEgtm_Cmu_enableClocks_callCount++; }
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *module) { (void)module; return 0.0f; }
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_getModuleFrequency_callCount++; return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }
boolean IfxEgtm_Cmu_isClkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk) { (void)module; (void)clk; return FALSE; }
boolean IfxEgtm_Cmu_isEclkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk) { (void)module; (void)eclk; return FALSE; }
boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx) { (void)module; (void)fx; return FALSE; }
void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk) { (void)module; (void)clk; }
void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, uint32 count) { (void)module; (void)clk; (void)count; }
void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, uint32 div) { (void)module; (void)eclk; (void)div; }
void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *module, uint32 numerator, uint32 denominator) { (void)module; (void)numerator; (void)denominator; }
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk) { (void)module; (void)clk; return 0.0f; }
float32 IfxEgtm_Cmu_getEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk) { (void)module; (void)eclk; return 0.0f; }
float32 IfxEgtm_Cmu_getFxClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx) { (void)module; (void)fx; return 0.0f; }
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency) { (void)module; (void)clk; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_callCount++; }
void IfxEgtm_Cmu_setEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, float32 frequency) { (void)module; (void)eclk; (void)frequency; }
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *module, uint32 numerator, uint32 denominator) { (void)module; (void)numerator; (void)denominator; mock_IfxEgtm_Cmu_setGclkFrequency_callCount++; }

/* Additional mandatory CMU mocks */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_enable_callCount++; }
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++; return mock_IfxEgtm_Cmu_isEnabled_returnValue; }

/* IfxEgtm_Pwm */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *driver, const IfxEgtm_Pwm_Config *config)
{
    (void)driver;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels                 = config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *driver, const float32 *duties, uint32 count)
{
    (void)driver;
    (void)count;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= MOCK_MAX_CHANNELS)
                ? _captured_numChannels
                : MOCK_MAX_CHANNELS;
    if (duties != NULL_PTR)
    {
        for (uint32 i = 0u; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = duties[i];
        }
    }
}

void IfxEgtm_Pwm_setDeadtime(IfxEgtm_Pwm *driver, const float32 *dtRising, const float32 *dtFalling, uint32 count)
{
    (void)driver;
    (void)count;
    mock_IfxEgtm_Pwm_setDeadtime_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= MOCK_MAX_CHANNELS)
                ? _captured_numChannels
                : MOCK_MAX_CHANNELS;
    if (dtRising != NULL_PTR)
    {
        for (uint32 i = 0u; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_setDeadtime_lastDtRising[i] = dtRising[i];
        }
    }
    if (dtFalling != NULL_PTR)
    {
        for (uint32 i = 0u; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_setDeadtime_lastDtFalling[i] = dtFalling[i];
        }
    }
}

/* IfxPort basic stubs */
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode) { (void)port; (void)pinIndex; (void)mode; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver driver) { (void)port; (void)pinIndex; (void)driver; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_togglePin_callCount++; }

/* =========================
 * Mock control API
 * ========================= */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_setDeadtime_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0u;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0u; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_setDeadtime_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_setDeadtime_lastDtFalling[i] = 0.0f;
    }

    mock_togglePin_callCount = 0u;
    _captured_numChannels = 0u;
}

/* =========================
 * Call count getters
 * ========================= */
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_setDeadtime_getCallCount(void) { return mock_IfxEgtm_Pwm_setDeadtime_callCount; }
