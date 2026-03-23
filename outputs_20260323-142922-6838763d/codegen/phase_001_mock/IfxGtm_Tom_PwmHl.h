#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "IfxGtm_Tom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32, boolean, float32, Ifx_TimerValue */

/* ============= Type Definitions ============= */
typedef struct IfxGtm_Tom_PwmHl IfxGtm_Tom_PwmHl;
typedef struct IfxGtm_Tom_PwmHl_Config IfxGtm_Tom_PwmHl_Config;
typedef enum {
    Ifx_Pwm_Mode_leftAligned = 0,
    Ifx_Pwm_Mode_centerAligned = 1
} Ifx_Pwm_Mode;

/* Opaque forward declarations for driver and config */
/* Minimal PWM mode enum with full type-name prefix */
/* API Declarations */
/* Mock control: call counts */
/* Mock control: set returns for non-void */
/* Mock control: last-arg capture accessors */
/* Array capture for tOn parameter (Pattern B) */
/* Mock control: reset */

/* ============= Function Declarations ============= */
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
boolean IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, float32 minPulse);
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMinPulse(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setDeadtime(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig(void);
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMode(boolean value);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMinPulse(boolean value);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setDeadtime(boolean value);
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value);
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMode_mode(void);
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMinPulse_value(void);
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setDeadtime_value(void);
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(uint32 index);
float32 IfxGtm_Tom_PwmHl_Mock_GetArgHistory_setOnTime(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Tom_PwmHl_Mock_GetArgHistoryCount_setOnTime(void);
void IfxGtm_Tom_PwmHl_Mock_Reset(void);

#endif