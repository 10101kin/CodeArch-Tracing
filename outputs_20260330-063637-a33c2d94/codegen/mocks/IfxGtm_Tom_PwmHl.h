#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Support types for PWM HL */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_centerAligned = 1,
    Ifx_Pwm_Mode_edgeAligned = 2
} Ifx_Pwm_Mode;

/* Map structure for TOM TOUT */
typedef struct IfxGtm_Tom_ToutMap
{
    Ifx_P       *port;
    uint8        pinIndex;
    uint8        tom;      /* IfxGtm_Tom index */
    uint8        channel;  /* IfxGtm_Tom_Ch */
} IfxGtm_Tom_ToutMap;

/* Separate map type used in tests for pin symbols */
typedef struct IfxGtm_Pwm_ToutMap
{
    uint32 placeholder;
} IfxGtm_Pwm_ToutMap;

/* Minimal StdIf PwmHl config with required fields for spy capture */
typedef struct
{
    float32 frequency;    /* required for spy capture */
    uint32  channelCount; /* total channels (pairs) or logical count */
} IfxStdIf_PwmHl_Config;

/* Base structure per mapping */
typedef struct
{
    Ifx_TimerValue  deadtime;
    Ifx_TimerValue  minPulse;
    Ifx_TimerValue  maxPulse;
    Ifx_Pwm_Mode    mode;
    sint8           setMode;
    Ifx_ActiveState ccxActiveState;
    Ifx_ActiveState coutxActiveState;
    boolean         inverted;
    uint8           channelCount;
} IfxGtm_Tom_PwmHl_Base;

/* PWM HL Config - include extra fields for spy capture per spec */
typedef struct
{
    IfxStdIf_PwmHl_Config   base;
    IfxGtm_Tom_Timer       *timer;
    IfxGtm_Tom              tom;
    const IfxGtm_Tom_ToutMap *ccx;
    const IfxGtm_Tom_ToutMap *coutx;
    boolean                 initPins;
    /* Spy-capture friendly fields */
    float32                 frequency;   /* duplicate for convenience */
    uint32                  numChannels; /* number of channels used by application */
} IfxGtm_Tom_PwmHl_Config;

/* Driver object */
typedef struct
{
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
    uint8                 usedChannels;
} IfxGtm_Tom_PwmHl;

/* Functions to mock */
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);

#endif /* IFXGTM_TOM_PWMHL_H */
