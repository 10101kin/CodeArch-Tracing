#ifndef QSPI_H
#define QSPI_H

#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Requirements-derived configuration macros */
#define SPI_MODULE_INSTANCE               XSPI0
#define SPI_DMA_ENABLED                   (0)
#define SPI_CHIP_SELECT_MODE              channelBased
#define SPI_CHIP_SELECT_POLARITY          activeLow
#define SPI_BIT_ORDER                     msbFirst
#define SPI_FRAME_LENGTH_BITS             (8u)
#define TIMING_MAXIMUM_BAUDRATE_HZ        (50000000u)
#define TIMING_SPI_MODE                   (0u) /* CPOL=0, CPHA=0 */
#define TIMING_CS_HOLD_TIME_NS            (0u)
#define TIMING_CS_LEAD_TIME_NS            (0u)
#define TIMING_INTER_FRAME_DELAY_NS       (0u)
#define CLOCK_REQUIRES_XTAL               (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ    (300u)

/* ISR priorities - use exact names from reference */
#define INTERRUPT_PRIORITY_QSPI3_TX       (40)
#define INTERRUPT_PRIORITY_QSPI3_RX       (41)
#define INTERRUPT_PRIORITY_QSPI3_ERR      (42)

/* ISR provider (type-of-service) */
#define QSPI_ISR_PROVIDER                 IfxSrc_Tos_cpu0

/* XSPI module SFR pointer (default to XSPI0 as per requirements) */
#ifndef QSPI_XSPI_SFR
#define QSPI_XSPI_SFR                     (&MODULE_XSPI0)
#endif

/* TLE9180 control pin assignments (from requirements) */
#define TLE9180_ENABLE_PORT               (&MODULE_P33)
#define TLE9180_ENABLE_PIN                (0u)  /* P33.0 */
#define TLE9180_NINT_PORT                 (&MODULE_P33)
#define TLE9180_NINT_PIN                  (1u)  /* P33.1 */

/* TLE9180 buffer size (from requirements) */
#define TLE9180_SIZE_BYTES                (8192u)

/* Forward-declare XSPI CPU job config type (from IfxXspi_Spi.h) */
typedef boolean (*Qspi_SpiExchangeIf)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

/* Communication context exposed to integrator / higher layers */
typedef struct
{
    IfxXspi_Spi           xspi;                   /* XSPI driver handle */
    Ifx_XSPI             *xspiSfr;                /* XSPI SFR base used */
    Qspi_SpiExchangeIf    spiExchange;            /* Exchange function pointer */
    uint32                tle9180WorkBufferSize;  /* Working buffer size for TLE9180 */
} QspiCommunication;

/* Public API (from SW Detailed Design) */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
