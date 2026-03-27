/* mock_qspi.c - Spy state + stub bodies + MODULE_* definitions */
#include "mock_qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"
#include <string.h>

/* Spy counters and return values - definitions */
int mock_IfxXspi_Spi_transferInit_callCount = 0;
int mock_IfxXspi_Spi_exchange_callCount = 0;
int mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;
int mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
int mock_IfxXspi_Spi_initModule_callCount = 0;
int mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
int mock_IfxXspi_Spi_isrTransmit_callCount = 0;
int mock_IfxXspi_Spi_isrReceive_callCount = 0;

boolean mock_IfxXspi_Spi_exchange_returnValue = FALSE;
uint32  mock_IfxXspi_Spi_isrDmaReceive_returnValue = 0u;
uint32  mock_IfxXspi_Spi_isrReceive_returnValue = 0u;
uint32  mock_IfxXspi_Spi_initModule_returnValue_u32 = 0u; /* cast to IfxXspi_Status in stub */

int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int mock_IfxCpu_Irq_getTos_callCount = 0;
IfxSrc_Tos mock_IfxCpu_Irq_getTos_returnValue = IfxSrc_Tos_cpu0;

int mock_IfxPort_getPinState_callCount = 0;
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
uint32 mock_togglePin_callCount = 0u;
uint32 mock_IfxPort_getPinState_returnValue_u32 = 0u; /* cast to IfxPort_State */

uint32  mock_IfxXspi_Spi_initModule_lastNumChannels = 0u;
float32 mock_IfxXspi_Spi_initModule_lastFrequency = 0.0f;
float32 mock_IfxXspi_Spi_exchange_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxXspi_Spi_transferInit_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxXspi_Spi_transferInit_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* MODULE_* instance definitions */
Ifx_XSPI   MODULE_XSPI  = {0};
Ifx_CPU    MODULE_CPU   = {0};
Ifx_P      MODULE_P00   = {0};
Ifx_ASCLIN0  MODULE_ASCLIN0  = {0};
Ifx_ASCLIN1  MODULE_ASCLIN1  = {0};
Ifx_ASCLIN2  MODULE_ASCLIN2  = {0};
Ifx_ASCLIN3  MODULE_ASCLIN3  = {0};
Ifx_ASCLIN4  MODULE_ASCLIN4  = {0};
Ifx_ASCLIN5  MODULE_ASCLIN5  = {0};
Ifx_ASCLIN6  MODULE_ASCLIN6  = {0};
Ifx_ASCLIN7  MODULE_ASCLIN7  = {0};
Ifx_ASCLIN8  MODULE_ASCLIN8  = {0};
Ifx_ASCLIN9  MODULE_ASCLIN9  = {0};
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
Ifx_CAN0   MODULE_CAN0  = {0};
Ifx_CAN1   MODULE_CAN1  = {0};
Ifx_CAN2   MODULE_CAN2  = {0};
Ifx_CBS    MODULE_CBS   = {0};
Ifx_CCU60  MODULE_CCU60 = {0};
Ifx_CCU61  MODULE_CCU61 = {0};
Ifx_CONVCTRL MODULE_CONVCTRL = {0};
Ifx_CPU0   MODULE_CPU0  = {0};
Ifx_CPU1   MODULE_CPU1  = {0};
Ifx_CPU2   MODULE_CPU2  = {0};
Ifx_CPU3   MODULE_CPU3  = {0};
Ifx_DAM0   MODULE_DAM0  = {0};
Ifx_DMA    MODULE_DMA   = {0};
Ifx_DMU    MODULE_DMU   = {0};
Ifx_DOM0   MODULE_DOM0  = {0};
Ifx_EDSADC MODULE_EDSADC = {0};
Ifx_ERAY0  MODULE_ERAY0 = {0};
Ifx_ERAY1  MODULE_ERAY1 = {0};
Ifx_EVADC  MODULE_EVADC = {0};
Ifx_FCE    MODULE_FCE   = {0};
Ifx_FSI    MODULE_FSI   = {0};
Ifx_GETH   MODULE_GETH  = {0};
Ifx_GPT120 MODULE_GPT120 = {0};
Ifx_GTM    MODULE_GTM   = {0};
Ifx_HSCT0  MODULE_HSCT0 = {0};
Ifx_HSSL0  MODULE_HSSL0 = {0};
Ifx_I2C0   MODULE_I2C0  = {0};
Ifx_I2C1   MODULE_I2C1  = {0};
Ifx_INT    MODULE_INT   = {0};
Ifx_IOM    MODULE_IOM   = {0};
Ifx_LMU0   MODULE_LMU0  = {0};
Ifx_MINIMCDS MODULE_MINIMCDS = {0};
Ifx_MSC0   MODULE_MSC0  = {0};
Ifx_MSC1   MODULE_MSC1  = {0};
Ifx_MSC2   MODULE_MSC2  = {0};
Ifx_MTU    MODULE_MTU   = {0};
Ifx_PFI0   MODULE_PFI0  = {0};
Ifx_PFI1   MODULE_PFI1  = {0};
Ifx_PFI2   MODULE_PFI2  = {0};
Ifx_PFI3   MODULE_PFI3  = {0};
Ifx_PMS    MODULE_PMS   = {0};
Ifx_PMU    MODULE_PMU   = {0};
Ifx_P01    MODULE_P01   = {0};
Ifx_P02    MODULE_P02   = {0};
Ifx_P10    MODULE_P10   = {0};
Ifx_P11    MODULE_P11   = {0};
Ifx_P12    MODULE_P12   = {0};
Ifx_P13    MODULE_P13   = {0};
Ifx_P14    MODULE_P14   = {0};
Ifx_P15    MODULE_P15   = {0};
Ifx_P20    MODULE_P20   = {0};
Ifx_P21    MODULE_P21   = {0};
Ifx_P22    MODULE_P22   = {0};
Ifx_P23    MODULE_P23   = {0};
Ifx_P24    MODULE_P24   = {0};
Ifx_P25    MODULE_P25   = {0};
Ifx_P26    MODULE_P26   = {0};
Ifx_P30    MODULE_P30   = {0};
Ifx_P31    MODULE_P31   = {0};
Ifx_P32    MODULE_P32   = {0};
Ifx_P33    MODULE_P33   = {0};
Ifx_P34    MODULE_P34   = {0};
Ifx_P40    MODULE_P40   = {0};
Ifx_P41    MODULE_P41   = {0};
Ifx_PSI5S  MODULE_PSI5S = {0};
Ifx_PSI5   MODULE_PSI5  = {0};
Ifx_QSPI0  MODULE_QSPI0 = {0};
Ifx_QSPI1  MODULE_QSPI1 = {0};
Ifx_QSPI2  MODULE_QSPI2 = {0};
Ifx_QSPI3  MODULE_QSPI3 = {0};
Ifx_QSPI4  MODULE_QSPI4 = {0};
Ifx_SBCU   MODULE_SBCU  = {0};
Ifx_SCU    MODULE_SCU   = {0};
Ifx_SENT   MODULE_SENT  = {0};
Ifx_SMU    MODULE_SMU   = {0};
Ifx_SRC    MODULE_SRC   = {0};
Ifx_STM0   MODULE_STM0  = {0};
Ifx_STM1   MODULE_STM1  = {0};
Ifx_STM2   MODULE_STM2  = {0};
Ifx_STM3   MODULE_STM3  = {0};

/* Stub bodies */
void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    (void)xspi;
    (void)config;
    mock_IfxXspi_Spi_transferInit_callCount++;
    /* Capture representative values if provided */
    if (config) {
        mock_IfxXspi_Spi_transferInit_lastDtRising[0] = (float32)config->dataLength;
        mock_IfxXspi_Spi_transferInit_lastDtFalling[0] = (float32)config->baudrate;
    }
}

boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig)
{
    (void)xspi;
    mock_IfxXspi_Spi_exchange_callCount++;
    if (jobConfig) {
        mock_IfxXspi_Spi_exchange_lastDuties[0] = (float32)jobConfig->length;
    }
    return mock_IfxXspi_Spi_exchange_returnValue;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    mock_IfxXspi_Spi_isrDmaReceive_callCount++;
    return mock_IfxXspi_Spi_isrDmaReceive_returnValue;
}

void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins)
{
    (void)xspi;
    (void)pins;
    mock_IfxXspi_Spi_setXspiGpioPins_callCount++;
}

IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    (void)xspi;
    mock_IfxXspi_Spi_initModule_callCount++;
    if (config) {
        mock_IfxXspi_Spi_initModule_lastNumChannels = 1u;
        mock_IfxXspi_Spi_initModule_lastFrequency = (float32)config->baudrate;
    }
    return (IfxXspi_Status)mock_IfxXspi_Spi_initModule_returnValue_u32;
}

void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi)
{
    (void)config;
    (void)xspi;
    mock_IfxXspi_Spi_initModuleConfig_callCount++;
}

void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi)
{
    (void)xspi;
    mock_IfxXspi_Spi_isrTransmit_callCount++;
}

uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    mock_IfxXspi_Spi_isrReceive_callCount++;
    return mock_IfxXspi_Spi_isrReceive_returnValue;
}

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    (void)isrFuncPointer;
    (void)serviceReqPrioNumber;
    mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

IfxSrc_Tos IfxCpu_Irq_getTos(void)
{
    mock_IfxCpu_Irq_getTos_callCount++;
    return mock_IfxCpu_Irq_getTos_returnValue;
}

IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    (void)pinIndex;
    mock_IfxPort_getPinState_callCount++;
    return (IfxPort_State)mock_IfxPort_getPinState_returnValue_u32;
}

void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{
    (void)port; (void)pinIndex; (void)mode;
    mock_IfxPort_setPinFunctionMode_callCount++;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_setPinHigh_callCount++;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_setPinLow_callCount++;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    (void)port; (void)pinIndex; (void)mode;
    mock_IfxPort_setPinModeInput_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    (void)port; (void)pinIndex; (void)action;
    mock_IfxPort_setPinState_callCount++;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

void IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_disableEmergencyStop_callCount++;
}

void IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_enableEmergencyStop_callCount++;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port; (void)pinIndex; (void)mode;
    mock_IfxPort_setPinMode_callCount++;
}

/* Mock control implementations */
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
    mock_togglePin_callCount = 0u;

    /* Reset return values */
    mock_IfxXspi_Spi_exchange_returnValue = FALSE;
    mock_IfxXspi_Spi_isrDmaReceive_returnValue = 0u;
    mock_IfxXspi_Spi_isrReceive_returnValue = 0u;
    mock_IfxXspi_Spi_initModule_returnValue_u32 = 0u;
    mock_IfxCpu_Irq_getTos_returnValue = IfxSrc_Tos_cpu0;
    mock_IfxPort_getPinState_returnValue_u32 = 0u;

    /* Reset captures */
    mock_IfxXspi_Spi_initModule_lastNumChannels = 0u;
    mock_IfxXspi_Spi_initModule_lastFrequency = 0.0f;
    memset(mock_IfxXspi_Spi_exchange_lastDuties, 0, sizeof(mock_IfxXspi_Spi_exchange_lastDuties));
    memset(mock_IfxXspi_Spi_transferInit_lastDtRising, 0, sizeof(mock_IfxXspi_Spi_transferInit_lastDtRising));
    memset(mock_IfxXspi_Spi_transferInit_lastDtFalling, 0, sizeof(mock_IfxXspi_Spi_transferInit_lastDtFalling));
}

int  mock_IfxXspi_Spi_transferInit_getCallCount(void) { return mock_IfxXspi_Spi_transferInit_callCount; }
int  mock_IfxXspi_Spi_exchange_getCallCount(void) { return mock_IfxXspi_Spi_exchange_callCount; }
int  mock_IfxXspi_Spi_isrDmaReceive_getCallCount(void) { return mock_IfxXspi_Spi_isrDmaReceive_callCount; }
int  mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(void) { return mock_IfxXspi_Spi_setXspiGpioPins_callCount; }
int  mock_IfxXspi_Spi_initModule_getCallCount(void) { return mock_IfxXspi_Spi_initModule_callCount; }
int  mock_IfxXspi_Spi_initModuleConfig_getCallCount(void) { return mock_IfxXspi_Spi_initModuleConfig_callCount; }
int  mock_IfxXspi_Spi_isrTransmit_getCallCount(void) { return mock_IfxXspi_Spi_isrTransmit_callCount; }
int  mock_IfxXspi_Spi_isrReceive_getCallCount(void) { return mock_IfxXspi_Spi_isrReceive_callCount; }

int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int  mock_IfxCpu_Irq_getTos_getCallCount(void) { return mock_IfxCpu_Irq_getTos_callCount; }

int  mock_IfxPort_getPinState_getCallCount(void) { return mock_IfxPort_getPinState_callCount; }
int  mock_IfxPort_setPinFunctionMode_getCallCount(void) { return mock_IfxPort_setPinFunctionMode_callCount; }
int  mock_IfxPort_setPinHigh_getCallCount(void) { return mock_IfxPort_setPinHigh_callCount; }
int  mock_IfxPort_setPinLow_getCallCount(void) { return mock_IfxPort_setPinLow_callCount; }
int  mock_IfxPort_setPinModeInput_getCallCount(void) { return mock_IfxPort_setPinModeInput_callCount; }
int  mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int  mock_IfxPort_setPinState_getCallCount(void) { return mock_IfxPort_setPinState_callCount; }
int  mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int  mock_IfxPort_disableEmergencyStop_getCallCount(void) { return mock_IfxPort_disableEmergencyStop_callCount; }
int  mock_IfxPort_enableEmergencyStop_getCallCount(void) { return mock_IfxPort_enableEmergencyStop_callCount; }
int  mock_IfxPort_setPinMode_getCallCount(void) { return mock_IfxPort_setPinMode_callCount; }
