#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxGtm_Tom_Timer.h"

/* PWM Mode */
typedef enum { Ifx_Pwm_Mode_edgeAligned = 0, Ifx_Pwm_Mode_centerAligned = 1 } Ifx_Pwm_Mode;

/* StdIf PwmHl config with fields used by production */
typedef struct
{
    uint8               channelCount;     /* number of PWM channels (pairs) */
    float32             frequency;        /* used by tests for capture */
    IfxPort_OutputMode  outputMode;       /* required by tests */
    IfxPort_PadDriver   padDriver;        /* required by tests */
} IfxStdIf_PwmHl_Config;

/* Tout mapping types */
struct IfxGtm_Tom_ToutMap
{
    Ifx_P        *port;
    uint8         pinIndex;
    IfxGtm_Tom    tom;
    IfxGtm_Tom_Ch channel;
};
typedef const struct IfxGtm_Tom_ToutMap * IfxGtm_Tom_ToutMapP;

typedef struct
{
    Ifx_P *port;
    uint8  pinIndex;
} IfxGtm_Pwm_ToutMap;

/* Callback types used by Mode structure */
struct IfxGtm_Tom_PwmHl; /* forward for callbacks */
typedef void (*IfxGtm_Tom_PwmHl_Update)(struct IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(struct IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(struct IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* IfxGtm_Tom_PwmHl_Base (from iLLD) */
typedef struct
{
    Ifx_TimerValue  deadtime;               /* Dead time between the top and bottom channel in ticks */
    Ifx_TimerValue  minPulse;               /* minimum pulse that is output */
    Ifx_TimerValue  maxPulse;               /* internal parameter */
    Ifx_Pwm_Mode    mode;                   /* actual PWM mode */
    sint16          setMode;                /* A non zero flag indicates that the PWM mode is being modified */
    Ifx_ActiveState ccxActiveState;         /* Top PWM active state */
    Ifx_ActiveState coutxActiveState;       /* Bottom PWM active state */
    boolean         inverted;               /* Flag indicating the center aligned inverted mode (TRUE) */
    uint8           channelCount;           /* Number of PWM channels */
} IfxGtm_Tom_PwmHl_Base;

/* IfxGtm_Tom_PwmHl_Config (from iLLD, extended via StdIf_PwmHl_Config) */
typedef struct
{
    IfxStdIf_PwmHl_Config          base;           /* PWM HL standard interface configuration */
    IfxGtm_Tom_Timer              *timer;          /* Pointer to the linked timer object */
    IfxGtm_Tom                     tom;            /* TOM unit used */
    IFX_STATIC IfxGtm_Tom_ToutMapP ccx;            /* channels used for CCX */
    IFX_STATIC IfxGtm_Tom_ToutMapP coutx;          /* channels used for COUTX */
    boolean                        initPins;       /* pin initialization flag */
} IfxGtm_Tom_PwmHl_Config;

/* IfxGtm_Tom_PwmHl_Mode (from iLLD) */
typedef struct
{
    Ifx_Pwm_Mode                 mode;                 /* Pwm Mode */
    boolean                      inverted;             /* Inverted configuration for the selected mode */
    IfxGtm_Tom_PwmHl_Update      update;               /* update call back function for the selected mode */
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;       /* update shift call back function for the selected mode */
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;          /* update pulse call back function for the selected mode */
} IfxGtm_Tom_PwmHl_Mode;

/* Driver object (minimal) */
typedef struct IfxGtm_Tom_PwmHl
{
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
} IfxGtm_Tom_PwmHl;

/* API */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

#endif /* IFXGTM_TOM_PWMHL_H */
