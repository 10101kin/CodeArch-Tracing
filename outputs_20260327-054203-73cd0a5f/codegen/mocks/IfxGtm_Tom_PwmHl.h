#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* Pwm mode enum */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_edgeAligned,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

/* Standard PWM HL interface config */
typedef struct {
    uint8   channelCount;   /* number of PWM HL channels (top/bottom pairs) */
    float32 frequency;      /* for spy capture convenience */
} IfxStdIf_PwmHl_Config;

/* ToutMap structure (pin mapping) */
typedef struct {
    Ifx_P            *port;     /* port module */
    uint8             pinIndex; /* pin number */
    IfxPort_OutputIdx outIdx;   /* output index */
} IfxGtm_Tom_ToutMap;

typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* PwmHl base (matches iLLD layout) */
typedef struct {
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

/* PwmHl mode descriptor (callbacks omitted in mock) */
typedef void (*IfxGtm_Tom_PwmHl_Update)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

typedef struct {
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Forward declare main struct */
typedef struct IfxGtm_Tom_PwmHl IfxGtm_Tom_PwmHl;

/* Config struct (matches required layout + base) */
typedef struct {
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    IfxGtm_Tom_ToutMapP            ccx;   /* array pointer of size base.channelCount/2 */
    IfxGtm_Tom_ToutMapP            coutx; /* array pointer of size base.channelCount/2 */
    boolean                        initPins;
} IfxGtm_Tom_PwmHl_Config;

/* Driver object (minimal) */
struct IfxGtm_Tom_PwmHl {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
};

/* Function declarations (subset to mock) */
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

#endif /* IFXGTM_TOM_PWMHL_H */
