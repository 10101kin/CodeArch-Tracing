#ifndef QSPI_H
#define QSPI_H

#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Runtime parameter macros (preserve exact names as required)
 */
#define INTERRUPT_PRIORITY_QSPI3_TX   (40)
#define INTERRUPT_PRIORITY_QSPI3_RX   (41)
#define INTERRUPT_PRIORITY_QSPI3_ERR  (42)

/* Requirement-derived configuration values */
#define QSPI_REQUIREMENTS_MAX_BAUDRATE_HZ      (50000000u)
#define QSPI_REQUIREMENTS_FRAME_LENGTH_BITS    (8u)
#define QSPI_REQUIREMENTS_DMA_ENABLED          (FALSE)
#define QSPI_REQUIREMENTS_ISR_PROVIDER         (IfxSrc_Tos_cpu0)
#define QSPI_REQUIREMENTS_MODE                 (0u) /* SPI mode 0 */
#define QSPI_REQUIREMENTS_CS_POLARITY_ACTIVE_LOW (1u)
#define QSPI_REQUIREMENTS_CS_HOLD_TIME_NS      (0u)
#define QSPI_REQUIREMENTS_CS_LEAD_TIME_NS      (0u)
#define QSPI_REQUIREMENTS_INTERFRAME_DELAY_NS  (0u)
#define QSPI_REQUIREMENTS_BIT_ORDER_MSB_FIRST  (1u)

/* XSPI module instance selection (from requirements) */
#define QSPI_XSPI_MODULE   (&MODULE_XSPI0)

/* TLE9180 control pins (from requirements) */
#define TLE9180_ENABLE_PORT   (&MODULE_P33)
#define TLE9180_ENABLE_PIN    ((uint8)0u)   /* P33.0 */
#define TLE9180_NINT_PORT     (&MODULE_P33)
#define TLE9180_NINT_PIN      ((uint8)1u)   /* P33.1 */

/* TLE9180 device configuration (from requirements) */
#define TLE9180_DEVICE_SIZE_BYTES   (8192u)

/* Forward-declared opaque structs from IfxXspi_Spi.h are used directly */

typedef boolean (*Qspi_SpiExchangeIf)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

/*
 * QSPI/XSPI communication context
 * Holds the XSPI handle and integration hooks for the higher-level device driver (TLE9180).
 */
typedef struct
{
    IfxXspi_Spi                     xspi;              /* XSPI driver handle */
    Ifx_XSPI                       *module;            /* XSPI module SFR base (e.g., &MODULE_XSPI0) */
    IfxXspi_Spi_CpuJobConfig        jobConfig;         /* Per-exchange job configuration (to be filled by caller) */

    /* Integration hooks for TLE9180 */
    uint32                          deviceSize;        /* Working buffer / device size (bytes) */
    Qspi_SpiExchangeIf              spiExchange;       /* Exchange function pointer (XSPI runtime) */

    /* Cached requirement parameters for diagnostics/consumers */
    uint32                          maximumBaudrate;   /* 50 MHz */
    uint8                           frameLengthBits;   /* 8 bits */
    IfxSrc_Tos                      isrProvider;       /* IfxSrc_Tos_cpu0 */
    uint8                           txPrio;            /* 40 */
    uint8                           rxPrio;            /* 41 */
    uint8                           errPrio;           /* 42 */
    boolean                         dmaEnabled;        /* FALSE */
} QspiCommunication;

/*
 * Initialize the SPI master on TC4xx using the XSPI driver and wire it to the TLE9180 abstraction.
 * Algorithm per SW Detailed Design:
 *  1) Load default XSPI module config for selected instance.
 *  2) Assign SCLK/MOSI/MISO/CS pins via XSPI PinMap driver.
 *  3) Set operating parameters (50 MHz, mode 0, MSB-first, 8-bit, active-low CS, channel-based CS, no DMA).
 *  4) Configure interrupt routing to CPU0 with TX/RX/ERR priorities 40/41/42 and install ISR wrappers.
 *  5) Initialize the XSPI module; store resulting handles into qspiCommunication.
 *  6) Initialize device control GPIOs: ENABLE (P33.0) output, drive inactive; NINT (P33.1) input pull-up.
 *  7) Prepare TLE9180 abstraction: size=8192, link spiExchange to IfxXspi_Spi_exchange, pass initialized handle.
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
