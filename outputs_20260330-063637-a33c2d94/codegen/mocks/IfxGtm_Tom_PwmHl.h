#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* StdIf PwmHl dependencies */
typedef enum {
    Ifx_Pwm_Mode_centerAligned = 0,
    Ifx_Pwm_Mode_edgeAligned   = 1
} Ifx_Pwm_Mode;

typedef struct {
    float32 frequency;     /* PWM base frequency */
    uint8   channelCount;  /* total high+low channel count */
} IfxStdIf_PwmHl_Config;

/* Function pointer typedefs used by Mode (minimal placeholders) */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void);

/* VERIFIED TYPE DEFINITIONS — copy verbatim order */
typedef struct
{
    Ifx_TimerValue  deadtime;               /**< Dead time between the top and bottom channel in ticks */
    Ifx_TimerValue  minPulse;               /**< minimum pulse that is output */
    Ifx_TimerValue  maxPulse;               /**< internal parameter */
    Ifx_Pwm_Mode    mode;                   /**< actual PWM mode */
    sint8           setMode;                /**< A non zero flag indicates that the PWM mode is being modified */
    Ifx_ActiveState ccxActiveState;         /**< Top PWM active state */
    Ifx_ActiveState coutxActiveState;       /**< Bottom PWM active state */
    boolean         inverted;               /**< Flag indicating the center aligned inverted mode (TRUE) */
    uint8           channelCount;           /**< Number of PWM channels, one channel is made of a top and bottom channel */
} IfxGtm_Tom_PwmHl_Base;

typedef struct
{
    IfxStdIf_PwmHl_Config          base;           /**< PWM HL standard interface configuration */
    IfxGtm_Tom_Timer              *timer;          /**< Pointer to the linked timer object */
    IfxGtm_Tom                     tom;            /**< TOM unit used */
    IFX_CONST IfxGtm_Tom_ToutMapP *ccx;            /**< Pointer to array of channels used for ccx */
    IFX_CONST IfxGtm_Tom_ToutMapP *coutx;          /**< Pointer to array of channels used for coutx */
    boolean                        initPins;       /**< TRUE: Initialize pins in driver */
} IfxGtm_Tom_PwmHl_Config;

typedef struct
{
    Ifx_Pwm_Mode                 mode;                 /**< Pwm Mode */
    boolean                      inverted;             /**< Inverted configuration for the selected mode */
    IfxGtm_Tom_PwmHl_Update      update;               /**< update call back function for the selected mode */
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;       /**< update shift call back function for the selected mode */
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;          /**< update pulse call back function for the selected mode */
} IfxGtm_Tom_PwmHl_Mode;

/* Driver object (minimal) */
typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
} IfxGtm_Tom_PwmHl;

/* Functions from DRIVERS TO MOCK */
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);

#endif /* IFXGTM_TOM_PWMHL_H */
