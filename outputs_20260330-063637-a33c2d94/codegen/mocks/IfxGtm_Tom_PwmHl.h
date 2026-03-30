#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* StdIf PWM HL base configuration (augmented to satisfy tests) */
typedef struct {
    float32             frequency;     /* required for spy capture */
    uint32              channelCount;  /* number of PWM channels (top+bottom counts as 2) */
    IfxPort_OutputMode  outputMode;    /* required by tests */
    IfxPort_PadDriver   padDriver;     /* required by tests */
    Ifx_ActiveState     activeState;   /* required by tests */
    void               *pins;          /* required by tests */
} IfxStdIf_PwmHl_Config;

/* Pwm Mode */
typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_centerAligned = 1,
    Ifx_Pwm_Mode_edgeAligned = 2
} Ifx_Pwm_Mode;

/* Function pointer typedefs used by Mode (placeholders) */
typedef void (*IfxGtm_Tom_PwmHl_Update)(void *driver, Ifx_TimerValue *tOn);
typedef void (*IfxGtm_Tom_PwmHl_UpdateShift)(void *driver, Ifx_TimerValue *tOn, Ifx_TimerValue *shift);
typedef void (*IfxGtm_Tom_PwmHl_UpdatePulse)(void *driver, Ifx_TimerValue *tHigh, Ifx_TimerValue *tLow);

/* Verified type definitions (emit verbatim) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Tom_Pwm_ClockSource;

/* Additional types referenced by verified config */
typedef struct {
    uint8 enabled;
    uint8 priority;
    uint8 typeOfService;
} IfxGtm_Tom_Pwm_Interrupt;

typedef struct {
    Ifx_P             *port;
    uint8              pinIndex;
    IfxPort_OutputMode outputMode;
    IfxPort_PadDriver  padDriver;
} IfxGtm_Tom_Pwm_pin;

/* Pwm OUT pin map type used by Pinmap externs */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
    uint8  tom;
    uint8  channel;
} IfxGtm_Pwm_ToutMap;

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

/* PwmHl base from template */
typedef struct
{
    Ifx_TimerValue  deadtime;               /**< Dead time between the top and bottom channel in ticks */
    Ifx_TimerValue  minPulse;               /**< minimum pulse that is output */
    Ifx_TimerValue  maxPulse;               /**< internal parameter */
    Ifx_Pwm_Mode    mode;                   /**< actual PWM mode */
    sint8           setMode;                /**< non zero when mode is being modified */
    Ifx_ActiveState ccxActiveState;         /**< Top PWM active state */
    Ifx_ActiveState coutxActiveState;       /**< Bottom PWM active state */
    boolean         inverted;               /**< center aligned inverted mode flag */
    uint8           channelCount;           /**< Number of PWM channels (top+bottom form a channel) */
} IfxGtm_Tom_PwmHl_Base;

/* Forward map pointer type used by config */
typedef const IfxGtm_Tom_ToutMap * IfxGtm_Tom_ToutMapP;

typedef struct
{
    IfxStdIf_PwmHl_Config          base;           /**< PWM HL standard interface configuration */
    IfxGtm_Tom_Timer              *timer;          /**< Pointer to the linked timer object */
    IfxGtm_Tom                     tom;            /**< TOM unit used */
    IFX_STATIC IfxGtm_Tom_ToutMapP *ccx;           /**< Pointer to array for CCx channels */
    IFX_STATIC IfxGtm_Tom_ToutMapP *coutx;         /**< Pointer to array for COUTx channels */
    boolean                        initPins;       /**< TRUE: Initialize pins in driver */
} IfxGtm_Tom_PwmHl_Config;

typedef struct {
    Ifx_Pwm_Mode                 mode;                 /**< Pwm Mode */
    boolean                      inverted;             /**< Inverted configuration for the selected mode */
    IfxGtm_Tom_PwmHl_Update      update;               /**< update call back function */
    IfxGtm_Tom_PwmHl_UpdateShift updateAndShift;       /**< update shift call back function */
    IfxGtm_Tom_PwmHl_UpdatePulse updatePulse;          /**< update pulse call back function */
} IfxGtm_Tom_PwmHl_Mode;

typedef struct {
    IfxGtm_Tom_PwmHl_Base base;
} IfxGtm_Tom_PwmHl;

/* API */
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_stdIfPwmHlInit(void);
void    IfxStdIf_Timer_disableUpdate(void);
void    IfxStdIf_Timer_setPeriod(void);
void    IfxStdIf_PwmHl_setOnTime(void);
void    IfxStdIf_Timer_applyUpdate(void);

/* Functions from DRIVERS TO MOCK */
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime);
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

#endif /* IFXGTM_TOM_PWMHL_H */
