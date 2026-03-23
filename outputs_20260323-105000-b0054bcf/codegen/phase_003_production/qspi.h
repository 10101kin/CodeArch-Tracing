#ifndef QSPI_H
#define QSPI_H

#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Configuration values from requirements (MUST match exactly)
 */
#define SPI_MODULE_INSTANCE                  XSPI0
#define SPI_CHIP_SELECT_MODE                 channelBased
#define SPI_CHIP_SELECT_POLARITY             activeLow
#define SPI_BIT_ORDER                        msbFirst
#define SPI_FRAME_LENGTH_BITS                (8u)
#define SPI_DMA_ENABLED                      (0u)
#define SPI_ISR_PROVIDER                     IfxSrc_Tos_cpu0
#define TIMING_MAXIMUM_BAUDRATE_HZ           (50000000u)
#define TIMING_SPI_MODE                      (0u) /* Mode 0 */
#define TIMING_CS_HOLD_TIME_NS               (0u)
#define TIMING_CS_LEAD_TIME_NS               (0u)
#define TIMING_INTER_FRAME_DELAY_NS          (0u)
#define CLOCK_REQUIRES_XTAL                  (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ       (300u)

/* ISR priorities from reference pattern (names must match exactly) */
#define INTERRUPT_PRIORITY_QSPI3_TX          (40u)
#define INTERRUPT_PRIORITY_QSPI3_RX          (41u)
#define INTERRUPT_PRIORITY_QSPI3_ERR         (42u)

/* TLE9180 control signal pins from requirements */
#define TLE9180_ENABLE_PORT                  (&MODULE_P33)
#define TLE9180_ENABLE_PIN_INDEX             (0u)      /* P33.0 */
#define TLE9180_ENABLE_ACTIVE_HIGH           (1u)

#define TLE9180_NINT_PORT                    (&MODULE_P33)
#define TLE9180_NINT_PIN_INDEX               (1u)      /* P33.1 */

/* TLE9180 buffer size requirement */
#define TLE9180_WORKING_BUFFER_SIZE_BYTES    (8192u)

/* XSPI module instance pointer (from requirements) */
#define XSPI_MODULE_PTR                      (&MODULE_XSPI0)

/* Forward declaration of XSPI job config type (from driver header) */
struct IfxXspi_Spi_CpuJobConfig;

/*
 * Driver/application context for QSPI->XSPI migration
 */
typedef struct
{
    IfxXspi_Spi                         xspi;           /* XSPI driver handle */
    Ifx_XSPI                           *module;         /* XSPI module base */
    IfxXspi_Spi_initTransferConfig      transferCfg;    /* Transfer configuration */
    uint32                               tle9180Size;    /* Working buffer size requirement */
    boolean (*spiExchange)(IfxXspi_Spi *xspi, struct IfxXspi_Spi_CpuJobConfig *jobConfig); /* Exchange callback for higher layer */
    boolean                              initialized;    /* Initialization state */
} QspiCommunication;

/* Public API from SW Detailed Design (EXACT signature) */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
