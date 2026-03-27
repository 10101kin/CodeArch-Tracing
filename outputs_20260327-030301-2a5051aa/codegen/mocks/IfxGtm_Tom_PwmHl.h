#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* Supporting types/enums */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_leftAligned,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

typedef struct {
    uint8   channelCount;
    float32 frequency;
} IfxStdIf_PwmHl_Config;

/* Callback typedefs for mode struct */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void* driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void* driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tShift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void* driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* Base/Mode/Config/Driver structs */
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

typedef struct {
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

typedef struct {
    IfxStdIf_PwmHl_Config   base;
    IfxGtm_Tom_Timer       *timer;
    IfxGtm_Tom              tom;
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;
    boolean                 initPins;
} IfxGtm_Tom_PwmHl_Config;

typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
} IfxGtm_Tom_PwmHl;

/* Function declarations */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_stdIfPwmHlInit(IfxGtm_Tom_PwmHl *driver);
void    IfxStdIf_Timer_disableUpdate(IfxGtm_Tom_PwmHl *driver);
void    IfxStdIf_Timer_setPeriod(IfxGtm_Tom_PwmHl *driver, float32 period);
void    IfxStdIf_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
void    IfxStdIf_Timer_applyUpdate(IfxGtm_Tom_PwmHl *driver);
Ifx_TimerValue IfxGtm_Tom_PwmHl_getDeadtime(IfxGtm_Tom_PwmHl *driver);
Ifx_TimerValue IfxGtm_Tom_PwmHl_getMinPulse(IfxGtm_Tom_PwmHl *driver);
Ifx_Pwm_Mode   IfxGtm_Tom_PwmHl_getMode(IfxGtm_Tom_PwmHl *driver);
void    IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue dt);
void    IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue mp);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
void    IfxGtm_Tom_PwmHl_setOnTimeAndShift(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tShift);
void    IfxGtm_Tom_PwmHl_setPulse(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);
void    IfxGtm_Tom_PwmHl_setupChannels(IfxGtm_Tom_PwmHl *driver);
void    IfxGtm_Tom_PwmHl_stdIfPwmHlInit(IfxGtm_Tom_PwmHl *driver);

#endif /* IFXGTM_TOM_PWMHL_H */
