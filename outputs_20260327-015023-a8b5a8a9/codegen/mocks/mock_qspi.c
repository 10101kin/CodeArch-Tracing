#include "mock_qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* Spy state definitions */
int mock_IfxXspi_Spi_transferInit_callCount = 0;
int mock_IfxXspi_Spi_exchange_callCount = 0;           boolean mock_IfxXspi_Spi_exchange_returnValue = FALSE;
int mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;      uint32  mock_IfxXspi_Spi_isrDmaReceive_returnValue = 0u;
int mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
int mock_IfxXspi_Spi_initModule_callCount = 0;         uint32  mock_IfxXspi_Spi_initModule_returnValue = 0u;
int mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
int mock_IfxXspi_Spi_isrTransmit_callCount = 0;
int mock_IfxXspi_Spi_isrReceive_callCount = 0;         uint32  mock_IfxXspi_Spi_isrReceive_returnValue = 0u;

int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int mock_IfxCpu_Irq_getTos_callCount = 0;              uint32  mock_IfxCpu_Irq_getTos_returnValue = (uint32)IfxSrc_Tos_cpu0;

int mock_IfxPort_getPinState_callCount = 0;            uint32  mock_IfxPort_getPinState_returnValue = 0u;
int mock_IfxPort_setPinFunctionMode_callCount = 0;
int mock_IfxPort_setPinHigh_callCount = 0;
int mock_IfxPort_setPinLow_callCount = 0;
int mock_IfxPort_setPinModeInput_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;
int mock_IfxPort_setPinState_callCount = 0;
int mock_IfxPort_togglePin_callCount = 0;
int mock_IfxPort_disableEmergencyStop_callCount = 0;
int mock_IfxPort_enableEmergencyStop_callCount = 0;
int mock_IfxPort_setPinMode_callCount = 0;

uint32  mock_init_lastNumChannels = 0u;
float32 mock_init_lastFrequency = 0.0f;
float32 mock_update_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0u;

void   *mock_IfxXspi_Spi_transferInit_lastXspi = NULL_PTR;
void   *mock_IfxXspi_Spi_transferInit_lastConfig = NULL_PTR;
void   *mock_IfxXspi_Spi_exchange_lastHandle = NULL_PTR;
void   *mock_IfxXspi_Spi_exchange_lastJobConfig = NULL_PTR;
void   *mock_IfxXspi_Spi_setXspiGpioPins_lastXspi = NULL_PTR;
void   *mock_IfxXspi_Spi_setXspiGpioPins_lastPins = NULL_PTR;
void   *mock_IfxXspi_Spi_initModule_lastHandle = NULL_PTR;
void   *mock_IfxXspi_Spi_initModule_lastConfig = NULL_PTR;
void   *mock_IfxXspi_Spi_initModuleConfig_lastConfig = NULL_PTR;
void   *mock_IfxXspi_Spi_initModuleConfig_lastXspi = NULL_PTR;
void   *mock_IfxCpu_Irq_installInterruptHandler_lastIsr = NULL_PTR;
uint32  mock_IfxCpu_Irq_installInterruptHandler_lastPrio = 0u;
void   *mock_IfxPort_lastPort = NULL_PTR;
uint32  mock_IfxPort_lastPinIndex = 0u;
uint32  mock_IfxPort_setPinState_lastAction = 0u;
uint32  mock_IfxPort_setPinModeOutput_lastMode = 0u;
uint32  mock_IfxPort_setPinModeOutput_lastIndex = 0u;
uint32  mock_IfxPort_setPinModeInput_lastMode = 0u;
uint32  mock_IfxPort_setPinMode_lastMode = 0u;

/* MODULE_* instances */
Ifx_XSPI MODULE_XSPI = {0};
Ifx_CPU  MODULE_CPU  = {0};
Ifx_P MODULE_P00 = {0}; Ifx_P MODULE_P01 = {0}; Ifx_P MODULE_P02 = {0}; Ifx_P MODULE_P10 = {0}; Ifx_P MODULE_P11 = {0}; Ifx_P MODULE_P12 = {0}; Ifx_P MODULE_P13 = {0}; Ifx_P MODULE_P14 = {0}; Ifx_P MODULE_P15 = {0}; Ifx_P MODULE_P20 = {0}; Ifx_P MODULE_P21 = {0}; Ifx_P MODULE_P22 = {0}; Ifx_P MODULE_P23 = {0}; Ifx_P MODULE_P24 = {0}; Ifx_P MODULE_P25 = {0}; Ifx_P MODULE_P26 = {0}; Ifx_P MODULE_P30 = {0}; Ifx_P MODULE_P31 = {0}; Ifx_P MODULE_P32 = {0}; Ifx_P MODULE_P33 = {0}; Ifx_P MODULE_P34 = {0}; Ifx_P MODULE_P40 = {0}; Ifx_P MODULE_P41 = {0};
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
Ifx_CBS  MODULE_CBS  = {0};
Ifx_CCU60 MODULE_CCU60 = {0};
Ifx_CCU61 MODULE_CCU61 = {0};
Ifx_CONVCTRL MODULE_CONVCTRL = {0};
Ifx_CPU0 MODULE_CPU0 = {0};
Ifx_CPU1 MODULE_CPU1 = {0};
Ifx_CPU2 MODULE_CPU2 = {0};
Ifx_CPU3 MODULE_CPU3 = {0};
Ifx_DAM0 MODULE_DAM0 = {0};
Ifx_DMA  MODULE_DMA  = {0};
Ifx_DMU  MODULE_DMU  = {0};
Ifx_DOM0 MODULE_DOM0 = {0};
Ifx_EDSADC MODULE_EDSADC = {0};
Ifx_ERAY0 MODULE_ERAY0 = {0};
Ifx_ERAY1 MODULE_ERAY1 = {0};
Ifx_EVADC MODULE_EVADC = {0};
Ifx_FCE   MODULE_FCE   = {0};
Ifx_FSI   MODULE_FSI   = {0};
Ifx_GETH  MODULE_GETH  = {0};
Ifx_GPT120 MODULE_GPT120 = {0};
Ifx_GTM   MODULE_GTM   = {0};
Ifx_HSCT0 MODULE_HSCT0 = {0};
Ifx_HSSL0 MODULE_HSSL0 = {0};
Ifx_I2C0  MODULE_I2C0  = {0};
Ifx_I2C1  MODULE_I2C1  = {0};
Ifx_INT   MODULE_INT   = {0};
Ifx_IOM   MODULE_IOM   = {0};
Ifx_LMU0  MODULE_LMU0  = {0};
Ifx_MINIMCDS MODULE_MINIMCDS = {0};
Ifx_MSC0  MODULE_MSC0  = {0};
Ifx_MSC1  MODULE_MSC1  = {0};
Ifx_MSC2  MODULE_MSC2  = {0};
Ifx_MTU   MODULE_MTU   = {0};
Ifx_PFI0  MODULE_PFI0  = {0};
Ifx_PFI1  MODULE_PFI1  = {0};
Ifx_PFI2  MODULE_PFI2  = {0};
Ifx_PFI3  MODULE_PFI3  = {0};
Ifx_PMS   MODULE_PMS   = {0};
Ifx_PMU   MODULE_PMU   = {0};
Ifx_PSI5S MODULE_PSI5S = {0};
Ifx_PSI5  MODULE_PSI5  = {0};
Ifx_QSPI0 MODULE_QSPI0 = {0};
Ifx_QSPI1 MODULE_QSPI1 = {0};
Ifx_QSPI2 MODULE_QSPI2 = {0};
Ifx_QSPI3 MODULE_QSPI3 = {0};
Ifx_QSPI4 MODULE_QSPI4 = {0};
Ifx_SBCU  MODULE_SBCU  = {0};
Ifx_SCU   MODULE_SCU   = {0};
Ifx_SENT  MODULE_SENT  = {0};
Ifx_SMU   MODULE_SMU   = {0};
Ifx_SRC   MODULE_SRC   = {0};
Ifx_STM0  MODULE_STM0  = {0};
Ifx_STM1  MODULE_STM1  = {0};
Ifx_STM2  MODULE_STM2  = {0};
Ifx_STM3  MODULE_STM3  = {0};

/* Stub bodies */
void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    mock_IfxXspi_Spi_transferInit_callCount++;
    mock_IfxXspi_Spi_transferInit_lastXspi = (void*)xspi;
    mock_IfxXspi_Spi_transferInit_lastConfig = (void*)config;
    (void)xspi; (void)config;
}

boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig)
{
    mock_IfxXspi_Spi_exchange_callCount++;
    mock_IfxXspi_Spi_exchange_lastHandle = (void*)xspi;
    mock_IfxXspi_Spi_exchange_lastJobConfig = (void*)jobConfig;
    (void)xspi; (void)jobConfig;
    return mock_IfxXspi_Spi_exchange_returnValue;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    mock_IfxXspi_Spi_isrDmaReceive_callCount++;
    (void)xspi;
    return mock_IfxXspi_Spi_isrDmaReceive_returnValue;
}

void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins)
{
    mock_IfxXspi_Spi_setXspiGpioPins_callCount++;
    mock_IfxXspi_Spi_setXspiGpioPins_lastXspi = (void*)xspi;
    mock_IfxXspi_Spi_setXspiGpioPins_lastPins = (void*)pins;
    (void)xspi; (void)pins;
}

IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    mock_IfxXspi_Spi_initModule_callCount++;
    mock_IfxXspi_Spi_initModule_lastHandle = (void*)xspi;
    mock_IfxXspi_Spi_initModule_lastConfig = (void*)config;
    (void)xspi; (void)config;
    return (IfxXspi_Status)mock_IfxXspi_Spi_initModule_returnValue;
}

void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi)
{
    mock_IfxXspi_Spi_initModuleConfig_callCount++;
    mock_IfxXspi_Spi_initModuleConfig_lastConfig = (void*)config;
    mock_IfxXspi_Spi_initModuleConfig_lastXspi = (void*)xspi;
    (void)config; (void)xspi;
}

void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi)
{
    mock_IfxXspi_Spi_isrTransmit_callCount++;
    (void)xspi;
}

uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi)
{
    mock_IfxXspi_Spi_isrReceive_callCount++;
    (void)xspi;
    return mock_IfxXspi_Spi_isrReceive_returnValue;
}

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
    mock_IfxCpu_Irq_installInterruptHandler_lastIsr = isrFuncPointer;
    mock_IfxCpu_Irq_installInterruptHandler_lastPrio = serviceReqPrioNumber;
    (void)isrFuncPointer; (void)serviceReqPrioNumber;
}

IfxSrc_Tos IfxCpu_Irq_getTos(void)
{
    mock_IfxCpu_Irq_getTos_callCount++;
    return (IfxSrc_Tos)mock_IfxCpu_Irq_getTos_returnValue;
}

IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_getPinState_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex;
    (void)port; (void)pinIndex;
    return (IfxPort_State)mock_IfxPort_getPinState_returnValue;
}

void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{
    mock_IfxPort_setPinFunctionMode_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex; (void)mode;
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_setPinHigh_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex;
    (void)port; (void)pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_setPinLow_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex;
    (void)port; (void)pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    mock_IfxPort_setPinModeInput_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex; mock_IfxPort_setPinModeInput_lastMode = (uint32)mode;
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    mock_IfxPort_setPinModeOutput_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex;
    mock_IfxPort_setPinModeOutput_lastMode = (uint32)mode;
    mock_IfxPort_setPinModeOutput_lastIndex = (uint32)index;
    (void)port; (void)pinIndex; (void)mode; (void)index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    mock_IfxPort_setPinState_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex; mock_IfxPort_setPinState_lastAction = (uint32)action;
    (void)port; (void)pinIndex; (void)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex;
    (void)port; (void)pinIndex;
}

void IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_disableEmergencyStop_callCount++;
    (void)port; (void)pinIndex;
}

void IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    mock_IfxPort_enableEmergencyStop_callCount++;
    (void)port; (void)pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    mock_IfxPort_setPinMode_callCount++;
    mock_IfxPort_lastPort = (void*)port; mock_IfxPort_lastPinIndex = pinIndex; mock_IfxPort_setPinMode_lastMode = (uint32)mode;
    (void)port; (void)pinIndex; (void)mode;
}

/* Mock control API implementations */
void mock_qspi_reset(void)
{
    /* Zero counters */
    mock_IfxXspi_Spi_transferInit_callCount = 0;
    mock_IfxXspi_Spi_exchange_callCount = 0;
    mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;
    mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
    mock_IfxXspi_Spi_initModule_callCount = 0;
    mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
    mock_IfxXspi_Spi_isrTransmit_callCount = 0;
    mock_IfxXspi_Spi_isrReceive_callCount = 0;

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
    mock_IfxPort_disableEmergencyStop_callCount = 0;
    mock_IfxPort_enableEmergencyStop_callCount = 0;
    mock_IfxPort_setPinMode_callCount = 0;

    /* Zero return values */
    mock_IfxXspi_Spi_exchange_returnValue = FALSE;
    mock_IfxXspi_Spi_isrDmaReceive_returnValue = 0u;
    mock_IfxXspi_Spi_initModule_returnValue = 0u;
    mock_IfxXspi_Spi_isrReceive_returnValue = 0u;
    mock_IfxCpu_Irq_getTos_returnValue = (uint32)IfxSrc_Tos_cpu0;
    mock_IfxPort_getPinState_returnValue = 0u;

    /* Zero captures */
    mock_init_lastNumChannels = 0u;
    mock_init_lastFrequency = 0.0f;
    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_update_lastDuties[i] = 0.0f;
        mock_dt_lastDtRising[i] = 0.0f;
        mock_dt_lastDtFalling[i] = 0.0f;
    }
    mock_togglePin_callCount = 0u;

    mock_IfxXspi_Spi_transferInit_lastXspi = NULL_PTR;
    mock_IfxXspi_Spi_transferInit_lastConfig = NULL_PTR;
    mock_IfxXspi_Spi_exchange_lastHandle = NULL_PTR;
    mock_IfxXspi_Spi_exchange_lastJobConfig = NULL_PTR;
    mock_IfxXspi_Spi_setXspiGpioPins_lastXspi = NULL_PTR;
    mock_IfxXspi_Spi_setXspiGpioPins_lastPins = NULL_PTR;
    mock_IfxXspi_Spi_initModule_lastHandle = NULL_PTR;
    mock_IfxXspi_Spi_initModule_lastConfig = NULL_PTR;
    mock_IfxXspi_Spi_initModuleConfig_lastConfig = NULL_PTR;
    mock_IfxXspi_Spi_initModuleConfig_lastXspi = NULL_PTR;
    mock_IfxCpu_Irq_installInterruptHandler_lastIsr = NULL_PTR;
    mock_IfxCpu_Irq_installInterruptHandler_lastPrio = 0u;
    mock_IfxPort_lastPort = NULL_PTR;
    mock_IfxPort_lastPinIndex = 0u;
    mock_IfxPort_setPinState_lastAction = 0u;
    mock_IfxPort_setPinModeOutput_lastMode = 0u;
    mock_IfxPort_setPinModeOutput_lastIndex = 0u;
    mock_IfxPort_setPinModeInput_lastMode = 0u;
    mock_IfxPort_setPinMode_lastMode = 0u;
}

/* Accessors */
int  mock_IfxXspi_Spi_transferInit_getCallCount(void)      { return mock_IfxXspi_Spi_transferInit_callCount; }
int  mock_IfxXspi_Spi_exchange_getCallCount(void)          { return mock_IfxXspi_Spi_exchange_callCount; }
int  mock_IfxXspi_Spi_isrDmaReceive_getCallCount(void)     { return mock_IfxXspi_Spi_isrDmaReceive_callCount; }
int  mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(void)   { return mock_IfxXspi_Spi_setXspiGpioPins_callCount; }
int  mock_IfxXspi_Spi_initModule_getCallCount(void)        { return mock_IfxXspi_Spi_initModule_callCount; }
int  mock_IfxXspi_Spi_initModuleConfig_getCallCount(void)  { return mock_IfxXspi_Spi_initModuleConfig_callCount; }
int  mock_IfxXspi_Spi_isrTransmit_getCallCount(void)       { return mock_IfxXspi_Spi_isrTransmit_callCount; }
int  mock_IfxXspi_Spi_isrReceive_getCallCount(void)        { return mock_IfxXspi_Spi_isrReceive_callCount; }
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int  mock_IfxCpu_Irq_getTos_getCallCount(void)             { return mock_IfxCpu_Irq_getTos_callCount; }
int  mock_IfxPort_getPinState_getCallCount(void)           { return mock_IfxPort_getPinState_callCount; }
int  mock_IfxPort_setPinFunctionMode_getCallCount(void)    { return mock_IfxPort_setPinFunctionMode_callCount; }
int  mock_IfxPort_setPinHigh_getCallCount(void)            { return mock_IfxPort_setPinHigh_callCount; }
int  mock_IfxPort_setPinLow_getCallCount(void)             { return mock_IfxPort_setPinLow_callCount; }
int  mock_IfxPort_setPinModeInput_getCallCount(void)       { return mock_IfxPort_setPinModeInput_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void)      { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxPort_setPinState_getCallCount(void)           { return mock_IfxPort_setPinState_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void)             { return mock_IfxPort_togglePin_callCount; }
int  mock_IfxPort_disableEmergencyStop_getCallCount(void)  { return mock_IfxPort_disableEmergencyStop_callCount; }
int  mock_IfxPort_enableEmergencyStop_getCallCount(void)   { return mock_IfxPort_enableEmergencyStop_callCount; }
int  mock_IfxPort_setPinMode_getCallCount(void)            { return mock_IfxPort_setPinMode_callCount; }
