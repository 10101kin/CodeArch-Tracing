#include "IfxXspi_Spi.h"

/* Call counters */
static uint32 s_initModule_count       = 0u;
static uint32 s_transferInit_count     = 0u;
static uint32 s_isrTransmit_count      = 0u;
static uint32 s_setXspiGpioPins_count  = 0u;
static uint32 s_isrReceive_count       = 0u;
static uint32 s_initModuleConfig_count = 0u;
static uint32 s_exchange_count         = 0u;
static uint32 s_isrDmaReceive_count    = 0u;

/* Return values */
static IfxXspi_Status s_initModule_ret     = 0u;
static uint32         s_isrReceive_ret     = 0u;
static boolean        s_exchange_ret       = 0u;
static uint32         s_isrDmaReceive_ret  = 0u;

/* API implementations */
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config)
{
    (void)xspi;
    (void)config;
    s_initModule_count++;
    return s_initModule_ret;
}

void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config)
{
    (void)xspi;
    (void)config;
    s_transferInit_count++;
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
    (void)jobConfig;
    s_exchange_count++;
    return s_exchange_ret;
}

uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi)
{
    (void)xspi;
    s_isrDmaReceive_count++;
    return s_isrDmaReceive_ret;
}

/* Mock control: GetCallCount */
uint32 IfxXspi_Spi_Mock_GetCallCount_initModule(void)      { return s_initModule_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_transferInit(void)    { return s_transferInit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrTransmit(void)     { return s_isrTransmit_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins(void) { return s_setXspiGpioPins_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrReceive(void)      { return s_isrReceive_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_initModuleConfig(void){ return s_initModuleConfig_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_exchange(void)        { return s_exchange_count; }
uint32 IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive(void)   { return s_isrDmaReceive_count; }

/* Mock control: SetReturn */
void IfxXspi_Spi_Mock_SetReturn_initModule(IfxXspi_Status value) { s_initModule_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_isrReceive(uint32 value)         { s_isrReceive_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_exchange(boolean value)          { s_exchange_ret = value; }
void IfxXspi_Spi_Mock_SetReturn_isrDmaReceive(uint32 value)      { s_isrDmaReceive_ret = value; }

/* Reset all counters and return values */
void IfxXspi_Spi_Mock_Reset(void)
{
    s_initModule_count       = 0u;
    s_transferInit_count     = 0u;
    s_isrTransmit_count      = 0u;
    s_setXspiGpioPins_count  = 0u;
    s_isrReceive_count       = 0u;
    s_initModuleConfig_count = 0u;
    s_exchange_count         = 0u;
    s_isrDmaReceive_count    = 0u;

    s_initModule_ret    = 0u;
    s_isrReceive_ret    = 0u;
    s_exchange_ret      = 0u;
    s_isrDmaReceive_ret = 0u;
}
