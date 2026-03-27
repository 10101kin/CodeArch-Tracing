/* IfxEgtm_Pwm mock header */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Forward-low complexity types referenced by unions */
typedef struct { uint32 dummy; } IfxEgtm_Atom_ToutMap;
typedef struct { uint32 dummy; } IfxEgtm_Tom_ToutMap;
typedef struct { uint32 dummy; } IfxEgtm_Hrpwm_Out;

/* DTM related enums used by PWM config */
typedef enum {
    IfxEgtm_Dtm_ClockSource_cmuClock0 = 0,
    IfxEgtm_Dtm_ClockSource_cmuClock1 = 1,
    IfxEgtm_Dtm_ClockSource_cmuClock2 = 2
} IfxEgtm_Dtm_ClockSource;

typedef enum {
    IfxEgtm_Dtm_ShutoffInput_0 = 0,
    IfxEgtm_Dtm_ShutoffInput_1 = 1
} IfxEgtm_Dtm_ShutoffInput;

typedef enum {
    IfxEgtm_Dtm_SignalLevel_low  = 0,
    IfxEgtm_Dtm_SignalLevel_high = 1
} IfxEgtm_Dtm_SignalLevel;

/* Enums */
typedef enum {
    IfxEgtm_Pwm_Alignment_edge   = 0,
    IfxEgtm_Pwm_Alignment_center = 1
} IfxEgtm_Pwm_Alignment;

typedef enum {
    IfxEgtm_Pwm_ChannelState_running = 0,
    IfxEgtm_Pwm_ChannelState_stopped
} IfxEgtm_Pwm_ChannelState;

typedef enum {
    IfxEgtm_Pwm_ResetEvent_onCm0     = 0,
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1
} IfxEgtm_Pwm_ResetEvent;

typedef enum {
    IfxEgtm_Pwm_State_unknown = -1,
    IfxEgtm_Pwm_State_init    = 0,
    IfxEgtm_Pwm_State_run     = 1,
    IfxEgtm_Pwm_State_stopped = 2,
    IfxEgtm_Pwm_State_error   = 3
} IfxEgtm_Pwm_State;

typedef enum {
    IfxEgtm_Pwm_SubModule_atom = 0,
    IfxEgtm_Pwm_SubModule_tom  = 1
} IfxEgtm_Pwm_SubModule;

typedef enum {
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

typedef enum {
    IfxEgtm_Pwm_SyncChannelIndex_0  = 0,
    IfxEgtm_Pwm_SyncChannelIndex_1  = 1,
    IfxEgtm_Pwm_SyncChannelIndex_2  = 2,
    IfxEgtm_Pwm_SyncChannelIndex_3  = 3,
    IfxEgtm_Pwm_SyncChannelIndex_4  = 4,
    IfxEgtm_Pwm_SyncChannelIndex_5  = 5,
    IfxEgtm_Pwm_SyncChannelIndex_6  = 6,
    IfxEgtm_Pwm_SyncChannelIndex_7  = 7,
    IfxEgtm_Pwm_SyncChannelIndex_8  = 8,
    IfxEgtm_Pwm_SyncChannelIndex_9  = 9,
    IfxEgtm_Pwm_SyncChannelIndex_10 = 10,
    IfxEgtm_Pwm_SyncChannelIndex_11 = 11,
    IfxEgtm_Pwm_SyncChannelIndex_12 = 12,
    IfxEgtm_Pwm_SyncChannelIndex_13 = 13,
    IfxEgtm_Pwm_SyncChannelIndex_14 = 14,
    IfxEgtm_Pwm_SyncChannelIndex_15 = 15
} IfxEgtm_Pwm_SyncChannelIndex;

/* Simple callback typedef to satisfy references (not used by tests) */
typedef void (*IfxEgtm_Pwm_callBack)(void *);

/* Structs */
typedef struct {
    float32 rising;
    float32 falling;
} IfxEgtm_Pwm_DeadTime;

typedef struct {
    IfxEgtm_Dtm_ShutoffInput inputSignal;
    boolean                  invertInputSignal;
    IfxEgtm_Dtm_SignalLevel  offState;
    IfxEgtm_Dtm_SignalLevel  complementaryOffState;
} IfxEgtm_Pwm_FastShutoffConfig;

typedef union {
    IfxEgtm_Atom_ToutMap atom;
    IfxEgtm_Tom_ToutMap  tom;
#if 1
    IfxEgtm_Hrpwm_Out    hrpwm;
#endif
} IfxEgtm_Pwm_ToutMap;

typedef struct {
    IfxEgtm_Pwm_DeadTime           deadTime;
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;
} IfxEgtm_Pwm_DtmConfig;

typedef struct {
    IfxEgtm_IrqMode      mode;
    IfxSrc_Tos           isrProvider;
    Ifx_Priority         priority;
    IfxSrc_VmId          vmId;
    IfxEgtm_Pwm_callBack periodEvent;
    IfxEgtm_Pwm_callBack dutyEvent;
} IfxEgtm_Pwm_InterruptConfig;

typedef struct {
    IfxEgtm_Pwm_ToutMap *pin;
    IfxEgtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState      polarity;
    Ifx_ActiveState      complementaryPolarity;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_OutputConfig;

typedef struct {
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

typedef struct {
    IfxEgtm_Pwm_ChannelRegisters registers;
    uint32                       upenMask;
    IfxEgtm_Pwm_callBack         periodEvent;
    IfxEgtm_Pwm_callBack         dutyEvent;
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    uint32                       phaseTicks;
    uint32                       dutyTicks;
} IfxEgtm_Pwm_Channel;

typedef struct {
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    float32                      phase;
    float32                      duty;
    IfxEgtm_Pwm_DtmConfig       *dtm;
    IfxEgtm_Pwm_OutputConfig    *output;
    IfxEgtm_MscOut              *mscOut;
    IfxEgtm_Pwm_InterruptConfig *interrupt;
} IfxEgtm_Pwm_ChannelConfig;

typedef struct {
    volatile Ifx_UReg_32Bit *reg0;
    volatile Ifx_UReg_32Bit *reg1;
    uint32                   upenMask0;
    uint32                   upenMask1;
    volatile Ifx_UReg_32Bit *endisCtrlReg0;
    volatile Ifx_UReg_32Bit *endisCtrlReg1;
} IfxEgtm_Pwm_GlobalControl;

/* Clock source union — use uint32 fields to be enum-agnostic */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

typedef struct {
    Ifx_EGTM               *egtmSFR;
    IfxEgtm_Cluster         cluster;
    IfxEgtm_Pwm_SubModule   subModule;
    IfxEgtm_Pwm_Alignment   alignment;
    uint8                   numChannels;
    IfxEgtm_Pwm_ChannelConfig *channels;
    float32                 frequency;
    IfxEgtm_Pwm_ClockSource clockSource;
    IfxEgtm_Dtm_ClockSource dtmClockSource;
    boolean                 syncUpdateEnabled;
    boolean                 syncStart;
} IfxEgtm_Pwm_Config;

typedef struct {
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

/* Functions used by this module */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);

/* Provide compatibility macro to fix 'channelConfig' access in existing tests */
#ifndef channelConfig
#define channelConfig channels
#endif

/* Pin map externs required by tests (TC4xx EGTM ATOM naming) */
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT33_P33_11_OUT;

#endif /* IFXEGTM_PWM_H */
