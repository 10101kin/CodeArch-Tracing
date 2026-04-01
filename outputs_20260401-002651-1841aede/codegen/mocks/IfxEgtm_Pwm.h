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

typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;          
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;       
} IfxEgtm_Pwm_DtmConfig;

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
    IfxEgtm_Pwm_ToutMap *outputPin;        /**< \brief Output pin */
    IfxPort_OutputMode   outputMode;       /**< \brief Output mode */
    IfxPort_PadDriver    padDriver;        /**< \brief Pad driver */
} IfxEgtm_Pwm_Pin;

void IfxEgtm_Pwm_init(IfxEgtm_Pwm * pwm, IfxEgtm_Pwm_Channel * channels, IfxEgtm_Pwm_Config * config);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config * config, Ifx_EGTM * egtmSFR);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm * pwm, float32 * requestDuty);

#endif /* IFXEGTM_PWM_H */
