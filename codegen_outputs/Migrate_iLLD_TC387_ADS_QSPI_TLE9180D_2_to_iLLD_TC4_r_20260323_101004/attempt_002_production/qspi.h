#ifndef QSPI_H
#define QSPI_H

/**
 * Qspi - XSPI-based SPI communication module for TLE9180 on TC4xx
 *
 * Production-ready driver using iLLD XSPI API. Implements the SW Detailed Design
 * API contract and preserves reference behavior with channel-based CS and
 * non-DMA exchanges. Interrupts are routed to CPU0 with priorities 40/41/42.
 */

#include "Ifx_Types.h"
#include "IfxPort.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= Requirements Macros (DO NOT RENAME) ========================= */
#define SPI_MODULE_INSTANCE                 (0u)                 /* XSPI0 selected by requirements */
#define SPI_CHIP_SELECT_MODE                channelBased         /* From requirements */
#define SPI_CHIP_SELECT_POLARITY            activeLow            /* From requirements */
#define SPI_BIT_ORDER                       msbFirst             /* From requirements */
#define SPI_FRAME_LENGTH_BITS               (8u)                 /* From requirements */
#define SPI_DMA_ENABLED                     (0u)                 /* False - From requirements */
#define SPI_ISR_PROVIDER                    IfxSrc_Tos_cpu0      /* From requirements */
#define TIMING_MAXIMUM_BAUDRATE_HZ          (50000000.0f)        /* 50 MHz From requirements */
#define TIMING_SPI_MODE                     (0u)                 /* Mode 0 From requirements */
#define TIMING_CS_HOLD_TIME_NS              (0u)                 /* From requirements */
#define TIMING_CS_LEAD_TIME_NS              (0u)                 /* From requirements */
#define TIMING_INTER_FRAME_DELAY_NS         (0u)                 /* From requirements */
#define CLOCK_REQUIRES_XTAL                 (1u)                 /* From requirements */
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ      (300u)               /* From requirements */

/* Control pins (From requirements) */
#define TLE9180_CONTROL_SIGNALS_ENABLE_PIN_PORT   (&MODULE_P33)
#define TLE9180_CONTROL_SIGNALS_ENABLE_PIN_INDEX  (0u)   /* P33.0 */
#define TLE9180_CONTROL_SIGNALS_NINT_PIN_PORT     (&MODULE_P33)
#define TLE9180_CONTROL_SIGNALS_NINT_PIN_INDEX    (1u)   /* P33.1 */

/* XSPI instance (From requirements: XSPI0) */
#define XSPI_MODULE_SFR                     (&MODULE_XSPI0)

/* Runtime parameter macros (EXACT names preserved) */
#define INTERRUPT_PRIORITY_QSPI3_TX         (40u)
#define INTERRUPT_PRIORITY_QSPI3_RX         (41u)
#define INTERRUPT_PRIORITY_QSPI3_ERR        (42u)

/* ========================= Public Types ========================= */

typedef boolean (*Qspi_ExchangeCb)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

/**
 * Communication context storing XSPI handle and TLE9180 binding.
 */
typedef struct
{
    /* XSPI driver handle and SFR base */
    IfxXspi_Spi   xspi;             /* XSPI driver handle */
    Ifx_XSPI     *xspiSfr;          /* XSPI SFR base (e.g., &MODULE_XSPI0) */

    /* TLE9180 abstraction binding */
    struct
    {
        uint32                 size;            /* Working buffer size (8192) */
        Ifx_P                 *enablePort;      /* Enable pin port */
        uint8                  enablePin;       /* Enable pin index */
        IfxPort_OutputMode     enablePadMode;   /* Pad/drive for enable */
        Ifx_P                 *nintPort;        /* nINT pin port */
        uint8                  nintPin;         /* nINT pin index */
        IfxPort_InputMode      nintInputMode;   /* Input mode for nINT */
        Qspi_ExchangeCb        spiExchange;     /* XSPI exchange callback */
    } tle9180;

    boolean initialized;           /* Set TRUE only after successful init */
} QspiCommunication;

/* ========================= Public API ========================= */

/**
 * Initialize XSPI (XSPI0) for communication with TLE9180 using the XSPI driver.
 * Algorithm (per SW Detailed Design):
 * 1) Create and load a default XSPI module configuration for XSPI0.
 * 2) Assign the SPI I/O pins (SCLK/MOSI/MISO/CS) using XSPI PinMap and provide
 *    directions/pad characteristics via the driver pins structure.
 * 3) Set operating parameters: 50 MHz, mode 0, MSB-first, 8-bit frame, active-low CS,
 *    channel-based CS, no DMA.
 * 4) Configure ISR routing to CPU0 with TX/RX/ERR priorities 40/41/42; provide ISR
 *    wrappers that call XSPI driver ISR handlers.
 * 5) Initialize the XSPI module and store handles in the provided context.
 * 6) Initialize TLE9180 control GPIOs: enable pin as push-pull output (inactive low
 *    per active-high requirement), nINT as input (pull-up).
 * 7) Bind TLE9180 abstraction: set size=8192, link spiExchange to IfxXspi_Spi_exchange,
 *    and pass initialized XSPI handle via context for runtime exchanges.
 *
 * Returns Ifx_Status_ok on success; Ifx_Status_notOk on failure.
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
