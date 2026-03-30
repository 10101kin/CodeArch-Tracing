/* mock_gtm_tom_3_phase_inverter_pwm.c - Spy state + stub bodies + MODULE_* definitions */
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

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
Ifx_CONVERTER MODULE_CONVCTRL = {0};
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
Ifx_GTM_TOM MODULE_GTM_TOM = {0};
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

/* Pin symbol allocations (required) */
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0N_TOUT7_P02_7_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0_TOUT0_P02_0_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_10_TOUT2_P02_2_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_12_TOUT4_P02_4_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_13_TOUT5_P02_5_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1_TOUT1_P02_1_OUT = {0};

/* Spy state definitions */
uint32  mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0;
float32 mock_IfxGtm_Tom_PwmHl_init_lastFrequency  = 0.0f;
uint32  mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency  = 0.0f;

float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

uint32 mock_togglePin_callCount = 0;

int mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_getPeriod_callCount = 0;
int mock_IfxGtm_Tom_Timer_init_callCount = 0;
int mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
int mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
Ifx_TimerValue mock_IfxGtm_Tom_Timer_getPeriod_returnValue = 0u;
boolean       mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;

int mock_IfxGtm_isEnabled_callCount = 0;
int mock_IfxGtm_enable_callCount = 0;
boolean mock_IfxGtm_isEnabled_returnValue = FALSE;

int mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int mock_IfxGtm_Cmu_enable_callCount = 0;
int mock_IfxGtm_Cmu_isEnabled_callCount = 0;
int mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

int mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
boolean mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

int mock_IfxGtm_PinMap_setTomTout_callCount = 0;

int mock_IfxPort_togglePin_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;

/* Internal channel count capture bound */
static uint32 _captured_numChannels = 0;

/* Stub bodies */
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount++;
}

Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_getPeriod_callCount++;
    return mock_IfxGtm_Tom_Timer_getPeriod_returnValue;
}

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{
    (void)driver;
    (void)config;
    mock_IfxGtm_Tom_Timer_init_callCount++;
    return mock_IfxGtm_Tom_Timer_init_returnValue;
}

void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm)
{
    (void)config;
    (void)gtm;
    mock_IfxGtm_Tom_Timer_initConfig_callCount++;
}

void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount++;
}

boolean IfxGtm_isEnabled(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_isEnabled_callCount++;
    return mock_IfxGtm_isEnabled_returnValue;
}

void IfxGtm_enable(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_enable_callCount++;
}

void IfxGtm_Cmu_enable(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_enable_callCount++;
}

boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_isEnabled_callCount++;
    return mock_IfxGtm_Cmu_isEnabled_returnValue;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    /* Provide sensible default if not overridden */
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* 100 MHz default */
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *module, float32 frequency)
{
    (void)module;
    (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *module, IfxGtm_Cmu_Clk clk, float32 frequency)
{
    (void)module;
    (void)clk;
    (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
}

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm;
    (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount++;
    /* Record same value for rising/falling for index 0 as a minimal behavior */
    if (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS)
    {
        for (uint32 i = 0; i < _captured_numChannels; ++i)
        {
            mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[i] = deadtime;
            mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[i] = deadtime;
        }
    }
    else
    {
        for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
        {
            mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[i] = deadtime;
            mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[i] = deadtime;
        }
    }
    return mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue;
}

boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = (uint32)config->base.channelCount;
        mock_IfxGtm_Tom_PwmHl_init_lastFrequency   = config->base.frequency;
        _captured_numChannels = (uint32)config->base.channelCount;
    }
    return mock_IfxGtm_Tom_PwmHl_init_returnValue;
}

void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (tOn != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = (float32)tOn[i];
        }
    }
}

boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode)
{
    (void)driver;
    (void)mode;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount++;
    return mock_IfxGtm_Tom_PwmHl_setMode_returnValue;
}

void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config)
{
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = (uint32)config->base.channelCount;
        mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency   = config->base.frequency;
        _captured_numChannels = (uint32)config->base.channelCount;
    }
}

void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config;
    (void)outputMode;
    (void)padDriver;
    mock_IfxGtm_PinMap_setTomTout_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port;
    (void)pinIndex;
    (void)mode;
    (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

/* Spy getters */
int mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_applyUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_getPeriod_getCallCount(void) { return mock_IfxGtm_Tom_Timer_getPeriod_callCount; }
int mock_IfxGtm_Tom_Timer_init_getCallCount(void) { return mock_IfxGtm_Tom_Timer_init_callCount; }
int mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_Timer_initConfig_callCount; }
int mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_disableUpdate_callCount; }

int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }

int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }

int mock_IfxGtm_Tom_PwmHl_setDeadtime_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount; }
int mock_IfxGtm_Tom_PwmHl_init_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_init_callCount; }
int mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setOnTime_callCount; }
int mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setMode_callCount; }
int mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_initConfig_callCount; }

int mock_IfxGtm_PinMap_setTomTout_getCallCount(void) { return mock_IfxGtm_PinMap_setTomTout_callCount; }

int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }

/* Reset function */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_getPeriod_callCount = 0;
    mock_IfxGtm_Tom_Timer_init_callCount = 0;
    mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = 0u;
    mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;

    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_isEnabled_returnValue = FALSE;

    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

    mock_IfxGtm_PinMap_setTomTout_callCount = 0;

    mock_IfxPort_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_togglePin_callCount = 0;

    mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0;
    mock_IfxGtm_Tom_PwmHl_init_lastFrequency  = 0.0f;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency  = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[i] = 0.0f;
    }
    _captured_numChannels = 0;
}
