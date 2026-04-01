#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"

/* Minimal dependent types for StdIf and TOM */
typedef uint32 Ifx_TimerValue;

typedef enum
{
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct
{
    uint32 dummy;
} IfxStdIf_Timer_Config;

typedef enum
{
    IfxGtm_Tom_0 = 0,
    IfxGtm_Tom_1 = 1,
    IfxGtm_Tom_2 = 2,
    IfxGtm_Tom_3 = 3
} IfxGtm_Tom;

typedef uint8 IfxGtm_Tom_Ch;

typedef struct { uint32 dummy; } IfxGtm_Tom_ToutMap;

typedef enum
{
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk1 = 1
} IfxGtm_Tom_Ch_ClkSrc;

/* Tom Timer types (from iLLD mapping) */
typedef struct
{
    Ifx_TimerValue          period;               
    boolean                 triggerEnabled;       
    float32                 clockFreq;            
    IfxStdIf_Timer_CountDir countDir;             
} IfxGtm_Tom_Timer_Base;

typedef struct
{
    IfxGtm_Tom_Timer_Base base;                                   
    Ifx_GTM              *gtm;                                    
    Ifx_GTM_TOM          *tom;                                    
    Ifx_GTM_TOM_TGC      *tgc[2];                                 
    IfxGtm_Tom            tomIndex;                               
    IfxGtm_Tom_Ch         timerChannel;                           
    IfxGtm_Tom_Ch         triggerChannel;                         
    uint16                channelsMask[2];                        
    Ifx_TimerValue        offset;                                 
    uint32                tgcGlobalControlDisableUpdate[2];       
    uint32                tgcGlobalControlApplyUpdate[2];         
    Ifx_GTM_CDTM_DTM     *dtm;                                    
    uint8                 dtmChannel;                             
} IfxGtm_Tom_Timer;

typedef struct
{
    IfxStdIf_Timer_Config  base;                 
    Ifx_GTM               *gtm;                  
    IfxGtm_Tom             tom;                  
    IfxGtm_Tom_Ch          timerChannel;         
    IfxGtm_Tom_ToutMap    *triggerOut;           
    IfxGtm_Tom_Ch_ClkSrc   clock;                
    IfxGtm_IrqMode         irqModeTimer;         
    IfxGtm_IrqMode         irqModeTrigger;       
    IfxGtm_Dtm_ClockSource dtmClockSource;       
    boolean                initPins;             /**< \brief TRUE : Initialize pins in driver.
                                                  * FALSE: Don't initialize pins : handled separately by user */
} IfxGtm_Tom_Timer_Config;

/* API subset required by the module */
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);

#endif /* IFXGTM_TOM_TIMER_H */
