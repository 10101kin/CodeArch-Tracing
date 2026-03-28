#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Additional standard interface types for PWM HL (single-owner here) */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_leftAligned,
    Ifx_Pwm_Mode_centerAligned,
    Ifx_Pwm_Mode_centerAlignedInverted
} Ifx_Pwm_Mode;

typedef struct {
    uint32  channelCount;
    float32 frequency;
    boolean inverted;
} IfxStdIf_PwmHl_Config;

/* Clock source union per rules (allows enum assignment without warnings) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Tom_Pwm_ClockSource;

/* PWM HL base per iLLD */
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

/* PwmHl Tout map types and aliases */
typedef IfxGtm_Tom_ToutMap IfxGtm_Tom_ToutMapP;
typedef struct { IfxGtm_Tom_ToutMap inner; } IfxGtm_Tom_Pwm_ToutMap;

/* PWM HL config per iLLD + required extras (numChannels, frequency, clockSource) */
typedef struct {
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;
    boolean                        initPins;
    /* Required extras for tests */
    uint32                         numChannels;
    float32                        frequency;
    IfxGtm_Tom_Pwm_ClockSource     clockSource;
} IfxGtm_Tom_PwmHl_Config;

/* Driver object (minimal) */
typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
} IfxGtm_Tom_PwmHl;

/* Functions to mock */
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

/* Pin symbol externs required by production code */
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT;
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_4_TOUT14_P00_5_OUT;
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_6_TOUT16_P00_7_OUT;
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_1_TOUT11_P00_2_OUT;
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT;
extern IfxGtm_Tom_Pwm_ToutMap IfxGtm_TOM1_5_TOUT15_P00_6_OUT;

#endif /* IFXGTM_TOM_PWMHL_H */
