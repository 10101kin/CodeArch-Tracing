#include "IfxXspi_Spi.h"

/* Call counters */
static uint32 s_initModule_count = 0u;
static uint32 s_transferInit_count = 0u;
static uint32 s_isrTransmit_count = 0u;
static uint32 s_setXspiGpioPins_count = 0u;
static uint32 s_isrReceive_count = 0u;
static uint32 s_initModuleConfig_count = 0u;
static uint32 s_exchange_count = 0u;
static uint32 s_isrDmaReceive_count = 0u;

/* Return controls */
static IfxXspi_Status s_initModule_ret = IfxXspi_Status_ok; /* default status */
static uint32 s_isrReceive_ret = 0u;
static boolean s_exchange_ret = (boolean)0;
static uint32 s_isrDmaReceive_ret = 0u;

/* Captured config fields (Pattern D) */
static uint32 s_initModule_last_baudrate = 0u;
static uint32 s_initModule_last_operatingMode = 0u;
static uint32 s_initModule_last_enableLoopback = 0u;

static uint32 s_transferInit_last_dataLength = 0u;
static uint32 s_transferInit_last_speed = 0u;

IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    (void)xspi;
    s_initModule_count++;
    /* Pattern D: capture fields from config if available */
    if (config != NULL_PTR) {
        s_initModule_last_baudrate = config->baudrate;
        s_initModule_last_operatingMode = config->operatingMode;
        s_initModule_last_enableLoopback = config->enableLoopback;
    }
    return s_initModule_ret;
}

void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    (void)xspi;
    s_transferInit_count++;
    /* Pattern D-like capture for initTransferConfig */
    if (config != NULL_PTR) {
        s_transferInit_last_dataLength = config->dataLength;
        s_transferInit_last_speed = config->speed;
    }
}

void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrTransmit_count++;
}

void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins)
{
    (void)xspi;
    (void)pins;
    s_setXspiGpioPins_count++;
}

uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrReceive_count++;
    return s_isrReceive_ret;
}

void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi)
{
    (void)config;
    (void)xspi;
    s_initModuleConfig_count++;
}

boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig)
{
    (void)xspi;
    (void)jobConfig; /* Do not dereference per mock rules */
    s_exchange_count++;
    return s_exchange_ret;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrDmaReceive_count++;
    return s_isrDmaReceive_ret;
}

/* Mock controls */
uint32 IfxXspi_Spi_Mock_GetCallCount_initModule(void) { return s_initModule_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_transferInit(void) { return s_transferInit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrTransmit(void) { return s_isrTransmit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins(void) { return s_setXspiGpioPins_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrReceive(void) { return s_isrReceive_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_initModuleConfig(void) { return s_initModuleConfig_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_exchange(void) { return s_exchange_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive(void) { return s_isrDmaReceive_count; }

void IfxXspi_Spi_Mock_SetReturn_initModule(IfxXspi_Status value) { s_initModule_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_isrReceive(uint32 value) { s_isrReceive_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_exchange(boolean value) { s_exchange_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_isrDmaReceive(uint32 value) { s_isrDmaReceive_ret = value; }

uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_baudrate(void) { return s_initModule_last_baudrate; }
uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_operatingMode(void) { return s_initModule_last_operatingMode; }
uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_enableLoopback(void) { return s_initModule_last_enableLoopback; }

uint32 IfxXspi_Spi_Mock_GetLastArg_transferInit_dataLength(void) { return s_transferInit_last_dataLength; }
uint32 IfxXspi_Spi_Mock_GetLastArg_transferInit_speed(void) { return s_transferInit_last_speed; }

void IfxXspi_Spi_Mock_Reset(void)
{
    s_initModule_count = 0u;
    s_transferInit_count = 0u;
    s_isrTransmit_count = 0u;
    s_setXspiGpioPins_count = 0u;
    s_isrReceive_count = 0u;
    s_initModuleConfig_count = 0u;
    s_exchange_count = 0u;
    s_isrDmaReceive_count = 0u;

    s_initModule_ret = IfxXspi_Status_ok;
    s_isrReceive_ret = 0u;
    s_exchange_ret = (boolean)0;
    s_isrDmaReceive_ret = 0u;

    s_initModule_last_baudrate = 0u;
    s_initModule_last_operatingMode = 0u;
    s_initModule_last_enableLoopback = 0u;

    s_transferInit_last_dataLength = 0u;
    s_transferInit_last_speed = 0u;
}
