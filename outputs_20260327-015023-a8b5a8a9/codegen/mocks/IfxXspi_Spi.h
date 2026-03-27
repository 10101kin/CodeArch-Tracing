#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "mock_qspi.h"
#include "IfxPort.h"

/* Inferred XSPI SPI driver types */
typedef enum
{
    IfxXspi_Status_ok = 0,
    IfxXspi_Status_notInitialized = 1,
    IfxXspi_Status_error = 2
} IfxXspi_Status;

typedef struct
{
    Ifx_XSPI *xspi;          /* associated module */
    uint32    isInitialized;  /* internal state */
} IfxXspi_Spi;

typedef struct
{
    uint32 baudrate;
    uint32 modeFlags; /* CPOL/CPHA, etc. */
} IfxXspi_Spi_initTransferConfig;

typedef struct
{
    const void *txBuffer;
    void       *rxBuffer;
    uint32      dataLength; /* in bytes */
} IfxXspi_Spi_CpuJobConfig;

typedef struct
{
    IfxPort_Pin sclk;
    IfxPort_Pin mosi;
    IfxPort_Pin miso;
    IfxPort_Pin cs;
} IfxXspi_Spi_GpioPins;

typedef struct
{
    Ifx_XSPI     *xspi;
    IfxXspi_Spi_GpioPins *pins;
    Ifx_Priority  txPriority;
    Ifx_Priority  rxPriority;
    IfxSrc_Tos    tos;
    uint32        maxBaudrate;
} IfxXspi_Spi_Config;

/* API declarations (exact signatures from mapping) */
void    IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32  IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
void    IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void    IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
void    IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
uint32  IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);

#endif /* IFXXSPI_SPI_H */
