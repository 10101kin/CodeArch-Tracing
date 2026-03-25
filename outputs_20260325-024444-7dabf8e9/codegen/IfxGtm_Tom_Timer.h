#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "illd_types/Ifx_Types.h"
#include "IfxGtm_Tom.h"

/* ============= Type Definitions ============= */
struct IfxGtm_Tom_Timer { uint32 __mock; };
/* IfxGtm_Tom_Timer - defined in Ifx_Types.h */
/* IfxGtm_Tom_Timer_Config - defined in Ifx_Types.h */

/* Minimal mock types to allow config capture */
/* API declarations */
/* Mock control */
/* Pattern D: capture config fields passed to init() */

/* ============= Function Declarations ============= */
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void);
void IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart(void);
void IfxGtm_Tom_Timer_Mock_Reset(void);

#endif