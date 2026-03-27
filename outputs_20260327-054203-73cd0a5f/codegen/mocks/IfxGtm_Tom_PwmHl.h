#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxStdIf.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* Enums and simple typedefs first */
typedef enum
{
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_high,
    Ifx_Pwm_Mode_low,
    Ifx_Pwm_Mode_centerAligned,
    Ifx_Pwm_Mode_edgeAligned
} Ifx_Pwm_Mode;

/* GTM TOM Tout mapping types (simplified) */
typedef struct
{
    uint8 tom;    /* TOM index */
    uint8 channel;/* TOM channel */
    uint8 tout;   /* TOUT number */
    uint8 port;   /* Port index */
} IfxGtm_Tom_ToutMap;

typedef const IfxGtm_Tom_ToutMap * IfxGtm_Tom_ToutMapP;

/* Forward declarations for standard interface PWM HL function pointer types used in Mode */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* Base structure from iLLD (field-complete) */
typedef struct
{
    Ifx_TimerValue  deadtime;               /* Dead time between the top and bottom channel in ticks */
    Ifx_TimerValue  minPulse;               /* minimum pulse that is output */
    Ifx_TimerValue  maxPulse;               /* internal parameter */
    Ifx_Pwm_Mode    mode;                   /* actual PWM mode */
    sint8           setMode;                /* non zero when PWM mode is being modified */
    Ifx_ActiveState ccxActiveState;         /* Top PWM active state */
    Ifx_ActiveState coutxActiveState;       /* Bottom PWM active state */
    boolean         inverted;               /* Center aligned inverted mode */
    uint8           channelCount;           /* Number of PWM channels (top+bottom) */
} IfxGtm_Tom_PwmHl_Base;

/* Mode structure (from iLLD) */
typedef struct
{
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Driver config (from iLLD) */
typedef struct
{
    IfxStdIf_PwmHl_Config          base;     /* PWM HL standard interface configuration */
    IfxGtm_Tom_Timer              *timer;    /* linked timer object */
    IfxGtm_Tom                     tom;      /* TOM unit used */
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;      /* array of size base.channelCount/2 */
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;    /* array of size base.channelCount/2 */
    boolean                        initPins; /* TRUE: init pins in driver */
} IfxGtm_Tom_PwmHl_Config;

/* Driver handle (simplified) */
typedef struct
{
    IfxGtm_Tom_PwmHl_Base base;
} IfxGtm_Tom_PwmHl;

/* Function declarations (per template requirements) */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_stdIfPwmHlInit(void *stdif, IfxGtm_Tom_PwmHl *driver);

/* StdIf Timer helpers exposed in this header per template */
void    IfxStdIf_Timer_disableUpdate(IfxGtm_Tom_Timer *timer);
void    IfxStdIf_Timer_setPeriod(IfxGtm_Tom_Timer *timer, Ifx_TimerValue period);
void    IfxStdIf_Timer_applyUpdate(IfxGtm_Tom_Timer *timer);

/* StdIf PwmHl helper */
void    IfxStdIf_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

/* PwmHl API (subset used by production and completeness) */
Ifx_TimerValue IfxGtm_Tom_PwmHl_getDeadtime(IfxGtm_Tom_PwmHl *driver);
Ifx_TimerValue IfxGtm_Tom_PwmHl_getMinPulse(IfxGtm_Tom_PwmHl *driver);
Ifx_Pwm_Mode   IfxGtm_Tom_PwmHl_getMode(IfxGtm_Tom_PwmHl *driver);
void           IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue rising, Ifx_TimerValue falling);
void           IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue minPulse);
boolean        IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void           IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
void           IfxGtm_Tom_PwmHl_setOnTimeAndShift(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
void           IfxGtm_Tom_PwmHl_setPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);
void           IfxGtm_Tom_PwmHl_setupChannels(IfxGtm_Tom_PwmHl *driver, uint32 mask);

#endif /* IFXGTM_TOM_PWMHL_H */
