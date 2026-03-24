#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Mock controls */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
uint32  IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void);
void    IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value);
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_syncStart(void);
void    IfxGtm_Tom_PwmHl_Mock_Reset(void);

#endif