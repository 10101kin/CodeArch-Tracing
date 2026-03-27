#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"

/* Basic PWM mode enum (StdIf) */
typedef enum {
    Ifx_Pwm_Mode_centerAligned = 0,
    Ifx_Pwm_Mode_leftAligned,
    Ifx_Pwm_Mode_rightAligned
} Ifx_Pwm_Mode;

/* StdIf PwmHl Config (minimal) */
typedef struct {
    uint8   channelCount;
    float32 frequency;
} IfxStdIf_PwmHl_Config;

/* Driver base struct (from iLLD) */
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

/* Forward declaration for callbacks */
struct IfxGtm_Tom_PwmHl;
typedef struct IfxGtm_Tom_PwmHl IfxGtm_Tom_PwmHl;

typedef void (*IfxGtm_Tom_PwmHl_Update)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *tOff);

/* Mode struct (from iLLD) */
typedef struct {
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    IfxGtm_Tom_PwmHl_Update      update;
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Config struct (from iLLD) */
typedef IFX_CONST IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

typedef struct {
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;
    boolean                        initPins;
} IfxGtm_Tom_PwmHl_Config;

/* Driver handle (minimal) */
struct IfxGtm_Tom_PwmHl {
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
};

/* Functions to mock (exact signatures provided) */
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);

#endif /* IFXGTM_TOM_PWMHL_H */
