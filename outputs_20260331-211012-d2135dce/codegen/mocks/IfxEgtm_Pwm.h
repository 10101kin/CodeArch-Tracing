/* Mock IfxEgtm_Pwm.h */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Forward/helper types */
typedef void (*IfxEgtm_Pwm_callBack)(void *data);

typedef union
{
    IfxEgtm_Atom_ToutMap atom;        
    IfxEgtm_Tom_ToutMap  tom;         
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE	
    IfxEgtm_Hrpwm_Out    hrpwm;       
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
} IfxEgtm_Pwm_ToutMap;

typedef struct IfxEgtm_Pwm_FastShutoffConfig IfxEgtm_Pwm_FastShutoffConfig; /* forward only */

/* VERIFIED TYPE DEFINITIONS — ORDER MATTERS (copied as required) */

typedef enum
{
    IfxEgtm_Pwm_Alignment_edge   = 0, 
    IfxEgtm_Pwm_Alignment_center = 1  
} IfxEgtm_Pwm_Alignmenttypedef enum
{
    IfxEgtm_Pwm_SubModule_atom = 0,  
    IfxEgtm_Pwm_SubModule_tom  = 1   
} IfxEgtm_Pwm_SubModutypedef enum
{
    IfxEgtm_Pwm_SubModule_Ch_0  = 0,   
    IfxEgtm_Pwm_SubModule_Ch_1  = 1,   
    IfxEgtm_Pwm_SubModule_Ch_2  = 2,   
    IfxEgtm_Pwm_SubModule_Ch_3  = 3,   
    IfxEgtm_Pwm_SubModule_Ch_4  = 4,   
    IfxEgtm_Pwm_SubModule_Ch_5  = 5,   
    IfxEgtm_Pwm_SubModule_Ch_6  = 6,   
    IfxEgtm_Pwm_SubModule_Ch_7  = 7,   
    IfxEgtm_Pwm_SubModule_Ch_8  = 8,   
    IfxEgtm_Pwm_SubModule_Ch_9  = 9,   
    IfxEgtm_Pwm_SubModule_Ch_10 = 10,  
    IfxEgtm_Pwm_SubModule_Ch_11 = 11,  
    IfxEgtm_Pwm_SubModule_Ch_12 = 12,  
    IfxEgtm_Pwm_SubModule_Ch_13 = 13,  
    IfxEgtm_Pwm_SubModule_Ch_14 = 14,  
    IfxEgtm_Pwm_SubModule_typedef enum
{
    IfxEgtm_Pwm_ChannelState_running = 0,  
    IfxEgtm_Pwm_ChannelState_stopped       
} IfxEgtm_Pwm_Channetypedef enum
{
    IfxEgtm_Pwm_ResetEvent_onCm0     = 0, 
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1  
} IfxEgtm_Pwm_ResetEventtypedef enum
{
    IfxEgtm_Pwm_State_unknown = -1,  
    IfxEgtm_Pwm_State_init    = 0,   
    IfxEgtm_Pwm_State_run     = 1,   
    IfxEgtm_Pwm_State_stopped = 2,   
    IfxEgtm_Pwm_State_error   = 3    
} IfxEgtypedef enum
{
    IfxEgtm_Pwm_SyncChannelIndex_0 = 0,  
    IfxEgtm_Pwm_SyncChannelIndex_1,      
    IfxEgtm_Pwm_SyncChannelIndex_2,      
    IfxEgtm_Pwm_SyncChannelIndex_3,      
    IfxEgtm_Pwm_SyncChannelIndex_4,      
    IfxEgtm_Pwm_SyncChannelIndex_5,      
    IfxEgtm_Pwm_SyncChannelIndex_6,      
    IfxEgtm_Pwm_SyncChannelIndex_7,      
    IfxEgtm_Pwm_SyncChannelIndex_8,      
    IfxEgtm_Pwm_SyncChannelIndex_9,      
    IfxEgtm_Pwm_SyncChannelIndex_10,     
    IfxEgtm_Pwm_SyncChannelIndex_11,     
    IfxEgtm_Pwm_SyncChannelIndex_12,     
    IfxEgtm_Pwm_SyncChannelIndex_13,     
    IfxEgtm_Pwm_SyncChannelIndex_14,     
    IfxEgtm_Pwm_SyncChannelIndex_15      
} IfxEgtm_Pwm_SyncChannelIndex;lIndex_14,
    IfxEgtm_Pwm_SyncChannelIndex_15
} IfxEgtm_Pwm_SyncChannelIndex;

typedeftypedef struct
{
    float32 rising;        
    float32 falling;       
} IfxEgtm_typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;          
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;       
} IfxEgtmtypedef struct
{
    IfxEgtm_IrqMode      mode;              
    IfxSrc_Tos           isrProvider;       
    Ifx_Priority         priority;          
    IfxSrc_VmId          vmId;              
    IfxEgtm_Pwm_callBack periodEvent;       
    IfxEgtm_Pwm_typedef struct
{
    IfxEgtm_Pwm_ToutMap *pin;                        
    IfxEgtm_Pwm_ToutMap *complementaryPin;           
    Ifx_ActiveState      polarity;                   
    Ifx_ActiveState      complementaryPolarity;      
    IfxPort_OutputMode   outputMode;      typedef struct
{
    volatile Ifx_UReg_32Bit *SR0;              
    volatile Ifx_UReg_32Bit *SR1;              
    volatile Ifx_UReg_32Bit *CM0;              
    volatile Ifx_UReg_32Bit *CM1;              
    volatile Ifx_UReg_32Bit *CN0;              
    volatile Ifx_UReg_32Bit *CTRL;             
    volatile Ifx_UReg_32Bit *GLB_CTRL;         
    volatile Ifx_UReg_32Bit *IRQ_NOTIFY;       
    vtypedef struct
{
    IfxEgtm_Pwm_ChannelRegisters registers;         
    uint32                       upenMask;          
    IfxEgtm_Pwm_callBack         periodEvent;       
    IfxEgtm_Pwm_callBack         dutyEvent;         
    IfxEgtm_Pwm_SubModule_Ch     timerCh;           
    uint32                       phaseTicks;        
    uint32    typedef struct
{
    IfxEgtm_Pwm_SubModule_Ch     timerCh;         
    float32                      phase;           
    float32                      duty;            
    IfxEgtm_Pwm_DtmConfig       *dtm;             
    IfxEgtm_Pwm_OutputConfig    *output;          
    IfxEgtm_MscOut              *mscOut;          
    IfxEgtm_Pwm_InterruptConfig *interrupt;       
} IfxEgtm_Pwm_ChannelConfig;Egtm_Pwm_callBack         periodEvent;
    Iftypedef struct
{
    volatile Ifx_UReg_32Bit *reg0;                /**< \brief ATOM: points to AGC_GLB_CTRL.
                                                   * TOM: If channels span 2 TGCs then points to TGC0_GLB_CTRL else to the TGC being used TGCx_GLB_CTRL */
    volatile Ifx_URegtypedef union
{
    IfxEgtm_Cmu_Clk   atom;       
    IfxEgtm_Cmu_Fxclk tom;typedef struct
{
    Ifx_EGTM                 *egtmSFR;                 
    Ifx_EGTM_CLS             *clusterSFR;              
    IfxEgtm_Cluster           cluster;                 
    IfxEgtm_Pwm_SubModule     subModule;               
    IfxEgtm_Pwm_Alignment     alignment;               
    uint8                     numChannels;             
    IfxEgtm_Pwm_Channel      *channels;                
    IfxEgtm_Pwm_GlobalControl globalControl;           
    float32                   sourceFrequency;         
    float32                   dtmFrequency;            
    float32                   frequency;               
    uint32                    periodTicks;             
    IfxEgtm_Pwm_ClockSoutypedef struct
{
    Ifx_EGTM                  *egtmSFR;                
    IfxEgtm_Cluster            cluster;                
    IfxEgtm_Pwm_SubModule      subModule;              
    IfxEgtm_Pwm_Alignment      alignment;              
    uint8                      numChannels;            
    IfxEgtm_Pwm_ChannelConfig *channels;               
    float32                    frequency;              
    IfxEgtm_Pwm_ClockSource    clockSource;            
    IfxEgtm_Dtm_ClockSource    dtmClockSource;   typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;        
    IfxPort_OutputMode   outputMode;       
    IfxPort_PadDriver    padDriver;        
} IfxEgtm_Pwm_Pin;nable;       
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
    boolean                    syncUpdateEnabled;      
    boolean                    syncStart;              
} IfxEgtm_Pwm_Config;eg_32Bit *endisCtrlReg1;
} IfxEgtm_Pwm_GlobalControl;

typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

typedef struct
{
    Ifx_EGTM                 *egtmSFR;
    Ifx_EGTM_CLS             *clusterSFR;
    IfxEgtm_Cluster           cluster;
    IfxEgtm_Pwm_SubModule     subModule;
    IfxEgtm_Pwm_Alignment     alignment;
    uint8                     numChannels;
    IfxEgtm_Pwm_Channel      *channels;
    IfxEgtm_Pwm_GlobalControl globalControl;
    float32                   sourceFrequency;
    float32                   dtmFrequency;
    float32                   frequency;
    uint32                    periodTicks;
    IfxEgtm_Pwm_ClockSource   clockSource;
    IfxEgtm_Dtm_ClockSource   dtmClockSource;
    boolean                   syncUpdateEnabled;
    IfxEgtm_Pwm_State         state;
} IfxEgtm_Pwm;

typedef struct
{
    Ifx_EGTM                  *egtmSFR;
    IfxEgtm_Cluster            cluster;
    IfxEgtm_Pwm_SubModule      subModule;
    IfxEgtm_Pwm_Alignment      alignment;
    uint8                      numChannels;
    IfxEgtm_Pwm_ChannelConfig *channels;
    float32                    frequency;
    IfxEgtm_Pwm_ClockSource    clockSource;
    IfxEgtm_Dtm_ClockSource    dtmClockSource;
    boolean                    syncUpdateEnabled;
    boolean                    syncStart;
} IfxEgtm_Pwm_Config;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_Pin;

/* Function declarations (subset required by module) */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);

#endif /* IFXEGTM_PWM_H */
