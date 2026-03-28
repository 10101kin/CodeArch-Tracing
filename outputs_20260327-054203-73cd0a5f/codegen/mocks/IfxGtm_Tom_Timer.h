#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_PwmHl.h" /* for IfxGtm_Tom enums and TOM/DTM/TGC stubs */

/* Enums required by Timer base/config */
typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

/* StdIf Timer config */
typedef struct {
    float32                 frequency;
    IfxStdIf_Timer_CountDir countDir;
} IfxStdIf_Timer_Config;

/* Forward declarations for SFR pointers are provided in IfxGtm_Tom_PwmHl.h */

/* IfxGtm_Tom_Timer_Base definition */
typedef struct
{
    Ifx_TimerValue          period;               /* cached period */
    boolean                 triggerEnabled;       /* trigger init flag */
    float32                 clockFreq;            /* input clock freq */
    IfxStdIf_Timer_CountDir countDir;             /* count direction */
} IfxGtm_Tom_Timer_Base;

/* IfxGtm_Tom_Timer definition */
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
    IfxGtm_Dtm_Ch         dtmChannel;                
} IfxGtm_Tom_Timer;

/* IfxGtm_Tom_Timer_Config definition */
typedef struct
{
    IfxStdIf_Timer_Config  base;                 
    Ifx_GTM               *gtm;                  
    IfxGtm_Tom             tom;                  
    IfxGtm_Tom_Ch          timerChannel;         
    void                  *triggerOut;           /* simplify pin map reference */
    IfxGtm_Tom_Ch_ClkSrc   clock;                
    IfxGtm_IrqMode         irqModeTimer;         
    IfxGtm_IrqMode         irqModeTrigger;       
    IfxGtm_Dtm_ClockSource dtmClockSource;       
    boolean                initPins;             
} IfxGtm_Tom_Timer_Config;

/* Function declarations (DRIVERS TO MOCK set) */
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);

/* Minimal StdIf for timer to avoid unknown references */
void IfxStdIf_Timer_run(void *driver);
void IfxStdIf_Timer_disableUpdate(void *driver);
void IfxStdIf_Timer_setPeriod(void *driver, Ifx_TimerValue period);
void IfxStdIf_Timer_applyUpdate(void *driver);

#endif /* IFXGTM_TOM_TIMER_H */
