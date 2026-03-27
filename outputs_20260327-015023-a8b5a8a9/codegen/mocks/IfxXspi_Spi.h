#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "mock_qspi.h"
#include "IfxPort.h"

/* Driver-specific enums and types */
typedef enum {
    IfxXspi_Status_ok = 0,
    IfxXspi_Status_busy,
    IfxXspi_Status_error
} IfxXspi_Status;

typedef struct {
    Ifx_XSPI *xspi;       /* associated XSPI module */
    uint32    isrPriority;
    uint32    dmaChannelTx;
    uint32    dmaChannelRx;
} IfxXspi_Spi;

typedef struct {
    Ifx_XSPI *xspi;       /* target module */
    uint32    maxBaudrate;
    uint8     dataWidth;  /* bits per frame */
    boolean   isMaster;   /* master=TRUE, slave=FALSE */
    boolean   msbFirst;   /* TRUE if MSB first */
} IfxXspi_Spi_Config;

typedef struct {
    IfxPort_Pin sclk;
    IfxPort_Pin mosi; /* MTSR */
    IfxPort_Pin miso; /* MRST */
    IfxPort_Pin cs;   /* SLSO */
} IfxXspi_Spi_GpioPins;

typedef struct {
    const uint8 *txData;  /* transmit buffer */
    uint8       *rxData;  /* receive buffer */
    uint32       length;  /* number of bytes */
    boolean      endOfFrame;
} IfxXspi_Spi_CpuJobConfig;

typedef struct {
    uint8   dataWidth;
    boolean msbFirst;
} IfxXspi_Spi_initTransferConfig;

/* API declarations (exact signatures from spec) */
void    IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32  IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
void    IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void    IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
void    IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
uint32  IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);

#endif /* IFXXSPI_SPI_H */
