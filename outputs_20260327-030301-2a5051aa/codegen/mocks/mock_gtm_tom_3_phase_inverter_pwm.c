#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"

/* Spy state definitions */
int mock_IfxGtm_PinMap_setTomTout_callCount = 0;
int mock_IfxGtm_enable_callCount = 0;
int mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
int mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_run_callCount = 0;
int mock_IfxGtm_Tom_Timer_init_callCount = 0;
int mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
int mock_IfxGtm_Cmu_enableClocks_callCount = 0;

boolean mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

uint32  mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0;
float32 mock_IfxGtm_Tom_PwmHl_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0u;

/* MODULE_* instance definitions */
Ifx_GTM MODULE_GTM = {0};
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

/* ======= Stub bodies (with counters) ======= */
/* IfxGtm_PinMap */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config; (void)outputMode; (void)padDriver;
    mock_IfxGtm_PinMap_setTomTout_callCount++;
}

/* IfxGtm */
boolean IfxGtm_isEnabled(Ifx_GTM *gtm) { (void)gtm; return TRUE; }
boolean IfxGtm_isModuleSuspended(Ifx_GTM *gtm) { (void)gtm; return FALSE; }
void IfxGtm_setSuspendMode(Ifx_GTM *gtm, int mode) { (void)gtm; (void)mode; }
void IfxGtm_disable(Ifx_GTM *gtm) { (void)gtm; }
void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }
float32 IfxGtm_getSysClkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
float32 IfxGtm_getClusterFrequency(Ifx_GTM *gtm, IfxGtm_Cluster cluster) { (void)gtm; (void)cluster; return 0.0f; }

/* IfxGtm_Cmu */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) { (void)gtm; (void)clkMask; mock_IfxGtm_Cmu_enableClocks_callCount++; }
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return 0.0f; }
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return 0.0f; }
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return 0.0f; }
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm) { (void)gtm; return 0.0f; }
static float32 s_mock_Cmu_getModuleFreq = 0.0f; /* internal storage for return control if needed */
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm) { (void)gtm; return (s_mock_Cmu_getModuleFreq != 0.0f) ? s_mock_Cmu_getModuleFreq : 100000000.0f; }
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; return TRUE; }
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk) { (void)gtm; (void)eclk; return TRUE; }
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk) { (void)gtm; (void)fxclk; return TRUE; }
void IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk) { (void)gtm; (void)clk; }
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq) { (void)gtm; (void)clk; (void)freq; }
void IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk, float32 freq) { (void)gtm; (void)eclk; (void)freq; }
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency) { (void)gtm; s_mock_Cmu_getModuleFreq = frequency; }

/* IfxGtm_Tom_Timer */
void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm)
{
    (void)gtm; mock_IfxGtm_Tom_Timer_initConfig_callCount++;
    if (config != NULL_PTR) {
        /* capture from config->base.frequency if available */
        mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = config->base.frequency;
    }
}
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_applyUpdate_callCount++; }
void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_run_callCount++; }
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{ (void)driver; mock_IfxGtm_Tom_Timer_init_callCount++; (void)config; return mock_IfxGtm_Tom_Timer_init_returnValue; }
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_disableUpdate_callCount++; }
void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount++; }

/* Additional Timer API stubs required by header */
void IfxGtm_Tom_Timer_stdIfTimerInit(IfxGtm_Tom_Timer *driver) { (void)driver; }
void IfxStdIf_Timer_run(IfxGtm_Tom_Timer *driver) { (void)driver; }
void IfxStdIf_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; }
void IfxStdIf_Timer_setPeriod(IfxGtm_Tom_Timer *driver, float32 period) { (void)driver; (void)period; }
void IfxStdIf_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; }
float32 IfxGtm_Tom_Timer_getOffset(IfxGtm_Tom_Timer *driver) { (void)driver; return 0.0f; }
void IfxGtm_Tom_Timer_acknowledgeTimerIrq(IfxGtm_Tom_Timer *driver) { (void)driver; }
void IfxGtm_Tom_Timer_acknowledgeTriggerIrq(IfxGtm_Tom_Timer *driver) { (void)driver; }
void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, uint16 mask0, uint16 mask1) { (void)driver; (void)mask0; (void)mask1; }
float32 IfxGtm_Tom_Timer_getFrequency(IfxGtm_Tom_Timer *driver) { (void)driver; return 0.0f; }
float32 IfxGtm_Tom_Timer_getInputFrequency(IfxGtm_Tom_Timer *driver) { (void)driver; return 0.0f; }
float32 IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver) { (void)driver; return 0.0f; }
float32 IfxGtm_Tom_Timer_getResolution(IfxGtm_Tom_Timer *driver) { (void)driver; return 0.0f; }
uint32  IfxGtm_Tom_Timer_getTrigger(IfxGtm_Tom_Timer *driver) { (void)driver; return 0u; }
void IfxGtm_Tom_Timer_setFrequency(IfxGtm_Tom_Timer *driver, float32 freq) { (void)driver; (void)freq; }
void IfxGtm_Tom_Timer_setPeriod(IfxGtm_Tom_Timer *driver, float32 period) { (void)driver; (void)period; }
void IfxGtm_Tom_Timer_setSingleMode(IfxGtm_Tom_Timer *driver, boolean single) { (void)driver; (void)single; }
void IfxGtm_Tom_Timer_setTrigger(IfxGtm_Tom_Timer *driver, uint32 trig) { (void)driver; (void)trig; }
void IfxGtm_Tom_Timer_stop(IfxGtm_Tom_Timer *driver) { (void)driver; }

/* IfxGtm_Tom_PwmHl */
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config)
{
    (void)driver; mock_IfxGtm_Tom_PwmHl_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = config->base.channelCount;
        mock_IfxGtm_Tom_PwmHl_init_lastFrequency = config->base.frequency;
    }
    return mock_IfxGtm_Tom_PwmHl_init_returnValue;
}
void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config)
{
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = config->base.channelCount;
        mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = config->base.frequency;
    }
}
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode)
{ (void)driver; (void)mode; mock_IfxGtm_Tom_PwmHl_setMode_callCount++; return mock_IfxGtm_Tom_PwmHl_setMode_returnValue; }
void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    (void)driver; mock_IfxGtm_Tom_PwmHl_setOnTime_callCount++;
    if (tOn != NULL_PTR) {
        /* Capture first MOCK_MAX_CHANNELS entries safely */
        for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
            mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = (float32)tOn[i];
        }
    }
}

/* Additional PwmHl API stubs required by header */
void IfxGtm_Tom_PwmHl_stdIfPwmHlInit(IfxGtm_Tom_PwmHl *driver) { (void)driver; }
Ifx_TimerValue IfxGtm_Tom_PwmHl_getDeadtime(IfxGtm_Tom_PwmHl *driver) { (void)driver; return 0u; }
Ifx_TimerValue IfxGtm_Tom_PwmHl_getMinPulse(IfxGtm_Tom_PwmHl *driver) { (void)driver; return 0u; }
Ifx_Pwm_Mode IfxGtm_Tom_PwmHl_getMode(IfxGtm_Tom_PwmHl *driver) { (void)driver; return 0; }
void IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue dt) { (void)driver; (void)dt; }
void IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue mp) { (void)driver; (void)mp; }
void IfxGtm_Tom_PwmHl_setOnTimeAndShift(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift) { (void)driver; (void)tOn; (void)shift; }
void IfxGtm_Tom_PwmHl_setPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff) { (void)driver; (void)tOn; (void)tOff; }
void IfxGtm_Tom_PwmHl_setupChannels(IfxGtm_Tom_PwmHl *driver) { (void)driver; }

/* Getters for call counters */
int mock_IfxGtm_PinMap_setTomTout_getCallCount(void) { return mock_IfxGtm_PinMap_setTomTout_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_Timer_initConfig_callCount; }
int mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_applyUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_run_getCallCount(void) { return mock_IfxGtm_Tom_Timer_run_callCount; }
int mock_IfxGtm_Tom_Timer_init_getCallCount(void) { return mock_IfxGtm_Tom_Timer_init_callCount; }
int mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_disableUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_updateInputFrequency_getCallCount(void) { return mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount; }
int mock_IfxGtm_Tom_PwmHl_init_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_init_callCount; }
int mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_initConfig_callCount; }
int mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setMode_callCount; }
int mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setOnTime_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }

/* Reset function */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxGtm_PinMap_setTomTout_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_run_callCount = 0;
    mock_IfxGtm_Tom_Timer_init_callCount = 0;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;

    mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

    mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0;
    mock_IfxGtm_Tom_PwmHl_init_lastFrequency = 0.0f;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDtFalling[i] = 0.0f;
    }
    mock_togglePin_callCount = 0u;
    s_mock_Cmu_getModuleFreq = 0.0f;
}
