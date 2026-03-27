/*
 * qspi.h
 * Public API for XSPI-based SPI communication with TLE9180 on AURIX TC4xx.
 *
 * Note: This header intentionally contains only the function prototype as required.
 */
#ifndef QSPI_H
#define QSPI_H

/* Forward declaration of the communication context */
typedef struct QspiCommunication QspiCommunication;

/* External types come from Ifx_Types.h in the build system */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#endif /* QSPI_H */
