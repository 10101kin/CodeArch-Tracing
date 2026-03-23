#include "IfxXspi_Spi.h"

/* Call counters */
static uint32 s_transferInit_count = 0;
static uint32 s_setXspiGpioPins_count = 0;
static uint32 s_exchange_count = 0;
static uint32 s_isrReceive_count = 0;
static uint32 s_isrTransmit_count = 0;
static uint32 s_isrDmaReceive_count = 0;
static uint32 s_initModule_count = 0;
static uint32 s_initModuleConfig_count = 0;

/* Return controls */
static boolean       s_exchange_ret = FALSE;
static uint32        s_isrReceive_ret = 0u;
static uint32        s_isrDmaReceive_ret = 0u;
static IfxXspi_Status s_initModule_ret = IfxXspi_Status_ok;

/* Captured config values (Pattern D) */
static uint32 s_transferInit_lastBaudrate = 0u;
static uint32 s_initModule_lastBaudrate = 0u;
static uint32 s_initModule_lastMode = 0u;
static uint32 s_initModule_lastEnableLoopback = 0u; /* cast of boolean */

void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    (void)xspi;
    s_transferInit_count++;
    /* Pattern D: capture key scalar fields from config if present */
    if (config != NULL_PTR) {
        s_transferInit_lastBaudrate = config->baudrate;
    }
}

void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins)
{
    (void)xspi;
    (void)pins;
    s_setXspiGpioPins_count++;
}

boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig)
{
    (void)xspi;
    (void)jobConfig;
    s_exchange_count++;
    return s_exchange_ret;
}

uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrReceive_count++;
    return s_isrReceive_ret;
}

void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrTransmit_count++;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrDmaReceive_count++;
    return s_isrDmaReceive_ret;
}

IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    (void)xspi;
    s_initModule_count++;
    /* Pattern D: capture essential fields if provided */
    if (config != NULL_PTR) {
        s_initModule_lastBaudrate      = config->baudrate;
        s_initModule_lastMode          = config->mode;
        s_initModule_lastEnableLoopback = (uint32)config->enableLoopback;
    }
    return s_initModule_ret;
}

void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi)
{
    (void)config;
    (void)xspi;
    s_initModuleConfig_count++;
}

/* Mock controls implementations */
uint32 IfxXspi_Spi_Mock_GetCallCount_transferInit(void)     { return s_transferInit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins(void)  { return s_setXspiGpioPins_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_exchange(void)         { return s_exchange_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrReceive(void)       { return s_isrReceive_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrTransmit(void)      { return s_isrTransmit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive(void)    { return s_isrDmaReceive_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_initModule(void)       { return s_initModule_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_initModuleConfig(void) { return s_initModuleConfig_count; }

void        IfxXspi_Spi_Mock_SetReturn_exchange(boolean value)      { s_exchange_ret = value; }
boolean     IfxXspi_Spi_Mock_GetReturn_exchange(void)               { return s_exchange_ret; }
void        IfxXspi_Spi_Mock_SetReturn_isrReceive(uint32 value)     { s_isrReceive_ret = value; }
uint32      IfxXspi_Spi_Mock_GetReturn_isrReceive(void)             { return s_isrReceive_ret; }
void        IfxXspi_Spi_Mock_SetReturn_isrDmaReceive(uint32 value)  { s_isrDmaReceive_ret = value; }
uint32      IfxXspi_Spi_Mock_GetReturn_isrDmaReceive(void)          { return s_isrDmaReceive_ret; }
void        IfxXspi_Spi_Mock_SetReturn_initModule(IfxXspi_Status v) { s_initModule_ret = v; }
IfxXspi_Status IfxXspi_Spi_Mock_GetReturn_initModule(void)          { return s_initModule_ret; }

uint32 IfxXspi_Spi_Mock_GetLastArg_transferInit_baudrate(void)      { return s_transferInit_lastBaudrate; }
uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_baudrate(void)        { return s_initModule_lastBaudrate; }
uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_mode(void)            { return s_initModule_lastMode; }
uint32 IfxXspi_Spi_Mock_GetLastArg_initModule_enableLoopback(void)  { return s_initModule_lastEnableLoopback; }

void IfxXspi_Spi_Mock_Reset(void)
{
    s_transferInit_count = 0u;
    s_setXspiGpioPins_count = 0u;
    s_exchange_count = 0u;
    s_isrReceive_count = 0u;
    s_isrTransmit_count = 0u;
    s_isrDmaReceive_count = 0u;
    s_initModule_count = 0u;
    s_initModuleConfig_count = 0u;

    s_exchange_ret = FALSE;
    s_isrReceive_ret = 0u;
    s_isrDmaReceive_ret = 0u;
    s_initModule_ret = IfxXspi_Status_ok;

    s_transferInit_lastBaudrate = 0u;
    s_initModule_lastBaudrate = 0u;
    s_initModule_lastMode = 0u;
    s_initModule_lastEnableLoopback = 0u;
}
