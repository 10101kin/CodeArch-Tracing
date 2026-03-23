#ifndef IFXSTM_H
#define IFXSTM_H

#include "illd_types/Ifx_Types.h"  /* Provides Ifx_STM, uint32, Ifx_TickTime, etc. */

/* NOTE: Ifx_STM is already defined in illd_types/Ifx_Types.h - do NOT redefine */
/* Global STM instances - defined in ifx_types.c */
extern Ifx_STM MODULE_STM0;
/* Timer functions - 2026-02-20 Yvan: Added IfxStm_get and IfxStm_getFrequency */

/* ============= Function Declarations ============= */
uint64 IfxStm_get(Ifx_STM *stm);
float32 IfxStm_getFrequency(Ifx_STM *stm);
sint32 IfxStm_getTicksFromMilliseconds(Ifx_STM *stm, uint32 milliSeconds);
sint32 IfxStm_getTicksFromMicroseconds(Ifx_STM *stm, uint32 microSeconds);

#endif /* IFXSTM_H */