#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* Spy state definitions */
int mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxGtm_Cmu_enable_callCount = 0;
int mock_IfxGtm_Cmu_isEnabled_callCount = 0;
int mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxGtm_isEnabled_callCount = 0;
int mock_IfxGtm_enable_callCount = 0;
int mock_IfxGtm_Pwm_init_callCount = 0;
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int mock_IfxGtm_Pwm_initConfig_callCount = 0;
int mock_IfxGtm_Pwm_getChannelState_callCount = 0;
uint32 mock_togglePin_callCount = 0;
uint32 mock_setPinModeOutput_callCount = 0;

boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;

uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

enum IfxGtm_Pwm_ChannelState mock_IfxGtm_Pwm_getChannelState_returnValue = IfxGtm_Pwm_ChannelState_running;

/* Bounded duty copy needs captured numChannels */
static uint32 _captured_numChannels = 0;

/* MODULE_* instance definitions */
Ifx_GTM MODULE_GTM = {0};
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
Ifx_GPT12 MODULE_GPT120 = {0};
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

/* Required Pin Symbol allocations */
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1N_TOUT14_P00_5_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2N_TOUT15_P00_6_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3N_TOUT16_P00_7_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_5_TOUT11_P00_2_OUT = {0};

/* Stubs: IfxGtm */
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

/* Stubs: IfxGtm_Cmu */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm; (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *module, float32 frequency)
{
    (void)module; (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *module, IfxGtm_Cmu_Clk clk, float32 frequency)
{
    (void)module; (void)clk; (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
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

/* Stubs: IfxGtm_Pwm */
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

IfxGtm_Pwm_ChannelState IfxGtm_Pwm_getChannelState(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch channel)
{
    (void)pwm; (void)channel;
    mock_IfxGtm_Pwm_getChannelState_callCount++;
    return mock_IfxGtm_Pwm_getChannelState_returnValue;
}

/* Stubs: IfxPort basic functions used by production/tests */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_togglePin_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_PadDriver padDriver)
{
    (void)port; (void)pinIndex; (void)mode; (void)padDriver;
    mock_setPinModeOutput_callCount++;
}

/* Mock control API implementations */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_getChannelState_callCount = 0;
    mock_togglePin_callCount = 0;
    mock_setPinModeOutput_callCount = 0;

    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Pwm_getChannelState_returnValue = IfxGtm_Pwm_ChannelState_running;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    _captured_numChannels = 0;
}

int  mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int  mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int  mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int  mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int  mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int  mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }
int  mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int  mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int  mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int  mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int  mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int  mock_IfxGtm_Pwm_getChannelState_getCallCount(void) { return mock_IfxGtm_Pwm_getChannelState_callCount; }
uint32 mock_IfxPort_togglePin_getCallCount(void) { return mock_togglePin_callCount; }
uint32 mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_setPinModeOutput_callCount; }
