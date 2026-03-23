#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Required API signatures */
/* Mock control - call counts */
/* Mock control - return setters/getters */
/* Mock control - reset */

/* ============= Function Declarations ============= */
void    IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
void    IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32  IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);
void    IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
uint32  IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void    IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
uint32 IfxXspi_Spi_Mock_GetCallCount_transferInit(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_exchange(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrReceive(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrTransmit(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_initModule(void);
uint32 IfxXspi_Spi_Mock_GetCallCount_initModuleConfig(void);
void           IfxXspi_Spi_Mock_SetReturn_exchange(boolean value);
boolean        IfxXspi_Spi_Mock_GetReturn_exchange(void);
void           IfxXspi_Spi_Mock_SetReturn_isrReceive(uint32 value);
uint32         IfxXspi_Spi_Mock_GetReturn_isrReceive(void);
void           IfxXspi_Spi_Mock_SetReturn_isrDmaReceive(uint32 value);
uint32         IfxXspi_Spi_Mock_GetReturn_isrDmaReceive(void);
void           IfxXspi_Spi_Mock_SetReturn_initModule(IfxXspi_Status value);
IfxXspi_Status IfxXspi_Spi_Mock_GetReturn_initModule(void);
void IfxXspi_Spi_Mock_Reset(void);

#endif