#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Mock control functions */

/* ============= Function Declarations ============= */
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_addToChannelMask_channel(void);
void    IfxGtm_Tom_Timer_Mock_Reset(void);

#endif /* IFXGTM_TOM_TIMER_H */