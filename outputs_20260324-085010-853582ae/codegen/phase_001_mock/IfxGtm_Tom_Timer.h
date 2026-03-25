#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
struct IfxGtm_Tom_Timer_Config {
    float32 frequency;   /* Hz */
    uint32  alignment;   /* enum-backed value */
    uint32  syncStart;   /* boolean/enum-backed */
};
struct IfxGtm_Tom_Timer {
    uint32 dummy;        /* placeholder to allow stack instances */
};

/* Minimal driver/config placeholders to enable init() field capture */
/* API declarations */
/* Mock controls */
/* Captured config fields for init() */

/* ============= Function Declarations ============= */
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value);
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart(void);
void    IfxGtm_Tom_Timer_Mock_Reset(void);

#endif