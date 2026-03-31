#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Callback type */
typedef void (*IfxEgtm_Pwm_callBack)(void *data);

/* Verified type definitions (and supporting types) */
/* Supporting pin map structs for ToutMap union */
typedef struct { uint32 reserved; } IfxEgtm_Atom_ToutMap;
typedef struct { uint32 reserved; } IfxEgtm_Tom_ToutMap;
typedef struct { uint32 reserved; } IfxEgtm_Hrpwm_Out;

/* Alignment */
typedef enum
{
    IfxEgtm_Pwm_Alignment_edge   = 0, 
    IfxEgtm_Pwm_Alignment_center = 1  
} IfxEgtm_Pwm_Alignment;

/* SubModule */
typedef enum
{
    IfxEgtm_Pwm_SubModule_atom = 0,  
    IfxEgtm_Pwm_SubModule_tom  = 1   
} IfxEgtm_Pwm_SubModule;

/* SubModule Channel */
typedef enum
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
    IfxEgtm_Pwm_SubModule_Ch_15 = 15   
} IfxEgtm_Pwm_SubModule_Ch;

/* Channel State */
typedef enum
{
    IfxEgtm_Pwm_ChannelState_running = 0,  
    IfxEgtm_Pwm_ChannelState_stopped       
} IfxEgtm_Pwm_ChannelState;

/* Reset Event */
typedef enum
{
    IfxEgtm_Pwm_ResetEvent_onCm0     = 0, 
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1  
} IfxEgtm_Pwm_ResetEvent;

/* State */
typedef enum
{
    IfxEgtm_Pwm_State_unknown = -1,  
    IfxEgtm_Pwm_State_init    = 0,   
    IfxEgtm_Pwm_State_run     = 1,   
    IfxEgtm_Pwm_State_stopped = 2,   
    IfxEgtm_Pwm_State_error   = 3    
} IfxEgtm_Pwm_State;

/* SyncChannelIndex */
typedef enum
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
} IfxEgtm_Pwm_SyncChannelIndex;

/* DTM Clock Source */
typedef enum
{
    IfxEgtm_Dtm_ClockSource_systemClock,  
    IfxEgtm_Dtm_ClockSource_cmuClock0,    
    IfxEgtm_Dtm_ClockSource_cmuClock1,    
    IfxEgtm_Dtm_ClockSource_cmuClock2     
} IfxEgtm_Dtm_ClockSource;

/* DeadTime */
typedef struct
{
    float32 rising;        
    float32 falling;       
} IfxEgtm_Pwm_DeadTime;

/* DTM Config */
typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;          
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;       
} IfxEgtm_Pwm_DtmConfig;

/* ToutMap (used in OutputConfig) */
typedef union
{
    IfxEgtm_Atom_ToutMap atom;        
    IfxEgtm_Tom_ToutMap  tom;         
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE	
    IfxEgtm_Hrpwm_Out    hrpwm;       
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
} IfxEgtm_Pwm_ToutMap;

/* Interrupt Config */
typedef struct
{
    IfxEgtm_IrqMode      mode;              
    IfxSrc_Tos           isrProvider;       
    Ifx_Priority         priority;          
    IfxSrc_VmId          vmId;              
    IfxEgtm_Pwm_callBack periodEvent;       
    IfxEgtm_Pwm_callBack dutyEvent;         
} IfxEgtm_Pwm_InterruptConfig;

/* Output Config */
typedef struct
{
    IfxEgtm_Pwm_ToutMap *pin;                        
    IfxEgtm_Pwm_ToutMap *complementaryPin;           
    Ifx_ActiveState      polarity;                   
    Ifx_ActiveState      complementaryPolarity;      
    IfxPort_OutputMode   outputMode;                 
    IfxPort_PadDriver    padDriver;                  
} IfxEgtm_Pwm_OutputConfig;

/* Channel Registers */
typedef struct
{
    volatile Ifx_UReg_32Bit *SR0;              
    volatile Ifx_UReg_32Bit *SR1;              
    volatile Ifx_UReg_32Bit *CM0;              
    volatile Ifx_UReg_32Bit *CM1;              
    volatile Ifx_UReg_32Bit *CN0;              
    volatile Ifx_UReg_32Bit *CTRL;             
    volatile Ifx_UReg_32Bit *GLB_CTRL;         
    volatile Ifx_UReg_32Bit *IRQ_NOTIFY;       
    volatile Ifx_UReg_32Bit *DTV;              
    volatile Ifx_UReg_32Bit *DTV_SR;           
} IfxEgtm_Pwm_ChannelRegisters;

/* Channel */
typedef struct
{
    IfxEgtm_Pwm_ChannelRegisters registers;         
    uint32                       upenMask;          
    IfxEgtm_Pwm_callBack         periodEvent;       
    IfxEgtm_Pwm_callBack         dutyEvent;         
    IfxEgtm_Pwm_SubModule_Ch     timerCh;           
    uint32                       phaseTicks;        
    uint32                       dutyTicks;         
} IfxEgtm_Pwm_Channel;

/* ChannelConfig */
typedef struct
{
    IfxEgtm_Pwm_SubModule_Ch     timerCh;         
    float32                      phase;           
    float32                      duty;            
    IfxEgtm_Pwm_DtmConfig       *dtm;             
    IfxEgtm_Pwm_OutputConfig    *output;          
    IfxEgtm_MscOut              *mscOut;          
    IfxEgtm_Pwm_InterruptConfig *interrupt;       
} IfxEgtm_Pwm_ChannelConfig;

/* GlobalControl */
typedef struct
{
    volatile Ifx_UReg_32Bit *reg0;                /**< \brief ATOM: points to AGC_GLB_CTRL.
                                                   * TOM: If channels span 2 TGCs then points to TGC0_GLB_CTRL else to the TGC being used TGCx_GLB_CTRL */
    volatile Ifx_UReg_32Bit *reg1;                
    uint32                   upenMask0;           
    uint32                   upenMask1;           
    volatile Ifx_UReg_32Bit *endisCtrlReg0;       /**< \brief ATOM: points to AGC_ENDIS_CTRL.
                                                   * TOM: If channels span 2 TGCs then points to TGC0_ENDIS_CTRL else to the TGC being used TGCx_GLB_CTRL */
    volatile Ifx_UReg_32Bit *endisCtrlReg1;       
} IfxEgtm_Pwm_GlobalControl;

/* ClockSource union (uint32 fields per rules) */
typedef union
{
    IfxEgtm_Cmu_Clk   atom;       
    IfxEgtm_Cmu_Fxclk tom;        
} IfxEgtm_Pwm_ClockSource;

/* PWM handle */
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

/* PWM Config */
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
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE	
    boolean                    highResEnable;          
    boolean                    dtmHighResEnable;       
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
    boolean                    syncUpdateEnabled;      
    boolean                    syncStart;              
} IfxEgtm_Pwm_Config;

/* Pin convenience */
typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;        
    IfxPort_OutputMode   outputMode;       
    IfxPort_PadDriver    padDriver;        
} IfxEgtm_Pwm_Pin;

/* Functions required by module/tests */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);

/* IRQ helper prototypes used by examples */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), Ifx_Priority priority);
void IfxCpu_enableInterrupts(void);

#endif /* IFXEGTM_PWM_H */
