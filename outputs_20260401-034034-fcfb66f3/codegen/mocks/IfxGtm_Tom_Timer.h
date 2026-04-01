#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"

/* Dependent/basic types for TOM Timer */
typedef uint32 Ifx_TimerValue;

typedef enum { IfxStdIf_Timer_CountDir_up = 0, IfxStdIf_Timer_CountDir_down = 1 } IfxStdIf_Timer_CountDir;

typedef struct { uint32 dummy; } IfxStdIf_Timer_Config;

typedef enum { IfxGtm_Tom_0 = 0, IfxGtm_Tom_1 = 1, IfxGtm_Tom_2 = 2, IfxGtm_Tom_3 = 3 } IfxGtm_Tom;

typedef enum { IfxGtm_Tom_Ch_0 = 0, IfxGtm_Tom_Ch_1, IfxGtm_Tom_Ch_2, IfxGtm_Tom_Ch_3, IfxGtm_Tom_Ch_4, IfxGtm_Tom_Ch_5, IfxGtm_Tom_Ch_6, IfxGtm_Tom_Ch_7 } IfxGtm_Tom_Ch;

typedef struct { uint32 dummy; } IfxGtm_Tom_ToutMap;

typedef enum { IfxGtm_Tom_Ch_ClkSrc_cmuClk0 = 0, IfxGtm_Tom_Ch_ClkSrc_cmuClk1 = 1, IfxGtm_Tom_Ch_ClkSrc_cmuClk2 = 2 } IfxGtm_Tom_Ch_ClkSrc;

typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

typedef enum { IfxGtm_Dtm_Ch_0 = 0, IfxGtm_Dtm_Ch_1 = 1 } IfxGtm_Dtm_Ch;

/* Structs from mapping */
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
    IfxGtm_Dtm_Ch         dtmChannel;                             
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
    boolean                initPins;             
} IfxGtm_Tom_Timer_Config;

/* Functions used by module */
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
