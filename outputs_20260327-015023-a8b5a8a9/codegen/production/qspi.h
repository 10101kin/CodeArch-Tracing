/*=======================
 * Qspi module - public API
 *=======================*/
#ifndef QSPI_H
#define QSPI_H

/* Public function prototypes only (no includes, typedefs, or macros here) */

/* Opaque context type is provided by integrator's abstraction layer */
struct QspiCommunication;
typedef struct QspiCommunication QspiCommunication;

/* Initialize XSPI for TLE9180 communication (TC4xx) */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication);

#endif /* QSPI_H */
