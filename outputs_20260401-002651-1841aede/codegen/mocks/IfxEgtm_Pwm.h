#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

typedef enum {
    IfxEgtm_Pwm_Alignment_edge,
    IfxEgtm_Pwm_Alignment_center
} IfxEgtm_Pwm_Alignment;

typedef enum {
    IfxEgtm_Pwm_ChannelState_running
} IfxEgtm_Pwm_ChannelState;

typedef enum {
    IfxEgtm_Pwm_ResetEvent_onCm0,
    IfxEgtm_Pwm_ResetEvent_onTrigger
} IfxEgtm_Pwm_ResetEvent;

typedef enum {
    IfxEgtm_Pwm_State_unknown,
    IfxEgtm_Pwm_State_init,
    IfxEgtm_Pwm_State_run,
    IfxEgtm_Pwm_State_stopped,
    IfxEgtm_Pwm_State_error
} IfxEgtm_Pwm_State;

typedef enum {
    IfxEgtm_Pwm_SubModule_atom,
    IfxEgtm_Pwm_SubModule_tom
} IfxEgtm_Pwm_SubModule;

typedef enum {
    IfxEgtm_Pwm_SubModule_Ch_0,
    IfxEgtm_Pwm_SubModule_Ch_1,
    IfxEgtm_Pwm_SubModule_Ch_2,
    IfxEgtm_Pwm_SubModule_Ch_3,
    IfxEgtm_Pwm_SubModule_Ch_4,
    IfxEgtm_Pwm_SubModule_Ch_5,
    IfxEgtm_Pwm_SubModule_Ch_6,
    IfxEgtm_Pwm_SubModule_Ch_7,
    IfxEgtm_Pwm_SubModule_Ch_8,
    IfxEgtm_Pwm_SubModule_Ch_9,
    IfxEgtm_Pwm_SubModule_Ch_10,
    IfxEgtm_Pwm_SubModule_Ch_11,
    IfxEgtm_Pwm_SubModule_Ch_12,
    IfxEgtm_Pwm_SubModule_Ch_13,
    IfxEgtm_Pwm_SubModule_Ch_14,
    IfxEgtm_Pwm_SubModule_Ch_15
} IfxEgtm_Pwm_SubModule_Ch;

typedef enum {
    IfxEgtm_Pwm_SyncChannelIndex_0,
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
    IfxEgtm_Pwm_SyncChannelIndex_14
} IfxEgtm_Pwm_SyncChannelIndex;

typedef struct
{
    float32 rising;        
    float32 falling;       
} IfxEgtm_Pwm_DeadTime;

typedef struct
{
    IfxEgtm_Dtm_ShutoffInput inputSignal;                 /**< \brief Select input signal to be used as shut off signal */
    boolean                  invertInputSignal;           /**< \brief TRUE: Input signal is inverted */
    IfxEgtm_Dtm_SignalLevel  offState;                    /**< \brief Desired state(High or low) of output 0 when shut-off is active */
    IfxEgtm_Dtm_SignalLevel  complementaryOffState;       /**< \brief Desired state(High or low) of output 1 (*_N) when shut-off is active */
} IfxEgtm_Pwm_FastShutoffConfig;

typedef union
{
    IfxEgtm_Atom_ToutMap atom;        /**< \brief ATOM map */
    IfxEgtm_Tom_ToutMap  tom;         /**< \brief TOM map */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE	
    IfxEgtm_Hrpwm_Out    hrpwm;       /**< \brief HRPWM Connection */
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
} IfxEgtm_Pwm_ToutMap;

typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;          
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;       
} IfxEgtm_Pwm_DtmConfig;

typedef struct
{
    IfxEgtm_IrqMode      mode;              /**< \brief IRQ mode of interrupt. Note: Use IfxEgtm_IrqMode_pulseNotify as default */
    IfxSrc_Tos           isrProvider;       /**< \brief Type of Service for Ccu0/1 interrupt */
    Ifx_Priority         priority;          /**< \brief Priority for Ccu0/1 interrupt */
    IfxSrc_VmId          vmId;              /**< \brief Virtual machine interrupt service provider */
    IfxEgtm_Pwm_callBack periodEvent;       /**< \brief Period interrupt callback function pointer */
    IfxEgtm_Pwm_callBack dutyEvent;         /**< \brief Duty interrupt callback function pointer */
} IfxEgtm_Pwm_InterruptConfig;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *pin;                        /**< \brief Output pin configuration */
    IfxEgtm_Pwm_ToutMap *complementaryPin;           /**< \brief Complementary output pin configuration (_N) */
    Ifx_ActiveState      polarity;                   /**< \brief Active low/high of pin */
    Ifx_ActiveState      complementaryPolarity;      /**< \brief Active low/high of complementary pin */
    IfxPort_OutputMode   outputMode;                 /**< \brief Output mode */
    IfxPort_PadDriver    padDriver;                  /**< \brief Pad driver */
} IfxEgtm_Pwm_OutputConfig;

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

typedef struct
{
    IfxEgtm_Pwm_SubModule_Ch     timerCh;         /**< \brief Channel Index */
    float32                      phase;           /**< \brief Initial phase in radians (range: 0.0 .. 2pi; only for edge aligned sync channels) */
    float32                      duty;            /**< \brief PWM duty in % (range: 0.0 .. 100.0) */
    IfxEgtm_Pwm_DtmConfig       *dtm;             /**< \brief Dead time configuration for this channel */
    IfxEgtm_Pwm_OutputConfig    *output;          /**< \brief Pin connections and polarities for this channel */
    IfxEgtm_MscOut              *mscOut;          /**< \brief MSC configuration for this channel */
    IfxEgtm_Pwm_InterruptConfig *interrupt;       /**< \brief Interrupt configuration for this channel */
} IfxEgtm_Pwm_ChannelConfig;

typedef union
{
    IfxEgtm_Cmu_Clk   atom;       /**< \brief Clock source for ATOM channels */
    IfxEgtm_Cmu_Fxclk tom;        /**< \brief Clock source for TOM channels */
} IfxEgtm_Pwm_ClockSource;

typedef struct
{
    IfxEgtm_Pwm_ChannelRegisters registers;         /**< \brief Contains pointers to frequenctly accessed channel specific registers */
    uint32                       upenMask;          /**< \brief Update enable mask of this channel */
    IfxEgtm_Pwm_callBack         periodEvent;       /**< \brief CCU0 interrupt callback function pointer */
    IfxEgtm_Pwm_callBack         dutyEvent;         /**< \brief CCU1 interrupt callback function pointer */
    IfxEgtm_Pwm_SubModule_Ch     timerCh;           /**< \brief Channel Index */
    uint32                       phaseTicks;        /**< \brief Current phase ticks */
    uint32                       dutyTicks;         /**< \brief Current duty ticks */
} IfxEgtm_Pwm_Channel;

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

typedef struct
{
    Ifx_EGTM                  *egtmSFR;                /**< \brief Pointer to GTM module */
    IfxEgtm_Cluster            cluster;                /**< \brief Index of the CLS object used */
    IfxEgtm_Pwm_SubModule      subModule;              /**< \brief Sub module to be used for PWM */
    IfxEgtm_Pwm_Alignment      alignment;              /**< \brief PWM alignment */
    uint8                      numChannels;            /**< \brief Number of channels (base + sync) to be configured */
    IfxEgtm_Pwm_ChannelConfig *channels;               /**< \brief Pointer to channel configuration */
    float32                    frequency;              /**< \brief Initial PWM frequency */
    IfxEgtm_Pwm_ClockSource    clockSource;            /**< \brief Clock source for Atom/Tom channels */
    IfxEgtm_Dtm_ClockSource    dtmClockSource;         /**< \brief Clock source for DTM channels */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE	
    boolean                    highResEnable;          /**< \brief Enable high res if using Atom */
    boolean                    dtmHighResEnable;       /**< \brief Enable high res in DTM module */
#endif /* #if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE */	
    boolean                    syncUpdateEnabled;      /**< \brief TRUE: Update compare registers from shadow at the end of period */
    boolean                    syncStart;              /**< \brief TRUE: Synchronously start all channels PWM by starting the counters at the end of init */
} IfxEgtm_Pwm_Config;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;        /**< \brief Output pin */
    IfxPort_OutputMode   outputMode;       /**< \brief Output mode */
    IfxPort_PadDriver    padDriver;        /**< \brief Pad driver */
} IfxEgtm_Pwm_Pin;

typedef struct
{
    Ifx_EGTM                 *egtmSFR;                 /**< \brief Pointer to GTM module */
    Ifx_EGTM_CLS             *clusterSFR;              /**< \brief Pointer to cluster SFR */
    IfxEgtm_Cluster           cluster;                 /**< \brief Index of the CLS object used */
    IfxEgtm_Pwm_SubModule     subModule;               /**< \brief Sub module to be used for PWM */
    IfxEgtm_Pwm_Alignment     alignment;               /**< \brief PWM alignment */
    uint8                     numChannels;             /**< \brief Number of channels configured (base + sync) */
    IfxEgtm_Pwm_Channel      *channels;                /**< \brief Stores state of PWM channels (base + sync) */
    IfxEgtm_Pwm_GlobalControl globalControl;           /**< \brief Pointer and mask for GLB_CTRL */
    float32                   sourceFrequency;         /**< \brief Source clock frequency in Hz */
    float32                   dtmFrequency;            /**< \brief DTM clock frequency in Hz */
    float32                   frequency;               /**< \brief Current PWM frequency in Hz */
    uint32                    periodTicks;             /**< \brief Current PWM Period in ticks */
    IfxEgtm_Pwm_ClockSource   clockSource;             /**< \brief Clock source for Atom/Tom channels */
    IfxEgtm_Dtm_ClockSource   dtmClockSource;          /**< \brief Clock source for DTM channels */
    boolean                   syncUpdateEnabled;       /**< \brief TRUE: Update compare registers from shadow at the end of period */
    IfxEgtm_Pwm_State         state;                   /**< \brief Module state */
} IfxEgtm_Pwm;

typedef enum
{
    IfxEgtm_Pwm_Alignment_edge   = 0, /**< \brief Edge aligned PWM */
    IfxEgtm_Pwm_Alignment_center = 1  /**< \brief Symmetric center aligned PWM */
} IfxEgtm_Pwm_Alignment;

typedef enum
{
    IfxEgtm_Pwm_ChannelState_running = 0,  /**< \brief Channel counters are running and output is enabled */
    IfxEgtm_Pwm_ChannelState_stopped       /**< \brief Either channel or it's output is disabled */
} IfxEgtm_Pwm_ChannelState;

typedef enum
{
    IfxEgtm_Pwm_ResetEvent_onCm0     = 0, /**< \brief Reset counter when CN0 = CM0 */
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1  /**< \brief Reset counter when trigger is received */
} IfxEgtm_Pwm_ResetEvent;

typedef enum
{
    IfxEgtm_Pwm_State_unknown = -1,  /**< \brief Unknown state */
    IfxEgtm_Pwm_State_init    = 0,   /**< \brief Module is being initialized */
    IfxEgtm_Pwm_State_run     = 1,   /**< \brief Run state */
    IfxEgtm_Pwm_State_stopped = 2,   /**< \brief Stopped state */
    IfxEgtm_Pwm_State_error   = 3    /**< \brief Error state */
} IfxEgtm_Pwm_State;

typedef enum
{
    IfxEgtm_Pwm_SubModule_atom = 0,  /**< \brief GTM submodule ATOM */
    IfxEgtm_Pwm_SubModule_tom  = 1   /**< \brief GTM submodule TOM */
} IfxEgtm_Pwm_SubModule;

typedef enum
{
    IfxEgtm_Pwm_SubModule_Ch_0  = 0,   /**< \brief ATOM / TOM channel 0 */
    IfxEgtm_Pwm_SubModule_Ch_1  = 1,   /**< \brief ATOM / TOM channel 1 */
    IfxEgtm_Pwm_SubModule_Ch_2  = 2,   /**< \brief ATOM / TOM channel 2 */
    IfxEgtm_Pwm_SubModule_Ch_3  = 3,   /**< \brief ATOM / TOM channel 3 */
    IfxEgtm_Pwm_SubModule_Ch_4  = 4,   /**< \brief ATOM / TOM channel 4 */
    IfxEgtm_Pwm_SubModule_Ch_5  = 5,   /**< \brief ATOM / TOM channel 5 */
    IfxEgtm_Pwm_SubModule_Ch_6  = 6,   /**< \brief ATOM / TOM channel 6 */
    IfxEgtm_Pwm_SubModule_Ch_7  = 7,   /**< \brief ATOM / TOM channel 7 */
    IfxEgtm_Pwm_SubModule_Ch_8  = 8,   /**< \brief TOM channel 8 */
    IfxEgtm_Pwm_SubModule_Ch_9  = 9,   /**< \brief TOM channel 9 */
    IfxEgtm_Pwm_SubModule_Ch_10 = 10,  /**< \brief TOM channel 10 */
    IfxEgtm_Pwm_SubModule_Ch_11 = 11,  /**< \brief TOM channel 11 */
    IfxEgtm_Pwm_SubModule_Ch_12 = 12,  /**< \brief TOM channel 12 */
    IfxEgtm_Pwm_SubModule_Ch_13 = 13,  /**< \brief TOM channel 13 */
    IfxEgtm_Pwm_SubModule_Ch_14 = 14,  /**< \brief TOM channel 14 */
    IfxEgtm_Pwm_SubModule_Ch_15 = 15   /**< \brief TOM channel 15 */
} IfxEgtm_Pwm_SubModule_Ch;

typedef enum
{
    IfxEgtm_Pwm_SyncChannelIndex_0 = 0,  /**< \brief Base Channel */
    IfxEgtm_Pwm_SyncChannelIndex_1,      /**< \brief Sync Channel 0 */
    IfxEgtm_Pwm_SyncChannelIndex_2,      /**< \brief Sync Channel 1 */
    IfxEgtm_Pwm_SyncChannelIndex_3,      /**< \brief Sync Channel 2 */
    IfxEgtm_Pwm_SyncChannelIndex_4,      /**< \brief Sync Channel 3 */
    IfxEgtm_Pwm_SyncChannelIndex_5,      /**< \brief Sync Channel 4 */
    IfxEgtm_Pwm_SyncChannelIndex_6,      /**< \brief Sync Channel 5 */
    IfxEgtm_Pwm_SyncChannelIndex_7,      /**< \brief Sync Channel 6 */
    IfxEgtm_Pwm_SyncChannelIndex_8,      /**< \brief Sync Channel 7 */
    IfxEgtm_Pwm_SyncChannelIndex_9,      /**< \brief Sync Channel 8 */
    IfxEgtm_Pwm_SyncChannelIndex_10,     /**< \brief Sync Channel 9 */
    IfxEgtm_Pwm_SyncChannelIndex_11,     /**< \brief Sync Channel 10 */
    IfxEgtm_Pwm_SyncChannelIndex_12,     /**< \brief Sync Channel 11 */
    IfxEgtm_Pwm_SyncChannelIndex_13,     /**< \brief Sync Channel 12 */
    IfxEgtm_Pwm_SyncChannelIndex_14,     /**< \brief Sync Channel 13 */
    IfxEgtm_Pwm_SyncChannelIndex_15      /**< \brief Sync Channel 14 */
} IfxEgtm_Pwm_SyncChannelIndex;

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm * pwm, float32 * requestDuty);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config * config, Ifx_EGTM * egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm * pwm, IfxEgtm_Pwm_Channel * channels, IfxEgtm_Pwm_Config * config);

#endif /* IFXEGTM_PWM_H */
