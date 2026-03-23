#ifndef QSPI_H
#define QSPI_H

/**
 * @file qspi.h
 * @brief XSPI-based SPI communication module for TLE9180 on TC4xx.
 *
 * Production-ready implementation that follows the unified iLLD driver pattern
 * and preserves reference behavior while replacing QSPI with XSPI.
 */

#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================= Requirements Macros ============================= */
#define INTERRUPT_PRIORITY_QSPI3_TX   (40u)
#define INTERRUPT_PRIORITY_QSPI3_RX   (41u)
#define INTERRUPT_PRIORITY_QSPI3_ERR  (42u)

#define SPI_MODULE_INSTANCE           XSPI0
#define SPI_CHIP_SELECT_MODE          channelBased
#define SPI_CHIP_SELECT_POLARITY      activeLow
#define SPI_BIT_ORDER                 msbFirst
#define SPI_FRAME_LENGTH_BITS         (8u)
#define SPI_DMA_ENABLED               (0u)
#define SPI_ISR_PROVIDER              IfxSrc_Tos_cpu0
#define TIMING_MAXIMUM_BAUDRATE_HZ    (50000000u)
#define TIMING_SPI_MODE               (0u)  /* Mode 0: CPOL=0, CPHA=0 */
#define TIMING_CS_HOLD_TIME_NS        (0u)
#define TIMING_CS_LEAD_TIME_NS        (0u)
#define TIMING_INTER_FRAME_DELAY_NS   (0u)
#define CLOCK_REQUIRES_XTAL           (1u)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ (300u)

/* TLE9180 control signals (from requirements) */
#define TLE9180_ENABLE_PORT           (&MODULE_P33)
#define TLE9180_ENABLE_PIN_IDX        (0u)   /* P33.0 */
#define TLE9180_ENABLE_ACTIVE_LEVEL   (IfxPort_State_high)

#define TLE9180_NINT_PORT             (&MODULE_P33)
#define TLE9180_NINT_PIN_IDX          (1u)   /* P33.1 */

/* ============================= Types and Handles ============================= */
/** Function pointer type for XSPI exchange used by the higher-level TLE9180 driver */
typedef boolean (*Qspi_ExchangeFct)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

/**
 * Communication context for the XSPI-based SPI master.
 * Holds the XSPI handle and linkage for the TLE9180 abstraction.
 */
typedef struct
{
    IfxXspi_Spi                    xspi;          /* XSPI driver handle */
    Qspi_ExchangeFct               exchange;      /* Exchange function callback (IfxXspi_Spi_exchange) */
    uint32                         tle9180Size;   /* Working buffer size for TLE9180 (bytes) */
    IfxXspi_Spi_initTransferConfig transferCfg;   /* Cached transfer config (frame length, speed, etc.) */
} QspiCommunication;

/* ============================= Public API ============================= */
/**
 * Initialize the SPI master using XSPI and wire it to the TLE9180 abstraction.
 *
 * Algorithm per SW Detailed Design:
 * 1) Load default XSPI module configuration for the selected module instance.
 * 2) Assign SPI I/O pins using XSPI PinMap and configure via driver pins structure.
 * 3) Set operating parameters: 50 MHz max, mode 0, MSB-first, 8-bit frame length,
 *    active-low CS, channel-based CS, DMA disabled.
 * 4) Configure interrupt routing to CPU0 with TX/RX/ERR priorities 40/41/42 and
 *    install ISR wrappers that call XSPI ISR handlers.
 * 5) Initialize the XSPI module and store handles in the communication context.
 * 6) Initialize target device control GPIOs: enable as push-pull output (inactive low),
 *    nINT as input with pull-up.
 * 7) Prepare TLE9180 linkage: set size=8192, exchange callback=IfxXspi_Spi_exchange.
 *
 * Returns Ifx_Status_ok on success, Ifx_Status_notOk otherwise.
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
