#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "illd_types/Ifx_Types.h"
#include "IfxGtm_Tom.h"

/* Mock control */

/* ============= Function Declarations ============= */
boolean      IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void         IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);
void         IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void         IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
void         IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
void         IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_updateInputFrequency(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_run(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_getPeriod(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_getPeriod(Ifx_TimerValue value);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void);
void    IfxGtm_Tom_Timer_Mock_Reset(void);

#endif