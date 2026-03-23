#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32, boolean, Ifx_XSPI */

/* ============= Type Definitions ============= */
typedef struct IfxXspi_Spi                IfxXspi_Spi;
typedef struct IfxXspi_Spi_Config         IfxXspi_Spi_Config;
typedef struct IfxXspi_Spi_initTransferConfig IfxXspi_Spi_initTransferConfig;
typedef struct IfxXspi_Spi_GpioPins       IfxXspi_Spi_GpioPins;
typedef struct IfxXspi_Spi_CpuJobConfig   IfxXspi_Spi_CpuJobConfig;
typedef uint32 IfxXspi_Status;

/* Opaque/placeholder types (pointers only) */
/* Status type placeholder (use integral type for mock) */
/* API declarations (exact signatures) */
/* Mock control: call counters */
/* Mock control: set return values for non-void APIs */
/* Reset all counters and return values */

/* ============= Function Declarations ============= */
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
void IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
void IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
uint32 IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);
void IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32 IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
uint32 IfxXspi_Spi_Mock_GetCallCount_initModule(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_transferInit(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrTransmit(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrReceive(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_initModuleConfig(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_exchange(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive(void);
void IfxXspi_Spi_Mock_SetReturn_initModule(IfxXspi_Status value);
void IfxXspi_Spi_Mock_SetReturn_isrReceive(uint32 value);
void IfxXspi_Spi_Mock_SetReturn_exchange(boolean value);
void IfxXspi_Spi_Mock_SetReturn_isrDmaReceive(uint32 value);
void IfxXspi_Spi_Mock_Reset(void);

#endif