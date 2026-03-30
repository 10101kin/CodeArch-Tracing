#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* Spy state - counters and return values */
int     mock_IfxGtm_isEnabled_callCount = 0;
boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
int     mock_IfxGtm_enable_callCount = 0;

int     mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int     mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxGtm_Cmu_enable_callCount = 0;
int     mock_IfxGtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

int     mock_IfxGtm_Pwm_initConfig_callCount = 0;
int     mock_IfxGtm_Pwm_init_callCount = 0;
int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int     mock_IfxPort_setPinModeOutput_callCount = 0;
int     mock_IfxPort_setPinMode_callCount = 0;
int     mock_IfxPort_togglePin_callCount = 0;
uint32  mock_togglePin_callCount = 0;

int     mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

/* Captured channel count for bounded copies */
static uint32 _captured_numChannels = 0;

/* MODULE_* instances */
Ifx_ASCLIN MODULE_ASCLIN0;
Ifx_ASCLIN MODULE_ASCLIN1;
Ifx_ASCLIN MODULE_ASCLIN2;
Ifx_ASCLIN MODULE_ASCLIN3;
Ifx_ASCLIN MODULE_ASCLIN4;
Ifx_ASCLIN MODULE_ASCLIN5;
Ifx_ASCLIN MODULE_ASCLIN6;
Ifx_ASCLIN MODULE_ASCLIN7;
Ifx_ASCLIN MODULE_ASCLIN8;
Ifx_ASCLIN MODULE_ASCLIN9;
Ifx_ASCLIN MODULE_ASCLIN10;
Ifx_ASCLIN MODULE_ASCLIN11;
Ifx_ASCLIN MODULE_ASCLIN12;
Ifx_ASCLIN MODULE_ASCLIN13;
Ifx_ASCLIN MODULE_ASCLIN14;
Ifx_ASCLIN MODULE_ASCLIN15;
Ifx_ASCLIN MODULE_ASCLIN16;
Ifx_ASCLIN MODULE_ASCLIN17;
Ifx_ASCLIN MODULE_ASCLIN18;
Ifx_ASCLIN MODULE_ASCLIN19;
Ifx_ASCLIN MODULE_ASCLIN20;
Ifx_ASCLIN MODULE_ASCLIN21;
Ifx_ASCLIN MODULE_ASCLIN22;
Ifx_ASCLIN MODULE_ASCLIN23;
Ifx_CAN MODULE_CAN0;
Ifx_CAN MODULE_CAN1;
Ifx_CAN MODULE_CAN2;
Ifx_CBS MODULE_CBS;
Ifx_CCU6 MODULE_CCU60;
Ifx_CCU6 MODULE_CCU61;
Ifx_CONVCTRL MODULE_CONVCTRL;
Ifx_CPU MODULE_CPU0;
Ifx_CPU MODULE_CPU1;
Ifx_CPU MODULE_CPU2;
Ifx_CPU MODULE_CPU3;
Ifx_DAM MODULE_DAM0;
Ifx_DMA MODULE_DMA;
Ifx_DMU MODULE_DMU;
Ifx_DOM MODULE_DOM0;
Ifx_EDSADC MODULE_EDSADC;
Ifx_ERAY MODULE_ERAY0;
Ifx_ERAY MODULE_ERAY1;
Ifx_EVADC MODULE_EVADC;
Ifx_FCE MODULE_FCE;
Ifx_FSI MODULE_FSI;
Ifx_GETH MODULE_GETH;
Ifx_GPT12 MODULE_GPT120;
Ifx_GTM MODULE_GTM;
Ifx_HSCT MODULE_HSCT0;
Ifx_HSSL MODULE_HSSL0;
Ifx_I2C MODULE_I2C0;
Ifx_I2C MODULE_I2C1;
Ifx_INT MODULE_INT;
Ifx_IOM MODULE_IOM;
Ifx_LMU MODULE_LMU0;
Ifx_MINIMCDS MODULE_MINIMCDS;
Ifx_MSC MODULE_MSC0;
Ifx_MSC MODULE_MSC1;
Ifx_MSC MODULE_MSC2;
Ifx_MTU MODULE_MTU;
Ifx_PFI MODULE_PFI0;
Ifx_PFI MODULE_PFI1;
Ifx_PFI MODULE_PFI2;
Ifx_PFI MODULE_PFI3;
Ifx_PMS MODULE_PMS;
Ifx_PMU MODULE_PMU;
Ifx_P MODULE_P00;
Ifx_P MODULE_P01;
Ifx_P MODULE_P02;
Ifx_P MODULE_P10;
Ifx_P MODULE_P11;
Ifx_P MODULE_P12;
Ifx_P MODULE_P13;
Ifx_P MODULE_P14;
Ifx_P MODULE_P15;
Ifx_P MODULE_P20;
Ifx_P MODULE_P21;
Ifx_P MODULE_P22;
Ifx_P MODULE_P23;
Ifx_P MODULE_P24;
Ifx_P MODULE_P25;
Ifx_P MODULE_P26;
Ifx_P MODULE_P30;
Ifx_P MODULE_P31;
Ifx_P MODULE_P32;
Ifx_P MODULE_P33;
Ifx_P MODULE_P34;
Ifx_P MODULE_P40;
Ifx_P MODULE_P41;
Ifx_PSI5S MODULE_PSI5S;
Ifx_PSI5 MODULE_PSI5;
Ifx_QSPI MODULE_QSPI0;
Ifx_QSPI MODULE_QSPI1;
Ifx_QSPI MODULE_QSPI2;
Ifx_QSPI MODULE_QSPI3;
Ifx_QSPI MODULE_QSPI4;
Ifx_SBCU MODULE_SBCU;
Ifx_SCU MODULE_SCU;
Ifx_SENT MODULE_SENT;
Ifx_SMU MODULE_SMU;
Ifx_SRC MODULE_SRC;
Ifx_STM MODULE_STM0;
Ifx_STM MODULE_STM1;
Ifx_STM MODULE_STM2;
Ifx_STM MODULE_STM3;

/* Stub bodies */
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

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm;
    (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency)
{
    (void)gtm;
    (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)gtm;
    (void)clkIndex;
    (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxGtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* sensible default: 100 MHz */
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    mock_IfxGtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_initConfig_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    mock_IfxGtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Pwm_init_lastNumChannels = (uint32)config->numChannels;
        mock_IfxGtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = (uint32)config->numChannels;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
               ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port;
    (void)pinIndex;
    (void)mode;
    (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port;
    (void)pinIndex;
    (void)mode;
    mock_IfxPort_setPinMode_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

typedef void (*Ifx_Isr)(void);
void IfxCpu_Irq_installInterruptHandler(Ifx_Isr isr, int priority)
{
    (void)isr;
    (void)priority;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

/* Getters */
int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_setPinMode_getCallCount(void) { return mock_IfxPort_setPinMode_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }

void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_enable_callCount = 0;

    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;

    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;
    mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount = 0;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxGtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxGtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }

    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_setPinMode_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0u;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;

    _captured_numChannels = 0u;
}
