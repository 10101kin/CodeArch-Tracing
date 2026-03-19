#ifndef IFXSTM_H
#define IFXSTM_H

#include "illd_types/Ifx_Types.h"

/* Mock controls */
/* Set returns */
/* Captures */

/* ============= Function Declarations ============= */
uint64 IfxStm_getFrequency(void);
uint64 IfxStm_getTicksFromMicroseconds(uint32 microSeconds);
uint64 IfxStm_getTicksFromMilliseconds(uint32 milliSeconds);
void   IfxStm_disableComparatorInterrupt(Ifx_CPU *cpu, IfxStm_Comparator comparator);
void   IfxStm_enableComparatorInterrupt(Ifx_CPU *cpu, IfxStm_Comparator comparator);
uint32 IfxStm_Mock_GetCallCount_getFrequency(void);
uint32 IfxStm_Mock_GetCallCount_getTicksFromMicroseconds(void);
uint32 IfxStm_Mock_GetCallCount_getTicksFromMilliseconds(void);
uint32 IfxStm_Mock_GetCallCount_disableComparatorInterrupt(void);
uint32 IfxStm_Mock_GetCallCount_enableComparatorInterrupt(void);
void   IfxStm_Mock_Reset(void);
void   IfxStm_Mock_SetReturn_getFrequency(uint64 ret);
void   IfxStm_Mock_SetReturn_getTicksFromMicroseconds(uint64 ret);
void   IfxStm_Mock_SetReturn_getTicksFromMilliseconds(uint64 ret);
uint32 IfxStm_Mock_GetLastArg_getTicksFromMicroseconds_us(void);
uint32 IfxStm_Mock_GetLastArg_getTicksFromMilliseconds_ms(void);
uint32 IfxStm_Mock_GetLastArg_disableComparatorInterrupt_comparator(void);
uint32 IfxStm_Mock_GetLastArg_enableComparatorInterrupt_comparator(void);

#endif