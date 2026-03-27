/* IfxGtm_Tom_PwmHl.h - mock */
#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* Pwm mode and std-if config used by PwmHl */
typedef enum
{
    Ifx_Pwm_Mode_centerAligned = 0,
    Ifx_Pwm_Mode_leftAligned,
    Ifx_Pwm_Mode_rightAligned
} Ifx_Pwm_Mode;

typedef struct
{
    uint8   channelCount;  /* total number of PWM channels (top+bottom) */
    float32 deadtime;      /* desired deadtime in ticks */
    float32 minPulse;      /* desired min pulse */
} IfxStdIf_PwmHl_Config;

/* IfxGtm_Tom_PwmHl_Base (from iLLD) */
typedef struct
{
    Ifx_TimerValue  deadtime;               /* dead time between top/bottom */
    Ifx_TimerValue  minPulse;               /* minimum pulse */
    Ifx_TimerValue  maxPulse;               /* internal */
    Ifx_Pwm_Mode    mode;                   /* current mode */
    sint8           setMode;                /* set-mode flag */
    Ifx_ActiveState ccxActiveState;         /* top active state */
    Ifx_ActiveState coutxActiveState;       /* bottom active state */
    boolean         inverted;               /* inverted flag */
    uint8           channelCount;           /* number of channels (pairs) */
} IfxGtm_Tom_PwmHl_Base;

/* Forward-declare mapping pointer type from PinMap */
typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* IfxGtm_Tom_PwmHl_Config (from iLLD) */
typedef struct
{
    IfxStdIf_PwmHl_Config   base;    /* std if config */
    IfxGtm_Tom_Timer       *timer;   /* linked timer */
    IfxGtm_Tom              tom;     /* TOM unit */
    IfxGtm_Tom_ToutMapP    *ccx;     /* top channel maps */
    IfxGtm_Tom_ToutMapP    *coutx;   /* bottom channel maps */
    boolean                 initPins; /* init pins in driver */
} IfxGtm_Tom_PwmHl_Config;

/* Mode descriptor (not used directly by tests) */
typedef struct
{
    Ifx_Pwm_Mode mode;
    boolean      inverted;
    void        *update;          /* callback placeholder */
    void        *updateAndShift;  /* callback placeholder */
    void        *updatePulse;     /* callback placeholder */
} IfxGtm_Tom_PwmHl_Mode;

/* Driver handle */
typedef struct
{
    IfxGtm_Tom_PwmHl_Base base;  /* base fields */
    IfxGtm_Tom_Timer     *timer; /* linked timer */
    uint8                 channels; /* number of channel pairs */
} IfxGtm_Tom_PwmHl;

/* API declarations */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

#endif /* IFXGTM_TOM_PWMHL_H */
