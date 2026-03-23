#ifndef IFXSTM_TIMER_H
#define IFXSTM_TIMER_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
typedef struct IfxCcu6_Timer_Config IfxCcu6_Timer_Config;

/* Forward declaration for config type (opaque to the mock) */
/* Required function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxCcu6_Timer_initConfig(IfxCcu6_Timer_Config *config);
void IfxCcu6_PwmHl_init(void);
void IfxCcu6_Timer_init(void);
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_initConfig(void);
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_init(void);
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_init(void);
void IfxStm_Timer_Mock_Reset(void);

#endif /* IFXSTM_TIMER_H */