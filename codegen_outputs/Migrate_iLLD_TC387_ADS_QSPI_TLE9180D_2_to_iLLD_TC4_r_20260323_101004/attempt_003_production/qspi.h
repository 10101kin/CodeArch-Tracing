/**
 * @file qspi.h
 * @brief XSPI-based SPI communication driver for TLE9180 on TC4xx.
 *
 * Production-ready implementation following SW Detailed Design and iLLD unified driver pattern.
 *
 * - Family: TC4xx
 * - Board: TC387 (migration target APIs compatible with TC4D7 XSPI)
 * - Peripheral: XSPI (replacing QSPI)
 *
 * DO NOT place any watchdog disable code here. Watchdog control is only allowed in CpuN_Main.c files.
 */
#ifndef QSPI_H
#define QSPI_H

#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Requirements-derived configuration values (MUST MATCH REQUIREMENTS)        */
/* ========================================================================== */
#define SPI_MODULE_INSTANCE_XSPI0              (1u)            /* Logical flag for XSPI0 instance */
#define SPI_MAXIMUM_BAUDRATE_HZ                (50000000u)     /* 50 MHz */
#define SPI_FRAME_LENGTH_BITS                  (8u)
#define SPI_DMA_ENABLED                        (0u)

/* ISR priorities (preserved names from reference pattern) */
#define INTERRUPT_PRIORITY_QSPI3_TX            (40u)
#define INTERRUPT_PRIORITY_QSPI3_RX            (41u)
#define INTERRUPT_PRIORITY_QSPI3_ERR           (42u)

/* TLE9180 control pins from requirements */
#define TLE9180_ENABLE_PORT                    (&MODULE_P33)
#define TLE9180_ENABLE_PIN                     (0u)      /* P33.0 */
#define TLE9180_NINT_PORT                      (&MODULE_P33)
#define TLE9180_NINT_PIN                       (1u)      /* P33.1 */

/* TLE9180 buffer size requirement */
#define TLE9180_WORKING_BUFFER_SIZE_BYTES      (8192u)

/* XSPI module instance selection (requirements: XSPI0) */
#ifndef XSPI_MODULE
#define XSPI_MODULE                             (&MODULE_XSPI0)
#endif

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */
/**
 * @brief XSPI exchange function pointer type (CPU job mode)
 */
typedef boolean (*Qspi_XspiExchangeFnc)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

/**
 * @brief SPI communication context, owned by application layer.
 */
typedef struct
{
    IfxXspi_Spi                 xspi;            /* XSPI driver handle */
    Ifx_XSPI                   *module;          /* XSPI SFR base used for init */
    Qspi_XspiExchangeFnc        exchangeFnc;     /* Linked to IfxXspi_Spi_exchange */
    uint32                      tleBufferSize;   /* 8192 (requirements) */
    boolean                     initialized;     /* TRUE after successful init */
} QspiCommunication;

/* ========================================================================== */
/* API                                                                        */
/* ========================================================================== */
/**
 * @brief Initialize XSPI master for TLE9180 according to SW Detailed Design.
 *
 * Algorithm summary:
 *  1) Load default XSPI module configuration for XSPI0.
 *  2) Assign SPI I/O pins (clock/MOSI/MISO/CS) via XSPI pin structure.
 *  3) Set operating parameters: 50 MHz, SPI mode 0, MSB-first, 8-bit frames, active-low CS, channel-based CS.
 *  4) Configure interrupts to IfxSrc_Tos_cpu0 with priorities TX/RX/ERR = 40/41/42.
 *  5) Initialize XSPI module and store handle in communication context.
 *  6) Initialize device control GPIOs: ENABLE as push-pull output (inactive low initially), NINT as input pull-up.
 *  7) Prepare linkage for TLE9180 abstraction: buffer size = 8192 and SPI exchange callback = IfxXspi_Spi_exchange.
 *
 * @param qspiCommunication Pointer to application-owned context (must be non-NULL)
 * @return Ifx_Status_ok on success, Ifx_Status_notOk on failure
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
