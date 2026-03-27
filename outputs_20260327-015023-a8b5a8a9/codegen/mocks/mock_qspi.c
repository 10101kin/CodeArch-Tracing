#include "mock_qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"
#include <string.h>

/* Spy counters and return values (definitions) */
int     mock_IfxXspi_Spi_transferInit_callCount      = 0;
int     mock_IfxXspi_Spi_exchange_callCount          = 0;
int     mock_IfxXspi_Spi_isrDmaReceive_callCount     = 0;
int     mock_IfxXspi_Spi_setXspiGpioPins_callCount   = 0;
int     mock_IfxXspi_Spi_initModule_callCount        = 0;
int     mock_IfxXspi_Spi_initModuleConfig_callCount  = 0;
int     mock_IfxXspi_Spi_isrTransmit_callCount       = 0;
int     mock_IfxXspi_Spi_isrReceive_callCount        = 0;
boolean mock_IfxXspi_Spi_exchange_returnValue        = FALSE;

int     mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int     mock_IfxCpu_Irq_getTos_callCount                 = 0;

int     mock_IfxPort_getPinState_callCount           = 0;
int     mock_IfxPort_setPinFunctionMode_callCount    = 0;
int     mock_IfxPort_setPinHigh_callCount            = 0;
int     mock_IfxPort_setPinLow_callCount             = 0;
int     mock_IfxPort_setPinModeInput_callCount       = 0;
int     mock_IfxPort_setPinModeOutput_callCount      = 0;
int     mock_IfxPort_setPinState_callCount           = 0;
int     mock_IfxPort_togglePin_callCount             = 0;
uint32  mock_togglePin_callCount                     = 0;

/* Value-capture (generic) */
uint32  mock_IfxXspi_Spi_initModule_lastNumChannels  = 0;
float32 mock_IfxXspi_Spi_initModule_lastFrequency    = 0.0f;
float32 mock_IfxXspi_Spi_exchange_lastDuties[MOCK_MAX_CHANNELS];
float32 mock_IfxXspi_Spi_transferInit_lastDtRising[MOCK_MAX_CHANNELS];
float32 mock_IfxXspi_Spi_transferInit_lastDtFalling[MOCK_MAX_CHANNELS];

/* MODULE_* instance definitions */
Ifx_GTM    MODULE_GTM;
Ifx_XSPI   MODULE_XSPI;
Ifx_CPU    MODULE_CPU;
Ifx_P      MODULE_P00; Ifx_P MODULE_P01; Ifx_P MODULE_P02; Ifx_P MODULE_P10; Ifx_P MODULE_P11; Ifx_P MODULE_P12; Ifx_P MODULE_P13; Ifx_P MODULE_P14; Ifx_P MODULE_P15; Ifx_P MODULE_P20; Ifx_P MODULE_P21; Ifx_P MODULE_P22; Ifx_P MODULE_P23; Ifx_P MODULE_P24; Ifx_P MODULE_P25; Ifx_P MODULE_P26; Ifx_P MODULE_P30; Ifx_P MODULE_P31; Ifx_P MODULE_P32; Ifx_P MODULE_P33; Ifx_P MODULE_P34; Ifx_P MODULE_P40; Ifx_P MODULE_P41;
Ifx_ASCLIN0 MODULE_ASCLIN0; Ifx_ASCLIN1 MODULE_ASCLIN1; Ifx_ASCLIN2 MODULE_ASCLIN2; Ifx_ASCLIN3 MODULE_ASCLIN3; Ifx_ASCLIN4 MODULE_ASCLIN4; Ifx_ASCLIN5 MODULE_ASCLIN5; Ifx_ASCLIN6 MODULE_ASCLIN6; Ifx_ASCLIN7 MODULE_ASCLIN7; Ifx_ASCLIN8 MODULE_ASCLIN8; Ifx_ASCLIN9 MODULE_ASCLIN9; Ifx_ASCLIN10 MODULE_ASCLIN10; Ifx_ASCLIN11 MODULE_ASCLIN11; Ifx_ASCLIN12 MODULE_ASCLIN12; Ifx_ASCLIN13 MODULE_ASCLIN13; Ifx_ASCLIN14 MODULE_ASCLIN14; Ifx_ASCLIN15 MODULE_ASCLIN15; Ifx_ASCLIN16 MODULE_ASCLIN16; Ifx_ASCLIN17 MODULE_ASCLIN17; Ifx_ASCLIN18 MODULE_ASCLIN18; Ifx_ASCLIN19 MODULE_ASCLIN19; Ifx_ASCLIN20 MODULE_ASCLIN20; Ifx_ASCLIN21 MODULE_ASCLIN21; Ifx_ASCLIN22 MODULE_ASCLIN22; Ifx_ASCLIN23 MODULE_ASCLIN23;
Ifx_CAN0 MODULE_CAN0; Ifx_CAN1 MODULE_CAN1; Ifx_CAN2 MODULE_CAN2;
Ifx_CBS MODULE_CBS; Ifx_CCU60 MODULE_CCU60; Ifx_CCU61 MODULE_CCU61; Ifx_CONVCTRL MODULE_CONVCTRL; Ifx_CPU0 MODULE_CPU0; Ifx_CPU1 MODULE_CPU1; Ifx_CPU2 MODULE_CPU2; Ifx_CPU3 MODULE_CPU3; Ifx_DAM0 MODULE_DAM0; Ifx_DMA MODULE_DMA; Ifx_DMU MODULE_DMU; Ifx_DOM0 MODULE_DOM0; Ifx_EDSADC MODULE_EDSADC; Ifx_ERAY0 MODULE_ERAY0; Ifx_ERAY1 MODULE_ERAY1; Ifx_EVADC MODULE_EVADC; Ifx_FCE MODULE_FCE; Ifx_FSI MODULE_FSI; Ifx_GETH MODULE_GETH; Ifx_GPT120 MODULE_GPT120; Ifx_HSCT0 MODULE_HSCT0; Ifx_HSSL0 MODULE_HSSL0; Ifx_I2C0 MODULE_I2C0; Ifx_I2C1 MODULE_I2C1; Ifx_INT MODULE_INT; Ifx_IOM MODULE_IOM; Ifx_LMU0 MODULE_LMU0; Ifx_MINIMCDS MODULE_MINIMCDS; Ifx_MSC0 MODULE_MSC0; Ifx_MSC1 MODULE_MSC1; Ifx_MSC2 MODULE_MSC2; Ifx_MTU MODULE_MTU; Ifx_PFI0 MODULE_PFI0; Ifx_PFI1 MODULE_PFI1; Ifx_PFI2 MODULE_PFI2; Ifx_PFI3 MODULE_PFI3; Ifx_PMS MODULE_PMS; Ifx_PMU MODULE_PMU; Ifx_PSI5S MODULE_PSI5S; Ifx_PSI5 MODULE_PSI5; Ifx_QSPI0 MODULE_QSPI0; Ifx_QSPI1 MODULE_QSPI1; Ifx_QSPI2 MODULE_QSPI2; Ifx_QSPI3 MODULE_QSPI3; Ifx_QSPI4 MODULE_QSPI4; Ifx_SBCU MODULE_SBCU; Ifx_SCU MODULE_SCU; Ifx_SENT MODULE_SENT; Ifx_SMU MODULE_SMU; Ifx_SRC MODULE_SRC; Ifx_STM0 MODULE_STM0; Ifx_STM1 MODULE_STM1; Ifx_STM2 MODULE_STM2; Ifx_STM3 MODULE_STM3;

/* Stub bodies (do not access struct members) */

/* IfxXspi_Spi */
void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    (void)xspi; (void)config; mock_IfxXspi_Spi_transferInit_callCount++;
}

boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig)
{
    (void)xspi; (void)jobConfig; mock_IfxXspi_Spi_exchange_callCount++; return mock_IfxXspi_Spi_exchange_returnValue;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    (void)xspi; mock_IfxXspi_Spi_isrDmaReceive_callCount++; return 0U;
}

void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins)
{
    (void)xspi; (void)pins; mock_IfxXspi_Spi_setXspiGpioPins_callCount++;
}

IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    (void)xspi; (void)config; mock_IfxXspi_Spi_initModule_callCount++; return (IfxXspi_Status)0;
}

void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi)
{
    (void)config; (void)xspi; mock_IfxXspi_Spi_initModuleConfig_callCount++;
}

void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi)
{
    (void)xspi; mock_IfxXspi_Spi_isrTransmit_callCount++;
}

uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi)
{
    (void)xspi; mock_IfxXspi_Spi_isrReceive_callCount++; return 0U;
}

/* IfxCpu_Irq */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    (void)isrFuncPointer; (void)serviceReqPrioNumber; mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

IfxSrc_Tos IfxCpu_Irq_getTos(void)
{
    mock_IfxCpu_Irq_getTos_callCount++; return IfxSrc_Tos_cpu0;
}

/* IfxPort */
IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex; mock_IfxPort_getPinState_callCount++; return IfxPort_State_notChanged;
}

void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{
    (void)port; (void)pinIndex; (void)mode; mock_IfxPort_setPinFunctionMode_callCount++;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex; mock_IfxPort_setPinHigh_callCount++;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex; mock_IfxPort_setPinLow_callCount++;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    (void)port; (void)pinIndex; (void)mode; mock_IfxPort_setPinModeInput_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index; mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    (void)port; (void)pinIndex; (void)action; mock_IfxPort_setPinState_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex; mock_IfxPort_togglePin_callCount++; mock_togglePin_callCount++;
}

/* Mock control API */
void mock_qspi_reset(void)
{
    mock_IfxXspi_Spi_transferInit_callCount = 0;
    mock_IfxXspi_Spi_exchange_callCount = 0;
    mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;
    mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
    mock_IfxXspi_Spi_initModule_callCount = 0;
    mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
    mock_IfxXspi_Spi_isrTransmit_callCount = 0;
    mock_IfxXspi_Spi_isrReceive_callCount = 0;
    mock_IfxXspi_Spi_exchange_returnValue = FALSE;

    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxCpu_Irq_getTos_callCount = 0;

    mock_IfxPort_getPinState_callCount = 0;
    mock_IfxPort_setPinFunctionMode_callCount = 0;
    mock_IfxPort_setPinHigh_callCount = 0;
    mock_IfxPort_setPinLow_callCount = 0;
    mock_IfxPort_setPinModeInput_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_setPinState_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_togglePin_callCount = 0U;

    mock_IfxXspi_Spi_initModule_lastNumChannels = 0U;
    mock_IfxXspi_Spi_initModule_lastFrequency = 0.0f;
    memset(mock_IfxXspi_Spi_exchange_lastDuties, 0, sizeof(mock_IfxXspi_Spi_exchange_lastDuties));
    memset(mock_IfxXspi_Spi_transferInit_lastDtRising, 0, sizeof(mock_IfxXspi_Spi_transferInit_lastDtRising));
    memset(mock_IfxXspi_Spi_transferInit_lastDtFalling, 0, sizeof(mock_IfxXspi_Spi_transferInit_lastDtFalling));
}

int  mock_IfxXspi_Spi_transferInit_getCallCount(void)     { return mock_IfxXspi_Spi_transferInit_callCount; }
int  mock_IfxXspi_Spi_exchange_getCallCount(void)         { return mock_IfxXspi_Spi_exchange_callCount; }
int  mock_IfxXspi_Spi_isrDmaReceive_getCallCount(void)    { return mock_IfxXspi_Spi_isrDmaReceive_callCount; }
int  mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(void)  { return mock_IfxXspi_Spi_setXspiGpioPins_callCount; }
int  mock_IfxXspi_Spi_initModule_getCallCount(void)       { return mock_IfxXspi_Spi_initModule_callCount; }
int  mock_IfxXspi_Spi_initModuleConfig_getCallCount(void) { return mock_IfxXspi_Spi_initModuleConfig_callCount; }
int  mock_IfxXspi_Spi_isrTransmit_getCallCount(void)      { return mock_IfxXspi_Spi_isrTransmit_callCount; }
int  mock_IfxXspi_Spi_isrReceive_getCallCount(void)       { return mock_IfxXspi_Spi_isrReceive_callCount; }
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int  mock_IfxCpu_Irq_getTos_getCallCount(void)            { return mock_IfxCpu_Irq_getTos_callCount; }
int  mock_IfxPort_getPinState_getCallCount(void)          { return mock_IfxPort_getPinState_callCount; }
int  mock_IfxPort_setPinFunctionMode_getCallCount(void)   { return mock_IfxPort_setPinFunctionMode_callCount; }
int  mock_IfxPort_setPinHigh_getCallCount(void)           { return mock_IfxPort_setPinHigh_callCount; }
int  mock_IfxPort_setPinLow_getCallCount(void)            { return mock_IfxPort_setPinLow_callCount; }
int  mock_IfxPort_setPinModeInput_getCallCount(void)      { return mock_IfxPort_setPinModeInput_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void)     { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxPort_setPinState_getCallCount(void)          { return mock_IfxPort_setPinState_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void)            { return mock_IfxPort_togglePin_callCount; }
