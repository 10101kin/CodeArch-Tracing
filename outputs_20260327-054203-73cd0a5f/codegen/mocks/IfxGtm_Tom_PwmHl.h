#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxStdIf.h"
#include "IfxGtm_Tom_Timer.h"

/* Callback typedefs used in Mode struct */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void);

/* Base structure */
typedef struct {
    Ifx_TimerValue  deadtime;        /* ticks */
    Ifx_TimerValue  minPulse;        /* ticks */
    Ifx_TimerValue  maxPulse;        /* internal */
    Ifx_Pwm_Mode    mode;            /* actual PWM mode */
    sint8           setMode;         /* mode transition flag */
    Ifx_ActiveState ccxActiveState;  /* Top PWM active state */
    Ifx_ActiveState coutxActiveState;/* Bottom PWM active state */
    boolean         inverted;        /* center aligned inverted mode */
    uint8           channelCount;    /* pairs*2 */
} IfxGtm_Tom_PwmHl_Base;

/* Config */
typedef struct {
    IfxStdIf_PwmHl_Config          base;    /* standard IF */
    IfxGtm_Tom_Timer              *timer;   /* linked timer */
    IfxGtm_Tom                     tom;     /* TOM unit */
    const IfxGtm_Tom_ToutMapP     *ccx;     /* array size base.channelCount/2 */
    const IfxGtm_Tom_ToutMapP     *coutx;   /* array size base.channelCount/2 */
    boolean                        initPins;/* TRUE init pins */
} IfxGtm_Tom_PwmHl_Config;

/* Mode */
typedef struct {
    Ifx_Pwm_Mode                  mode;
    boolean                       inverted;
    IfxGtm_Tom_PwmHl_Update       update;
    IfxGtm_Tom_PwmHl_UpdateShift  updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse  updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Driver object (minimal) */
typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
} IfxGtm_Tom_PwmHl;

/* Functions (subset used by production) */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

#endif /* IFXGTM_TOM_PWMHL_H */
