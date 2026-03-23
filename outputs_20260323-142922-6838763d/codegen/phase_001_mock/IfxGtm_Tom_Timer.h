#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32, boolean, float32, Ifx_GTM */

/* ============= Type Definitions ============= */
typedef struct IfxGtm_Tom_Timer IfxGtm_Tom_Timer;
typedef struct IfxGtm_Tom_Timer_Config IfxGtm_Tom_Timer_Config;

/* Opaque forward declarations for driver and config */
/* API Declarations */
/* Mock control: call counts */
/* Mock control: set returns for non-void */
/* Mock control: reset */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void);
void   IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
void IfxGtm_Tom_Timer_Mock_Reset(void);

#endif