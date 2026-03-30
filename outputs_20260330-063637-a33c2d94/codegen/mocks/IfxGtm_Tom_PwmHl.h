#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* StdIf PWM HL config used in PwmHl_Config */
typedef struct {
    uint32  channelCount;
    float32 frequency;
} IfxStdIf_PwmHl_Config;

/* PWM mode enum used in PwmHl_Base and setMode */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_leftAligned,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

/* Callback typedefs used by IfxGtm_Tom_PwmHl_Mode */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* Verified type definitions (union) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Tom_Pwm_ClockSource;

/* Dependent placeholder types used by verified config */
typedef struct { uint32 dummy; } IfxGtm_Tom_Pwm_Interrupt;
typedef struct { Ifx_P *port; uint8 pinIndex; } IfxGtm_Tom_Pwm_pin;

/* Verified type definition (struct) */
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
    IfxGtm_Tom_Pwm_Interrupt interrupt;
    IfxGtm_Tom_Pwm_pin       pin;
    boolean                  immediateStartEnabled;
} IfxGtm_Tom_Pwm_Config;

/* PwmHL base/mode/driver structs */
typedef struct {
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

typedef struct {
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    const IfxGtm_Tom_ToutMapP     *ccx;
    const IfxGtm_Tom_ToutMapP     *coutx;
    boolean                        initPins;
} IfxGtm_Tom_PwmHl_Config;

typedef struct {
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_PwmHl_Mode modeCfg;
} IfxGtm_Tom_PwmHl;

/* Functions to mock */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);

#endif /* IFXGTM_TOM_PWMHL_H */
