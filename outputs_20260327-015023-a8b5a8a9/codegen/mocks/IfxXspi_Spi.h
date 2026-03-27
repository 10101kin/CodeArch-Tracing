#ifndef IFXXSPI_SPI_H
#define IFXXSPI_SPI_H

#include "mock_qspi.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IfxXspi_Status_ok = 0,
    IfxXspi_Status_notOk = 1,
    IfxXspi_Status_busy = 2
} IfxXspi_Status;

typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
} IfxXspi_Spi_Pin;

typedef struct {
    IfxPort_Pin sclk;
    IfxPort_Pin mtsr;
    IfxPort_Pin mrst;
    IfxPort_Pin slso;
} IfxXspi_Spi_GpioPins;

typedef struct IfxXspi_Spi_Config_s {
    Ifx_XSPI            *xspi;
    uint32               baudrate;
    uint8                dataWidth;
    boolean              cpol;
    boolean              cpha;
    Ifx_ActiveState      csPolarity;
    Ifx_Priority         txPriority;
    Ifx_Priority         rxPriority;
    Ifx_Priority         erPriority;
    IfxSrc_Tos           isrProvider;
    IfxXspi_Spi_GpioPins pins;
} IfxXspi_Spi_Config;

typedef struct IfxXspi_Spi_s {
    Ifx_XSPI                  *xspi;
    const IfxXspi_Spi_Config  *config;
    uint8                      state;
    void                      *tx;
    void                      *rx;
    uint32                     length;
    boolean                    transferInProgress;
} IfxXspi_Spi;

typedef struct {
    void   *tx;
    void   *rx;
    uint32  length;
    boolean deassertSsel;
} IfxXspi_Spi_initTransferConfig;

typedef struct {
    void   *tx;
    void   *rx;
    uint32  length;
    boolean deassertSsel;
} IfxXspi_Spi_CpuJobConfig;

/* Driver API */
void    IfxXspi_Spi_transferInit(Ifx_XSPI *xspi, IfxXspi_Spi_initTransferConfig *config);
boolean IfxXspi_Spi_exchange(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
uint32  IfxXspi_Spi_isrDmaReceive(IfxXspi_Spi *xspi);
void    IfxXspi_Spi_setXspiGpioPins(Ifx_XSPI *xspi, IfxXspi_Spi_GpioPins *pins);
IfxXspi_Status IfxXspi_Spi_initModule(IfxXspi_Spi *xspi, const IfxXspi_Spi_Config *config);
void    IfxXspi_Spi_initModuleConfig(IfxXspi_Spi_Config *config, Ifx_XSPI *xspi);
void    IfxXspi_Spi_isrTransmit(IfxXspi_Spi *xspi);
uint32  IfxXspi_Spi_isrReceive(IfxXspi_Spi *xspi);

#ifdef __cplusplus
}
#endif

#endif /* IFXXSPI_SPI_H */
