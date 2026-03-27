#include "mock_qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* Spy counters and return values */
int mock_IfxXspi_Spi_transferInit_callCount = 0;
int mock_IfxXspi_Spi_exchange_callCount = 0;
boolean mock_IfxXspi_Spi_exchange_returnValue = FALSE;
int mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;
int mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
int mock_IfxXspi_Spi_initModule_callCount = 0;
int mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
int mock_IfxXspi_Spi_isrTransmit_callCount = 0;
int mock_IfxXspi_Spi_isrReceive_callCount = 0;
int mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
int mock_IfxPort_setPinState_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;
int mock_IfxPort_setPinModeInput_callCount = 0;

/* Generic PWM-oriented spies */
uint32  mock_initEgtmAtom_lastNumChannels = 0;
float32 mock_initEgtmAtom_lastFrequency = 0.0f;
float32 mock_updateEgtmAtom_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_dtEgtmAtom_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_dtEgtmAtom_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0;

/* MODULE_* instance definitions */
Ifx_XSPI MODULE_XSPI = {0};
Ifx_CPU  MODULE_CPU  = {0};
Ifx_P MODULE_P00 = {0}; Ifx_P MODULE_P01 = {0}; Ifx_P MODULE_P02 = {0}; Ifx_P MODULE_P10 = {0}; Ifx_P MODULE_P11 = {0}; Ifx_P MODULE_P12 = {0}; Ifx_P MODULE_P13 = {0}; Ifx_P MODULE_P14 = {0}; Ifx_P MODULE_P15 = {0}; Ifx_P MODULE_P20 = {0}; Ifx_P MODULE_P21 = {0}; Ifx_P MODULE_P22 = {0}; Ifx_P MODULE_P23 = {0}; Ifx_P MODULE_P24 = {0}; Ifx_P MODULE_P25 = {0}; Ifx_P MODULE_P26 = {0}; Ifx_P MODULE_P30 = {0}; Ifx_P MODULE_P31 = {0}; Ifx_P MODULE_P32 = {0}; Ifx_P MODULE_P33 = {0}; Ifx_P MODULE_P34 = {0}; Ifx_P MODULE_P40 = {0}; Ifx_P MODULE_P41 = {0};
Ifx_ASCLIN0 MODULE_ASCLIN0 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN1 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN2 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN3 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN4 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN5 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN6 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN7 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN8 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN9 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN10 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN11 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN12 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN13 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN14 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN15 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN16 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN17 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN18 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN19 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN20 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN21 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN22 = {0}; Ifx_ASCLIN0 MODULE_ASCLIN23 = {0};
Ifx_CAN0 MODULE_CAN0 = {0}; Ifx_CAN0 MODULE_CAN1 = {0}; Ifx_CAN0 MODULE_CAN2 = {0};
Ifx_CBS  MODULE_CBS  = {0};
Ifx_CCU60 MODULE_CCU60 = {0}; Ifx_CCU60 MODULE_CCU61 = {0};
Ifx_CONVCTRL MODULE_CONVCTRL = {0};
Ifx_CPU0 MODULE_CPU0 = {0}; Ifx_CPU0 MODULE_CPU1 = {0}; Ifx_CPU0 MODULE_CPU2 = {0}; Ifx_CPU0 MODULE_CPU3 = {0};
Ifx_DAM0 MODULE_DAM0 = {0};
Ifx_DMA  MODULE_DMA  = {0};
Ifx_DMU  MODULE_DMU  = {0};
Ifx_DOM0 MODULE_DOM0 = {0};
Ifx_EDSADC MODULE_EDSADC = {0};
Ifx_ERAY0 MODULE_ERAY0 = {0}; Ifx_ERAY0 MODULE_ERAY1 = {0};
Ifx_EVADC MODULE_EVADC = {0};
Ifx_FCE  MODULE_FCE  = {0};
Ifx_FSI  MODULE_FSI  = {0};
Ifx_GETH MODULE_GETH = {0};
Ifx_GPT120 MODULE_GPT120 = {0};
Ifx_GTM  MODULE_GTM  = {0};
Ifx_HSCT0 MODULE_HSCT0 = {0};
Ifx_HSSL0 MODULE_HSSL0 = {0};
Ifx_I2C0 MODULE_I2C0 = {0}; Ifx_I2C0 MODULE_I2C1 = {0};
Ifx_INT  MODULE_INT  = {0};
Ifx_IOM  MODULE_IOM  = {0};
Ifx_LMU0 MODULE_LMU0 = {0};
Ifx_MINIMCDS MODULE_MINIMCDS = {0};
Ifx_MSC0 MODULE_MSC0 = {0}; Ifx_MSC0 MODULE_MSC1 = {0}; Ifx_MSC0 MODULE_MSC2 = {0};
Ifx_MTU  MODULE_MTU  = {0};
Ifx_PFI0 MODULE_PFI0 = {0}; Ifx_PFI0 MODULE_PFI1 = {0}; Ifx_PFI0 MODULE_PFI2 = {0}; Ifx_PFI0 MODULE_PFI3 = {0};
Ifx_PMS  MODULE_PMS  = {0};
Ifx_PMU  MODULE_PMU  = {0};
Ifx_PSI5S MODULE_PSI5S = {0};
Ifx_PSI5 MODULE_PSI5 = {0};
Ifx_QSPI0 MODULE_QSPI0 = {0}; Ifx_QSPI0 MODULE_QSPI1 = {0}; Ifx_QSPI0 MODULE_QSPI2 = {0}; Ifx_QSPI0 MODULE_QSPI3 = {0}; Ifx_QSPI0 MODULE_QSPI4 = {0};
Ifx_SBCU MODULE_SBCU = {0};
Ifx_SCU  MODULE_SCU  = {0};
Ifx_SENT MODULE_SENT = {0};
Ifx_SMU  MODULE_SMU  = {0};
Ifx_SRC  MODULE_SRC  = {0};
Ifx_STM0 MODULE_STM0 = {0}; Ifx_STM0 MODULE_STM1 = {0}; Ifx_STM0 MODULE_STM2 = {0}; Ifx_STM0 MODULE_STM3 = {0};

/* Stub bodies - IfxXspi_Spi */
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
    (void)xspi; (void)config; mock_IfxXspi_Spi_initModule_callCount++; return IfxXspi_Status_ok;
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

/* Stub bodies - IfxCpu_Irq */
void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber)
{
    (void)isrFuncPointer; (void)serviceReqPrioNumber; mock_IfxCpu_Irq_installInterruptHandler_callCount++;
}

IfxSrc_Tos IfxCpu_Irq_getTos(void)
{
    return IfxSrc_Tos_cpu0;
}

/* Stub bodies - IfxPort */
IfxPort_State IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex; return IfxPort_State_notChanged;
}

void IfxPort_setPinFunctionMode(Ifx_P *port, uint8 pinIndex, IfxPort_PinFunctionMode mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
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
    (void)port; (void)pinIndex; mock_togglePin_callCount++;
}

void IfxPort_disableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_enableEmergencyStop(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port; (void)pinIndex; (void)mode;
}

/* Mock control */
void mock_qspi_reset(void)
{
    int i;
    mock_IfxXspi_Spi_transferInit_callCount = 0;
    mock_IfxXspi_Spi_exchange_callCount = 0;
    mock_IfxXspi_Spi_exchange_returnValue = FALSE;
    mock_IfxXspi_Spi_isrDmaReceive_callCount = 0;
    mock_IfxXspi_Spi_setXspiGpioPins_callCount = 0;
    mock_IfxXspi_Spi_initModule_callCount = 0;
    mock_IfxXspi_Spi_initModuleConfig_callCount = 0;
    mock_IfxXspi_Spi_isrTransmit_callCount = 0;
    mock_IfxXspi_Spi_isrReceive_callCount = 0;
    mock_IfxCpu_Irq_installInterruptHandler_callCount = 0;
    mock_IfxPort_setPinState_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxPort_setPinModeInput_callCount = 0;

    mock_initEgtmAtom_lastNumChannels = 0;
    mock_initEgtmAtom_lastFrequency = 0.0f;
    for (i = 0; i < MOCK_MAX_CHANNELS; ++i) {
        mock_updateEgtmAtom_lastDuties[i] = 0.0f;
        mock_dtEgtmAtom_lastDtRising[i] = 0.0f;
        mock_dtEgtmAtom_lastDtFalling[i] = 0.0f;
    }
    mock_togglePin_callCount = 0;
}

/* Call count getters */
int mock_IfxXspi_Spi_transferInit_getCallCount(void) { return mock_IfxXspi_Spi_transferInit_callCount; }
int mock_IfxXspi_Spi_exchange_getCallCount(void) { return mock_IfxXspi_Spi_exchange_callCount; }
int mock_IfxXspi_Spi_isrDmaReceive_getCallCount(void) { return mock_IfxXspi_Spi_isrDmaReceive_callCount; }
int mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(void) { return mock_IfxXspi_Spi_setXspiGpioPins_callCount; }
int mock_IfxXspi_Spi_initModule_getCallCount(void) { return mock_IfxXspi_Spi_initModule_callCount; }
int mock_IfxXspi_Spi_initModuleConfig_getCallCount(void) { return mock_IfxXspi_Spi_initModuleConfig_callCount; }
int mock_IfxXspi_Spi_isrTransmit_getCallCount(void) { return mock_IfxXspi_Spi_isrTransmit_callCount; }
int mock_IfxXspi_Spi_isrReceive_getCallCount(void) { return mock_IfxXspi_Spi_isrReceive_callCount; }
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void) { return mock_IfxCpu_Irq_installInterruptHandler_callCount; }
int mock_IfxPort_setPinState_getCallCount(void) { return mock_IfxPort_setPinState_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }
int mock_IfxPort_setPinModeInput_getCallCount(void) { return mock_IfxPort_setPinModeInput_callCount; }
