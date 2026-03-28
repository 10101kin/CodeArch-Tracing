#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"
#include "IfxPort.h"

/* PWM mode enum */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_edgeAligned,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

/* Forward declare update callback types used by Mode (function pointer types) */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* Standard interface config for PwmHl (single owner here) */
typedef struct {
    float32 frequency;   /* Hz */
    uint32  numChannels; /* number of PWM HL channels */
} IfxStdIf_PwmHl_Config;

/* Include Pwm ToutMap typedefs here (tests may reference IfxGtm_Pwm_ToutMap) */
typedef struct IfxGtm_Tom_ToutMap IfxGtm_Tom_ToutMap; /* from Timer header */
typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;
typedef IfxGtm_Tom_ToutMap IfxGtm_Pwm_ToutMap; /* alias to satisfy references */

/* Base structure (matches iLLD layout) */
typedef struct {
    Ifx_TimerValue  deadtime;               /* Top-Bottom dead time (ticks) */
    Ifx_TimerValue  minPulse;               /* minimum pulse (ticks) */
    Ifx_TimerValue  maxPulse;               /* internal parameter */
    Ifx_Pwm_Mode    mode;                   /* actual PWM mode */
    signed char     setMode;                /* non-zero while changing mode */
    Ifx_ActiveState ccxActiveState;         /* Top PWM active state */
    Ifx_ActiveState coutxActiveState;       /* Bottom PWM active state */
    boolean         inverted;               /* center aligned inverted mode */
    uint8           channelCount;           /* number of PWM channels (pairs) */
} IfxGtm_Tom_PwmHl_Base;

/* Config structure (matches iLLD layout; plus mock-top fields for tests if provided) */
typedef struct {
    IfxStdIf_PwmHl_Config          base;           /* PWM HL standard interface configuration */
    IfxGtm_Tom_Timer              *timer;          /* linked timer */
    IfxGtm_Tom                     tom;            /* TOM unit used */
    const IfxGtm_Tom_ToutMapP     *ccx;            /* array of ccx channels */
    const IfxGtm_Tom_ToutMapP     *coutx;          /* array of coutx channels */
    boolean                        initPins;       /* TRUE: init pins in driver */
    /* Optional top-level fields (mock-friendly) used by tests to pass frequency/channel count */
    float32                        frequency;      /* Hz */
    uint32                         numChannels;    /* channels */
} IfxGtm_Tom_PwmHl_Config;

/* Mode description (kept for completeness) */
typedef struct {
    Ifx_Pwm_Mode                 mode;                 /* Pwm Mode */
    boolean                      inverted;             /* Inverted configuration */
    IfxGtm_Tom_PwmHl_Update      update;               /* update callback */
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;       /* update + shift callback */
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;          /* update pulse callback */
} IfxGtm_Tom_PwmHl_Mode;

/* Driver object (minimal representation) */
typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
} IfxGtm_Tom_PwmHl;

/* Functions from DRIVERS TO MOCK */
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

#endif /* IFXGTM_TOM_PWMHL_H */
