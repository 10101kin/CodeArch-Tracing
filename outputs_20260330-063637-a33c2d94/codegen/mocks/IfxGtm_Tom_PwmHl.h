#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* StdIf PWM HL config */
typedef enum { Ifx_Pwm_Mode_off = 0, Ifx_Pwm_Mode_centerAligned = 1, Ifx_Pwm_Mode_edgeAligned = 2 } Ifx_Pwm_Mode;

typedef struct {
    uint8   channelCount;   /* number of PWM channels (HL pairs -> even) */
    float32 frequency;      /* added for test capture */
} IfxStdIf_PwmHl_Config;

/* PwmHl base (from iLLD template) */
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

/* TOM Tout map types for channels/pins */
typedef struct { Ifx_P *port; uint8 pinIndex; } IfxGtm_Tom_ToutMap;
typedef IfxGtm_Tom_ToutMap const * IfxGtm_Tom_ToutMapP;
/* Generic PWM Tout map (used by pin symbol externs) */
typedef IfxGtm_Tom_ToutMap IfxGtm_Pwm_ToutMap;

typedef struct IfxGtm_Tom_PwmHl_s IfxGtm_Tom_PwmHl; /* forward self */

/* PwmHl config (contains StdIf and TOM mapping) */
typedef struct
{
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    IfxGtm_Tom_ToutMapP            ccx;     /* array ptr (size base.channelCount/2) */
    IfxGtm_Tom_ToutMapP            coutx;   /* array ptr (size base.channelCount/2) */
    boolean                        initPins;
    /* Additional convenience fields for tests */
    uint32                         numChannels; /* mirrors base.channelCount */
    float32                        frequency;   /* mirrors base.frequency */
} IfxGtm_Tom_PwmHl_Config;

/* Mode helper type (keep simple; callbacks not needed in mocks) */
typedef struct
{
    Ifx_Pwm_Mode mode;
    boolean      inverted;
    void        *update;
    void        *updateAndShift;
    void        *updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* VERIFIED TYPE DEFINITIONS — copy verbatim (applies to non-HL TOM PWM) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Tom_Pwm_ClockSource;

typedef struct
{
    Ifx_GTM                 *gtm;                            
    IfxGtm_Tom               tom;                            
    IfxGtm_Tom_Ch            tomChannel;                     
    IfxGtm_Tom_Ch_ClkSrc     clock;                          
    uint16                   period;                         
    uint16                   dutyCycle;                      
    Ifx_ActiveState          signalLevel;                    
    boolean                  oneShotModeEnabled;             
    IfxGtm_Dtm_ClockSource   dtmClockSource;                 
    boolean                  synchronousUpdateEnabled;       
    int                      interrupt;                      
    int                      pin;                            
    boolean                  immediateStartEnabled;          
} IfxGtm_Tom_Pwm_Config;

/* Function declarations to mock */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

#endif /* IFXGTM_TOM_PWMHL_H */
