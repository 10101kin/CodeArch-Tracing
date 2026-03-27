#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Tom_Timer.h"

/* Enums used by this header */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_fast,
    Ifx_Pwm_Mode_centerAligned,
    Ifx_Pwm_Mode_edgeAligned
} Ifx_Pwm_Mode;

/* Forward decl */
struct IfxGtm_Tom_PwmHl;
typedef struct IfxGtm_Tom_PwmHl IfxGtm_Tom_PwmHl;

/* Standard PWM HL interface minimal config used in this header */
typedef struct {
    uint8  channelCount; /* number of PWM outputs (top+bottom count) */
} IfxStdIf_PwmHl_Config;

/* IfxGtm_Tom_ToutMap and pointer form used by config */
typedef struct {
    uint8 tom;      /* TOM index */
    uint8 channel;  /* Channel index */
    void *pin;      /* Placeholder */
} IfxGtm_Tom_ToutMap;
typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* Base structure as in iLLD */
typedef struct
{
    uint32          deadtime;               /* Ifx_TimerValue */
    uint32          minPulse;               /* Ifx_TimerValue */
    uint32          maxPulse;               /* Ifx_TimerValue */
    Ifx_Pwm_Mode    mode;                   
    sint8           setMode;                
    Ifx_ActiveState ccxActiveState;         
    Ifx_ActiveState coutxActiveState;       
    boolean         inverted;               
    uint8           channelCount;           
} IfxGtm_Tom_PwmHl_Base;

/* Mode descriptor (callbacks not used by mocks) */
typedef void (*IfxGtm_Tom_PwmHl_Update)(IfxGtm_Tom_PwmHl *driver, uint32 *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(IfxGtm_Tom_PwmHl *driver, uint32 *tOn, uint32 *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(IfxGtm_Tom_PwmHl *driver, uint32 *tOn, uint32 *tOff);

typedef struct
{
    Ifx_Pwm_Mode                 mode;                 
    boolean                      inverted;             
    IfxGtm_Tom_PwmHl_Update      update;               
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;       
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;          
} IfxGtm_Tom_PwmHl_Mode;

/* Driver config */
typedef struct
{
    IfxStdIf_PwmHl_Config          base;           
    IfxGtm_Tom_Timer              *timer;          
    IfxGtm_Tom                     tom;            
    const IfxGtm_Tom_ToutMapP     *ccx;            
    const IfxGtm_Tom_ToutMapP     *coutx;          
    boolean                        initPins;       
} IfxGtm_Tom_PwmHl_Config;

/* Driver handle (minimal) */
struct IfxGtm_Tom_PwmHl
{
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    IfxGtm_Tom            tom;
};

/* Functions to mock (exact signatures provided) */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, uint32 *tOn /* Ifx_TimerValue* */);

#endif /* IFXGTM_TOM_PWMHL_H */
