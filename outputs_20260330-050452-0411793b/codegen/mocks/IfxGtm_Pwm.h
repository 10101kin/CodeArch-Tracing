#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Dependent SFR struct stubs referenced by PWM driver */
typedef struct { uint32 reserved; } Ifx_GTM_ATOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM;

typedef struct IfxGtm_Trig_MscOut_s { uint32 reserved; } IfxGtm_Trig_MscOut;

typedef struct { uint32 dummy; } IfxGtm_Atom_ToutMap;
typedef struct { uint32 dummy; } IfxGtm_Tom_ToutMap;

typedef union {
    IfxGtm_Atom_ToutMap atom;
    IfxGtm_Tom_ToutMap  tom;
} IfxGtm_Pwm_ToutMap;

typedef void (*IfxGtm_Pwm_callBack)(void);

/* VERIFIED TYPE DEFINITIONS — emitted exactly as provided */

typedef enum
{
    IfxGtm_Pwm_Alignment_edge   = 0, 
    IfxGtm_Pwm_Alignment_center = 1  
} IfxGtm_Pwm_Alignment;

typedef enum
{
    IfxGtm_Pwm_SubModule_atom = 0,  
    IfxGtm_Pwm_SubModule_tom  = 1   
} IfxGtm_Pwm_SubModule;

typedef enum
{
    IfxGtm_Pwm_SubModule_Ch_0  = 0,   
    IfxGtm_Pwm_SubModule_Ch_1  = 1,   
    IfxGtm_Pwm_SubModule_Ch_2  = 2,   
    IfxGtm_Pwm_SubModule_Ch_3  = 3,   
    IfxGtm_Pwm_SubModule_Ch_4  = 4,   
    IfxGtm_Pwm_SubModule_Ch_5  = 5,   
    IfxGtm_Pwm_SubModule_Ch_6  = 6,   
    IfxGtm_Pwm_SubModule_Ch_7  = 7,   
    IfxGtm_Pwm_SubModule_Ch_8  = 8,   
    IfxGtm_Pwm_SubModule_Ch_9  = 9,   
    IfxGtm_Pwm_SubModule_Ch_10 = 10,  
    IfxGtm_Pwm_SubModule_Ch_11 = 11,  
    IfxGtm_Pwm_SubModule_Ch_12 = 12,  
    IfxGtm_Pwm_SubModule_Ch_13 = 13,  
    IfxGtm_Pwm_SubModule_Ch_14 = 14,  
    IfxGtm_Pwm_SubModule_Ch_15 = 15   
} IfxGtm_Pwm_SubModule_Ch;

typedef enum
{
    IfxGtm_Pwm_ChannelState_running = 0,  
    IfxGtm_Pwm_ChannelState_stopped       
} IfxGtm_Pwm_ChannelState;

typedef enum
{
    IfxGtm_Pwm_ResetEvent_onCm0     = 0, 
    IfxGtm_Pwm_ResetEvent_onTrigger = 1  
} IfxGtm_Pwm_ResetEvent;

typedef enum
{
    IfxGtm_Pwm_State_unknown = -1,  
    IfxGtm_Pwm_State_init    = 0,   
    IfxGtm_Pwm_State_run     = 1,   
    IfxGtm_Pwm_State_stopped = 2,   
    IfxGtm_Pwm_State_error   = 3    
} IfxGtm_Pwm_State;

typedef enum
{
    IfxGtm_Pwm_SyncChannelIndex_0 = 0,  
    IfxGtm_Pwm_SyncChannelIndex_1,      
    IfxGtm_Pwm_SyncChannelIndex_2,      
    IfxGtm_Pwm_SyncChannelIndex_3,      
    IfxGtm_Pwm_SyncChannelIndex_4,      
    IfxGtm_Pwm_SyncChannelIndex_5,      
    IfxGtm_Pwm_SyncChannelIndex_6,      
    IfxGtm_Pwm_SyncChannelIndex_7,      
    IfxGtm_Pwm_SyncChannelIndex_8,      
    IfxGtm_Pwm_SyncChannelIndex_9,      
    IfxGtm_Pwm_SyncChannelIndex_10,     
    IfxGtm_Pwm_SyncChannelIndex_11,     
    IfxGtm_Pwm_SyncChannelIndex_12,     
    IfxGtm_Pwm_SyncChannelIndex_13,     
    IfxGtm_Pwm_SyncChannelIndex_14,     
    IfxGtm_Pwm_SyncChannelIndex_15      
} IfxGtm_Pwm_SyncChannelIndex;

typedef enum
{
    IfxGtm_Dtm_ClockSource_systemClock,  
    IfxGtm_Dtm_ClockSource_cmuClock0,    
    IfxGtm_Dtm_ClockSource_cmuClock1,    
    IfxGtm_Dtm_ClockSource_cmuClock2     
} IfxGtm_Dtm_ClockSource;

typedef struct
{
    float32 rising;        
    float32 falling;       
} IfxGtm_Pwm_DeadTime;

typedef struct
{
    IfxGtm_Pwm_DeadTime deadTime;       
} IfxGtm_Pwm_DtmConfig;

typedef struct
{
    IfxGtm_IrqMode      mode;              
    IfxSrc_Tos          isrProvider;       
    Ifx_Priority        priority;          
    IfxGtm_Pwm_callBack periodEvent;       
    IfxGtm_Pwm_callBack dutyEvent;         
} IfxGtm_Pwm_InterruptConfig;

typedef struct
{
    IfxGtm_Pwm_ToutMap *pin;                        
    IfxGtm_Pwm_ToutMap *complementaryPin;           
    Ifx_ActiveState     polarity;                   
    Ifx_ActiveState     complementaryPolarity;      
    IfxPort_OutputMode  outputMode;                 
    IfxPort_PadDriver   padDriver;                  
} IfxGtm_Pwm_OutputConfig;

typedef struct
{
    volatile uint32 *SR0;              
    volatile uint32 *SR1;              
    volatile uint32 *CM0;              
    volatile uint32 *CM1;              
    volatile uint32 *CN0;              
    volatile uint32 *CTRL;             
    volatile uint32 *GLB_CTRL;         
    volatile uint32 *IRQ_NOTIFY;       
    volatile uint32 *DTV;              
} IfxGtm_Pwm_ChannelRegisters;

typedef struct
{
    IfxGtm_Pwm_ChannelRegisters registers;         
    uint32                      upenMask;          
    IfxGtm_Pwm_callBack         periodEvent;       
    IfxGtm_Pwm_callBack         dutyEvent;         
    IfxGtm_Pwm_SubModule_Ch     timerCh;           
    uint32                      phaseTicks;        
    uint32                      dutyTicks;         
} IfxGtm_Pwm_Channel;

typedef struct
{
    IfxGtm_Pwm_SubModule_Ch     timerCh;         
    float32                     phase;           
    float32                     duty;            
    IfxGtm_Pwm_DtmConfig       *dtm;             
    IfxGtm_Pwm_OutputConfig    *output;          
    IfxGtm_Trig_MscOut         *mscOut;          
    IfxGtm_Pwm_InterruptConfig *interrupt;       
} IfxGtm_Pwm_ChannelConfig;

typedef struct
{
    Ifx_GTM_ATOM *ATOM;       
    Ifx_GTM_TOM  *TOM;        
    Ifx_GTM_CDTM *CDTM;       
} IfxGtm_Pwm_ClusterSFR;

typedef struct
{
    volatile uint32 *reg0;                
    volatile uint32 *reg1;                
    uint32           upenMask0;           
    uint32           upenMask1;           
    volatile uint32 *endisCtrlReg0;       
    volatile uint32 *endisCtrlReg1;       
} IfxGtm_Pwm_GlobalControl;

typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Pwm_ClockSource;

typedef struct
{
    Ifx_GTM                 *gtmSFR;                  
    IfxGtm_Pwm_ClusterSFR    clusterSFR;              
    IfxGtm_Cluster           cluster;                 
    IfxGtm_Pwm_SubModule     subModule;               
    IfxGtm_Pwm_Alignment     alignment;               
    uint8                    numChannels;             
    IfxGtm_Pwm_Channel      *channels;                
    IfxGtm_Pwm_GlobalControl globalControl;           
    float32                  sourceFrequency;         
    float32                  dtmFrequency;            
    float32                  frequency;               
    uint32                   periodTicks;             
    IfxGtm_Pwm_ClockSource   clockSource;             
    IfxGtm_Dtm_ClockSource   dtmClockSource;          
    boolean                  syncUpdateEnabled;       
    IfxGtm_Pwm_State         state;                   
} IfxGtm_Pwm;

typedef struct
{
    Ifx_GTM                  *gtmSFR;                 
    IfxGtm_Cluster            cluster;                
    IfxGtm_Pwm_SubModule      subModule;              
    IfxGtm_Pwm_Alignment      alignment;              
    uint8                     numChannels;            
    IfxGtm_Pwm_ChannelConfig *channels;               
    float32                   frequency;              
    IfxGtm_Pwm_ClockSource    clockSource;            
    IfxGtm_Dtm_ClockSource    dtmClockSource;         
    boolean                   syncUpdateEnabled;      
    boolean                   syncStart;              
} IfxGtm_Pwm_Config;

typedef struct
{
    IfxGtm_Pwm_ToutMap *outputPin;       
    IfxPort_OutputMode  outputMode;      
    IfxPort_PadDriver   padDriver;       
} IfxGtm_Pwm_Pin;

/* API declarations (subset required by module/tests) */
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);

#endif /* IFXGTM_PWM_H */
