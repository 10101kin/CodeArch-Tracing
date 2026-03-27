/* IfxXspi_Spi.h - Driver-specific types + functions */
#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "mock_qspi.h"
#include "IfxPort.h"

/* Driver-specific enums */
typedef enum {
    IfxXspi_Status_ok = 0,
    IfxXspi_Status_error = 1,
    IfxXspi_Status_busy = 2
} IfxXspi_Status;

/* Forward typedefs */
typedef struct IfxXspi_Spi              IfxXspi_Spi;
typedef struct IfxXspi_Spi_Config       IfxXspi_Spi_Config;
typedef struct IfxXspi_Spi_GpioPins     IfxXspi_Spi_GpioPins;
typedef struct IfxXspi_Spi_initTransferConfig IfxXspi_Spi_initTransferConfig;
typedef struct IfxXspi_Spi_CpuJobConfig IfxXspi_Spi_CpuJobConfig;

/* Driver-specific structs */
struct IfxXspi_Spi {
    Ifx_XSPI   *xspi;      /* XSPI module base */
    uint32      isInitialized;
};

struct IfxXspi_Spi_GpioPins {
    IfxPort_Pin sclk;
    IfxPort_Pin mosi;
    IfxPort_Pin miso;
    IfxPort_Pin slso;
};

struct IfxXspi_Spi_Config {
    Ifx_XSPI              *xspi;
    IfxXspi_Spi_GpioPins   pins;
    uint32                 baudrate;
    boolean                isMaster;
    Ifx_Priority           txPriority;
    Ifx_Priority           rxPriority;
    IfxSrc_Tos             isrProvider;
};

struct IfxXspi_Spi_initTransferConfig {
    uint32  baudrate;
    uint32  dataLength;
    boolean msbFirst;
};

struct IfxXspi_Spi_CpuJobConfig {
    const uint8 *txData;
    uint8       *rxData;
    uint32       length;
    uint8        cs;
    boolean      endOfFrame;
};

/* Function declarations (exact signatures from DRIVERS TO MOCK) */
void    IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32  IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
void    IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void    IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
void    IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
uint32  IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);

#endif /* IFXXSPI_SPI_H */
