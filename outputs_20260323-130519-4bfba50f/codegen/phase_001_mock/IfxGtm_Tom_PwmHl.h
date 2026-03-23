#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations (stubs) */
/* Mock control functions */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode(void);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMode(boolean value);
uint32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMode_mode(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetArgHistoryCount_setOnTime(void);
Ifx_TimerValue IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(uint32 index);
Ifx_TimerValue IfxGtm_Tom_PwmHl_Mock_GetArgHistory_setOnTime(uint32 callIdx, uint32 elemIdx);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value);
void   IfxGtm_Tom_PwmHl_Mock_Reset(void);

#endif /* IFXGTM_TOM_PWMHL_H */