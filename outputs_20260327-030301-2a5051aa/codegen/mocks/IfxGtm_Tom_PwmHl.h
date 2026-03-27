#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxStdIf.h"
#include "IfxGtm_Tom_Timer.h"

#ifndef IFX_CONST
#define IFX_CONST const
#endif

/* Callback typedefs for mode operations */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void *driver, Ifx_TimerValue *pulse);

/* Base */
typedef struct
{
    Ifx_TimerValue  deadtime;
    Ifx_TimerValue  minPulse;
    Ifx_TimerValue  maxPulse;
    Ifx_Pwm_Mode    mode;
    signed char     setMode;
    Ifx_ActiveState ccxActiveState;
    Ifx_ActiveState coutxActiveState;
    boolean         inverted;
    uint8           channelCount;
} IfxGtm_Tom_PwmHl_Base;

/* Config */
typedef struct
{
    IfxStdIf_PwmHl_Config         base;     /* channelCount in base */
    IfxGtm_Tom_Timer             *timer;    /* linked timer */
    IfxGtm_Tom                    tom;      /* tom unit */
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;     /* top channels */
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;   /* bottom channels */
    boolean                       initPins; /* init pins */
} IfxGtm_Tom_PwmHl_Config;

/* Mode description */
typedef struct
{
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Driver */
typedef struct
{
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
} IfxGtm_Tom_PwmHl;

/* Function declarations (subset required) */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

/* StdIf adapter prototypes expected by template */
void    IfxStdIf_Timer_disableUpdate(void *driver);
void    IfxStdIf_Timer_setPeriod(void *driver, float32 period);
void    IfxStdIf_PwmHl_setOnTime(void *driver, Ifx_TimerValue *tOn);
void    IfxStdIf_Timer_applyUpdate(void *driver);

#endif /* IFXGTM_TOM_PWMHL_H */
