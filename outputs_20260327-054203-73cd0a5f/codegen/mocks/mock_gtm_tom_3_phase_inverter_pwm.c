/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxPort.h"

/* Spy storage */
uint32  mock_init_lastNumChannels = 0u;
float32 mock_init_lastFrequency   = 0.0f;
float32 mock_update_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS]   = {0};
float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS]  = {0};
uint32  mock_togglePin_callCount = 0u;

/* Counters */
int mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setMode_callCount    = 0;
int mock_IfxGtm_Tom_PwmHl_init_callCount       = 0;
int mock_IfxGtm_Tom_PwmHl_setOnTime_callCount  = 0;
int mock_IfxGtm_Tom_Timer_init_callCount       = 0;
int mock_IfxGtm_Tom_Timer_run_callCount        = 0;
int mock_IfxGtm_Tom_Timer_applyUpdate_callCount= 0;
int mock_IfxGtm_Tom_Timer_disableUpdate_callCount=0;
int mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
int mock_IfxGtm_Cmu_enableClocks_callCount     = 0;
int mock_IfxGtm_enable_callCount               = 0;
int mock_IfxPort_setPinHigh_callCount          = 0;
int mock_IfxPort_setPinLow_callCount           = 0;

/* Return controls */
boolean mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_init_returnValue    = FALSE;
boolean mock_IfxGtm_Tom_Timer_init_returnValue    = FALSE;
boolean mock_IfxGtm_isEnabled_returnValue         = FALSE;
boolean mock_IfxGtm_isModuleSuspended_returnValue = FALSE;
float32 mock_IfxGtm_getSysClkFrequency_returnValue= 0.0f;
float32 mock_IfxGtm_getClusterFrequency_returnValue=0.0f;
float32 mock_IfxGtm_Cmu_getClkFrequency_returnValue=0.0f;
float32 mock_IfxGtm_Cmu_getEclkFrequency_returnValue=0.0f;
float32 mock_IfxGtm_Cmu_getFxClkFrequency_returnValue=0.0f;
float32 mock_IfxGtm_Cmu_getGclkFrequency_returnValue=0.0f;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue=0.0f;
boolean mock_IfxGtm_Cmu_isClkClockEnabled_returnValue=FALSE;
boolean mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue=FALSE;
boolean mock_IfxGtm_Cmu_isFxClockEnabled_returnValue=FALSE;

/* MODULE_* instances */
Ifx_GTM MODULE_GTM;
Ifx_ASCLIN0 MODULE_ASCLIN0; Ifx_ASCLIN1 MODULE_ASCLIN1; Ifx_ASCLIN2 MODULE_ASCLIN2; Ifx_ASCLIN3 MODULE_ASCLIN3;
Ifx_ASCLIN4 MODULE_ASCLIN4; Ifx_ASCLIN5 MODULE_ASCLIN5; Ifx_ASCLIN6 MODULE_ASCLIN6; Ifx_ASCLIN7 MODULE_ASCLIN7;
Ifx_ASCLIN8 MODULE_ASCLIN8; Ifx_ASCLIN9 MODULE_ASCLIN9; Ifx_ASCLIN10 MODULE_ASCLIN10; Ifx_ASCLIN11 MODULE_ASCLIN11;
Ifx_ASCLIN12 MODULE_ASCLIN12; Ifx_ASCLIN13 MODULE_ASCLIN13; Ifx_ASCLIN14 MODULE_ASCLIN14; Ifx_ASCLIN15 MODULE_ASCLIN15;
Ifx_ASCLIN16 MODULE_ASCLIN16; Ifx_ASCLIN17 MODULE_ASCLIN17; Ifx_ASCLIN18 MODULE_ASCLIN18; Ifx_ASCLIN19 MODULE_ASCLIN19;
Ifx_ASCLIN20 MODULE_ASCLIN20; Ifx_ASCLIN21 MODULE_ASCLIN21; Ifx_ASCLIN22 MODULE_ASCLIN22; Ifx_ASCLIN23 MODULE_ASCLIN23;
Ifx_CAN0 MODULE_CAN0; Ifx_CAN1 MODULE_CAN1; Ifx_CAN2 MODULE_CAN2; Ifx_CBS MODULE_CBS; Ifx_CCU60 MODULE_CCU60; Ifx_CCU61 MODULE_CCU61;
Ifx_CONVCTRL MODULE_CONVCTRL; Ifx_CPU0 MODULE_CPU0; Ifx_CPU1 MODULE_CPU1; Ifx_CPU2 MODULE_CPU2; Ifx_CPU3 MODULE_CPU3;
Ifx_DAM0 MODULE_DAM0; Ifx_DMA MODULE_DMA; Ifx_DMU MODULE_DMU; Ifx_DOM0 MODULE_DOM0; Ifx_EDSADC MODULE_EDSADC;
Ifx_ERAY0 MODULE_ERAY0; Ifx_ERAY1 MODULE_ERAY1; Ifx_EVADC MODULE_EVADC; Ifx_FCE MODULE_FCE; Ifx_FSI MODULE_FSI;
Ifx_GETH MODULE_GETH; Ifx_GPT120 MODULE_GPT120; Ifx_HSCT0 MODULE_HSCT0; Ifx_HSSL0 MODULE_HSSL0; Ifx_I2C0 MODULE_I2C0; Ifx_I2C1 MODULE_I2C1;
Ifx_INT MODULE_INT; Ifx_IOM MODULE_IOM; Ifx_LMU0 MODULE_LMU0; Ifx_MINIMCDS MODULE_MINIMCDS; Ifx_MSC0 MODULE_MSC0; Ifx_MSC1 MODULE_MSC1; Ifx_MSC2 MODULE_MSC2;
Ifx_MTU MODULE_MTU; Ifx_PFI0 MODULE_PFI0; Ifx_PFI1 MODULE_PFI1; Ifx_PFI2 MODULE_PFI2; Ifx_PFI3 MODULE_PFI3; Ifx_PMS MODULE_PMS; Ifx_PMU MODULE_PMU;
Ifx_P MODULE_P00; Ifx_P MODULE_P01; Ifx_P MODULE_P02; Ifx_P MODULE_P10; Ifx_P MODULE_P11; Ifx_P MODULE_P12; Ifx_P MODULE_P13; Ifx_P MODULE_P14; Ifx_P MODULE_P15;
Ifx_P MODULE_P20; Ifx_P MODULE_P21; Ifx_P MODULE_P22; Ifx_P MODULE_P23; Ifx_P MODULE_P24; Ifx_P MODULE_P25; Ifx_P MODULE_P26;
Ifx_P MODULE_P30; Ifx_P MODULE_P31; Ifx_P MODULE_P32; Ifx_P MODULE_P33; Ifx_P MODULE_P34; Ifx_P MODULE_P40; Ifx_P MODULE_P41;
Ifx_PSI5S MODULE_PSI5S; Ifx_PSI5 MODULE_PSI5; Ifx_QSPI0 MODULE_QSPI0; Ifx_QSPI1 MODULE_QSPI1; Ifx_QSPI2 MODULE_QSPI2; Ifx_QSPI3 MODULE_QSPI3; Ifx_QSPI4 MODULE_QSPI4;
Ifx_SBCU MODULE_SBCU; Ifx_SCU MODULE_SCU; Ifx_SENT MODULE_SENT; Ifx_SMU MODULE_SMU; Ifx_SRC MODULE_SRC;
Ifx_STM0 MODULE_STM0; Ifx_STM1 MODULE_STM1; Ifx_STM2 MODULE_STM2; Ifx_STM3 MODULE_STM3;

/* ===== Stubs: IfxGtm.h ===== */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm) { (void)gtm; return mock_IfxGtm_isEnabled_returnValue; }
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm) { (void)gtm; return mock_IfxGtm_isModuleSuspended_returnValue; }
void IfxGtm_setSuspendMode(Ifx_GTM *gtm, sint32 mode) { (void)gtm; (void)mode; }
void IfxGtm_disable(Ifx_GTM *gtm) { (void)gtm; }
void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm) { (void)gtm; return mock_IfxGtm_getSysClkFrequency_returnValue; }
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, sint32 cluster) { (void)gtm; (void)cluster; return mock_IfxGtm_getClusterFrequency_returnValue; }

/* ===== Stubs: IfxGtm_Cmu.h ===== */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) { (void)gtm; (void)clkMask; mock_IfxGtm_Cmu_enableClocks_callCount++; }
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return mock_IfxGtm_Cmu_getClkFrequency_returnValue; }
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return mock_IfxGtm_Cmu_getEclkFrequency_returnValue; }
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fx) { (void)gtm; (void)fx; return mock_IfxGtm_Cmu_getFxClkFrequency_returnValue; }
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) { (void)gtm; return mock_IfxGtm_Cmu_getGclkFrequency_returnValue; }
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm) { (void)gtm; return (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxGtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; }
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return mock_IfxGtm_Cmu_isClkClockEnabled_returnValue; }
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue; }
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fx) { (void)gtm; (void)fx; return mock_IfxGtm_Cmu_isFxClockEnabled_returnValue; }
void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, IfxGtm_Cmu_Clk input) { (void)gtm; (void)clk; (void)input; }
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq) { (void)gtm; (void)clk; (void)freq; }
void IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk, float32 freq) { (void)gtm; (void)eclk; (void)freq; }
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency) { (void)gtm; (void)frequency; }

/* ===== Stubs: IfxGtm_Tom_Timer.h ===== */
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) { (void)driver; (void)config; mock_IfxGtm_Tom_Timer_init_callCount++; return mock_IfxGtm_Tom_Timer_init_returnValue; }
void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_run_callCount++; }
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_applyUpdate_callCount++; }
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_disableUpdate_callCount++; }
void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm) { (void)config; (void)gtm; mock_IfxGtm_Tom_Timer_initConfig_callCount++; }

/* ===== Stubs: IfxGtm_Tom_PwmHl.h ===== */
void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config) { (void)config; mock_IfxGtm_Tom_PwmHl_initConfig_callCount++; }
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode) { (void)driver; (void)mode; mock_IfxGtm_Tom_PwmHl_setMode_callCount++; return mock_IfxGtm_Tom_PwmHl_setMode_returnValue; }
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config) { (void)driver; (void)config; mock_IfxGtm_Tom_PwmHl_init_callCount++; return mock_IfxGtm_Tom_PwmHl_init_returnValue; }
void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn) { (void)driver; (void)tOn; mock_IfxGtm_Tom_PwmHl_setOnTime_callCount++; }

/* ===== Stubs: IfxPort.h ===== */
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_IfxPort_setPinHigh_callCount++; mock_togglePin_callCount++; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_IfxPort_setPinLow_callCount++; mock_togglePin_callCount++; }

/* ===== Spy getters ===== */
int  mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(void){ return mock_IfxGtm_Tom_PwmHl_initConfig_callCount; }
int  mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(void){ return mock_IfxGtm_Tom_PwmHl_setMode_callCount; }
int  mock_IfxGtm_Tom_PwmHl_init_getCallCount(void){ return mock_IfxGtm_Tom_PwmHl_init_callCount; }
int  mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(void){ return mock_IfxGtm_Tom_PwmHl_setOnTime_callCount; }
int  mock_IfxGtm_Tom_Timer_init_getCallCount(void){ return mock_IfxGtm_Tom_Timer_init_callCount; }
int  mock_IfxGtm_Tom_Timer_run_getCallCount(void){ return mock_IfxGtm_Tom_Timer_run_callCount; }
int  mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void){ return mock_IfxGtm_Tom_Timer_applyUpdate_callCount; }
int  mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void){ return mock_IfxGtm_Tom_Timer_disableUpdate_callCount; }
int  mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void){ return mock_IfxGtm_Tom_Timer_initConfig_callCount; }
int  mock_IfxGtm_Cmu_enableClocks_getCallCount(void){ return mock_IfxGtm_Cmu_enableClocks_callCount; }
int  mock_IfxGtm_enable_getCallCount(void){ return mock_IfxGtm_enable_callCount; }
int  mock_IfxPort_setPinHigh_getCallCount(void){ return mock_IfxPort_setPinHigh_callCount; }
int  mock_IfxPort_setPinLow_getCallCount(void){ return mock_IfxPort_setPinLow_callCount; }
int  mock_togglePin_getCallCount(void){ return (int)mock_togglePin_callCount; }

/* ===== Reset ===== */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_init_lastNumChannels = 0u;
    mock_init_lastFrequency = 0.0f;
    for (int i=0;i<MOCK_MAX_CHANNELS;i++){ mock_update_lastDuties[i]=0.0f; mock_dt_lastDtRising[i]=0.0f; mock_dt_lastDtFalling[i]=0.0f; }
    mock_togglePin_callCount = 0u;

    mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount    = 0;
    mock_IfxGtm_Tom_PwmHl_init_callCount       = 0;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount  = 0;
    mock_IfxGtm_Tom_Timer_init_callCount       = 0;
    mock_IfxGtm_Tom_Timer_run_callCount        = 0;
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount= 0;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount=0;
    mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount     = 0;
    mock_IfxGtm_enable_callCount               = 0;
    mock_IfxPort_setPinHigh_callCount          = 0;
    mock_IfxPort_setPinLow_callCount           = 0;

    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_init_returnValue    = FALSE;
    mock_IfxGtm_Tom_Timer_init_returnValue    = FALSE;
    mock_IfxGtm_isEnabled_returnValue         = FALSE;
    mock_IfxGtm_isModuleSuspended_returnValue = FALSE;
    mock_IfxGtm_getSysClkFrequency_returnValue= 0.0f;
    mock_IfxGtm_getClusterFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_getClkFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_getEclkFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_getFxClkFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_getGclkFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue=0.0f;
    mock_IfxGtm_Cmu_isClkClockEnabled_returnValue=FALSE;
    mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue=FALSE;
    mock_IfxGtm_Cmu_isFxClockEnabled_returnValue=FALSE;
}
