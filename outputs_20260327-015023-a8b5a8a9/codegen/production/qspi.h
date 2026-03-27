/*
 * qspi.h
 *
 * Public interface for the XSPI-based QSPI migration driver (TC4xx).
 *
 * NOTE: Header must contain prototypes only as per project rules.
 */
#ifndef QSPI_H
#define QSPI_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward declaration of communication context type used by the API */
typedef struct QspiCommunication QspiCommunication;

/*
 * Initialize the SPI master using the XSPI driver and bind it to the
 * TLE9180 abstraction. See qspi.c for the detailed behavior.
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_H */
