#ifndef IFXSTM_H
#define IFXSTM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for float32, sint32, uint32 */

/* Mock controls */

/* ============= Function Declarations ============= */
float32 IfxStm_getFrequency(Ifx_STM *stm);
sint32  IfxStm_getTicksFromMicroseconds(Ifx_STM *stm, uint32 microSeconds);
sint32  IfxStm_getTicksFromMilliseconds(Ifx_STM *stm, uint32 milliSeconds);
void    IfxStm_disableComparatorInterrupt(Ifx_STM *stm, IfxStm_Comparator comparator);
void    IfxStm_enableComparatorInterrupt(Ifx_STM *stm, IfxStm_Comparator comparator);
void    IfxStm_Mock_SetReturn_getFrequency(float32 ret);
uint32  IfxStm_Mock_GetCallCount_getFrequency(void);
void    IfxStm_Mock_SetReturn_getTicksFromMicroseconds(sint32 ret);
uint32  IfxStm_Mock_GetCallCount_getTicksFromMicroseconds(void);
uint32  IfxStm_Mock_GetLastArg_getTicksFromMicroseconds_us(void);
void    IfxStm_Mock_SetReturn_getTicksFromMilliseconds(sint32 ret);
uint32  IfxStm_Mock_GetCallCount_getTicksFromMilliseconds(void);
uint32  IfxStm_Mock_GetLastArg_getTicksFromMilliseconds_ms(void);
uint32  IfxStm_Mock_GetCallCount_disableComparatorInterrupt(void);
uint32  IfxStm_Mock_GetLastArg_disableComparatorInterrupt_comparator(void);
uint32  IfxStm_Mock_GetCallCount_enableComparatorInterrupt(void);
uint32  IfxStm_Mock_GetLastArg_enableComparatorInterrupt_comparator(void);
void    IfxStm_Mock_Reset(void);

#endif /* IFXSTM_H */