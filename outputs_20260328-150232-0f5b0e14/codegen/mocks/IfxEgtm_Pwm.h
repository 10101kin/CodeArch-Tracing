/* Mock of IfxEgtm_Pwm.h */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/*----------------------------------------------------*/
/* Forward/auxiliary types used by PWM                */
/*----------------------------------------------------*/
/* Simple stub maps referenced by ToutMap union */
typedef struct { uint8 dummy; } IfxEgtm_Atom_ToutMap;
typedef struct { uint8 dummy; } IfxEgtm_Tom_ToutMap;
typedef struct { uint8 dummy; } IfxEgtm_Hrpwm_Out;

/* Callback type used in multiple PWM structs */
typedef void (*IfxEgtm_Pwm_callBack)(void *arg);

/*----------------------------------------------------*/
/* Enums (must appear before structs that use them)    */
/*----------------------------------------------------*/
typedef enum
{
    IfxEgtm_Pwm_Alignment_edge   = 0,
    IfxEgtm_Pwm_Alignment_center = 1,
    /* Backward-compatible alias to fix prior build error */
    IfxEgtm_Pwm_Alignment_centerAligned = IfxEgtm_Pwm_Alignment_center
} IfxEgtm_Pwm_Alignment;

typedef enum
{
    IfxEgtm_Pwm_ChannelState_running = 0
} IfxEgtm_Pwm_ChannelState;

typedef enum
{
    IfxEgtm_Pwm_ResetEvent_onCm0     = 0,
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1
} IfxEgtm_Pwm_ResetEvent;

typedef enum
{
    IfxEgtm_Pwm_State_unknown = -1,
    IfxEgtm_Pwm_State_init    = 0,
    IfxEgtm_Pwm_State_run     = 1,
    IfxEgtm_Pwm_State_stopped = 2,
    IfxEgtm_Pwm_State_error   = 3
} IfxEgtm_Pwm_State;

typedef enum
{
    IfxEgtm_Pwm_SubModule_atom = 0,
    IfxEgtm_Pwm_SubModule_tom  = 1
} IfxEgtm_Pwm_SubModule;

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

/*----------------------------------------------------*/
/* Structs/Unions                                      */
/*----------------------------------------------------*/
typedef struct
{
    float32 rising;
    float32 falling;
} IfxEgtm_Pwm_DeadTime;

typedef enum { IfxEgtm_Dtm_SignalLevel_low = 0, IfxEgtm_Dtm_SignalLevel_high = 1 } IfxEgtm_Dtm_SignalLevel;
typedef enum { IfxEgtm_Dtm_ShutoffInput_0 = 0, IfxEgtm_Dtm_ShutoffInput_1 = 1 } IfxEgtm_Dtm_ShutoffInput;

typedef struct
{
    IfxEgtm_Dtm_ShutoffInput inputSignal;
    boolean                  invertInputSignal;
    IfxEgtm_Dtm_SignalLevel  offState;
    IfxEgtm_Dtm_SignalLevel  complementaryOffState;
} IfxEgtm_Pwm_FastShutoffConfig;

typedef union
{
    IfxEgtm_Atom_ToutMap atom;
    IfxEgtm_Tom_ToutMap  tom;
#if 1
    IfxEgtm_Hrpwm_Out    hrpwm;
#endif
} IfxEgtm_Pwm_ToutMap;

typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;
    /* Fix prior build error: add minPulse field */
    float32                        minPulse;
} IfxEgtm_Pwm_DtmConfig;

typedef struct
{
    IfxEgtm_IrqMode      mode;
    IfxSrc_Tos           isrProvider;
    Ifx_Priority         priority;
    uint32               vmId; /* IfxSrc_VmId simplified */
    IfxEgtm_Pwm_callBack periodEvent;
    IfxEgtm_Pwm_callBack dutyEvent;
} IfxEgtm_Pwm_InterruptConfig;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *pin;
    IfxEgtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState      polarity;
    Ifx_ActiveState      complementaryPolarity;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
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

/* ClockSource union must use uint32 fields (per rules) */
typedef union
{
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* Optional convenience macro for clock source symbol (fix prior error) */
#ifndef IfxEgtm_Pwm_ClockSource_cmuFxclk0
#define IfxEgtm_Pwm_ClockSource_cmuFxclk0 ((uint32)IfxEgtm_Cmu_Fxclk_0)
#endif

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

typedef struct
{
    volatile Ifx_UReg_32Bit *reg0;
    volatile Ifx_UReg_32Bit *reg1;
    uint32                   upenMask0;
    uint32                   upenMask1;
    volatile Ifx_UReg_32Bit *endisCtrlReg0;
    volatile Ifx_UReg_32Bit *endisCtrlReg1;
} IfxEgtm_Pwm_GlobalControl;

typedef struct
{
    Ifx_EGTM                  *egtmSFR;
    uint32                     cluster;        /* IfxEgtm_Cluster simplified */
    IfxEgtm_Pwm_SubModule      subModule;
    IfxEgtm_Pwm_Alignment      alignment;
    uint8                      numChannels;
    struct IfxEgtm_Pwm_ChannelConfig *channels;
    float32                    frequency;
    IfxEgtm_Pwm_ClockSource    clockSource;
    uint32                     dtmClockSource; /* IfxEgtm_Dtm_ClockSource simplified */
    boolean                    syncUpdateEnabled;
    boolean                    syncStart;
} IfxEgtm_Pwm_Config;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_Pin;

typedef struct IfxEgtm_Pwm_ChannelConfig
{
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    float32                      phase;
    float32                      duty;
    IfxEgtm_Pwm_DtmConfig       *dtm;
    IfxEgtm_Pwm_OutputConfig    *output;
    void                        *mscOut;    /* pointer to MSC config (void*) */
    IfxEgtm_Pwm_InterruptConfig *interrupt;
} IfxEgtm_Pwm_ChannelConfig;

typedef struct
{
    Ifx_EGTM                 *egtmSFR;
    struct Ifx_EGTM_CLS      *clusterSFR;  /* simple forward stub */
    uint32                     cluster;
    IfxEgtm_Pwm_SubModule     subModule;
    IfxEgtm_Pwm_Alignment     alignment;
    uint8                      numChannels;
    IfxEgtm_Pwm_Channel      *channels;
    IfxEgtm_Pwm_GlobalControl globalControl;
    float32                   sourceFrequency;
    float32                   dtmFrequency;
    float32                   frequency;
    uint32                    periodTicks;
    IfxEgtm_Pwm_ClockSource   clockSource;
    uint32                    dtmClockSource;
    boolean                   syncUpdateEnabled;
    IfxEgtm_Pwm_State         state;
} IfxEgtm_Pwm;

/*----------------------------------------------------*/
/* Function declarations                               */
/*----------------------------------------------------*/
/* CPU/IRQ helpers */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), sint32 vectabNum, Ifx_Priority priority);
void IfxCpu_enableInterrupts(void);

/* Primary PWM APIs (subset + extended list as required) */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *cfg);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_setChannelPolarity(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, Ifx_ActiveState pol);
void IfxEgtm_Pwm_updateFrequency(IfxEgtm_Pwm *pwm, float32 freq);
void IfxEgtm_Pwm_updateFrequencyImmediate(IfxEgtm_Pwm *pwm, float32 freq);
void IfxEgtm_Pwm_updateSyncedGroupsFrequency(IfxEgtm_Pwm *pwm, uint32 groupMask, float32 freq);
void IfxEgtm_Pwm_updateChannelPhase(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxEgtm_Pwm_updateChannelPhaseImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxEgtm_Pwm_updateChannelsPhase(IfxEgtm_Pwm *pwm, float32 *phases);
void IfxEgtm_Pwm_updateChannelsPhaseImmediate(IfxEgtm_Pwm *pwm, float32 *phases);
void IfxEgtm_Pwm_updateChannelDuty(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxEgtm_Pwm_updateChannelDutyImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *duties);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_updateChannelDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall);
void IfxEgtm_Pwm_updateChannelDeadTimeImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall);
void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, float32 *rise, float32 *fall);
void IfxEgtm_Pwm_updateChannelPulse(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 pulse);
void IfxEgtm_Pwm_updateChannelPulseImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 pulse);
void IfxEgtm_Pwm_updateChannelsPulse(IfxEgtm_Pwm *pwm, float32 *pulses);
void IfxEgtm_Pwm_updateChannelsPulseImmediate(IfxEgtm_Pwm *pwm, float32 *pulses);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm, uint32 chMask);
void IfxEgtm_Pwm_stopSyncedChannels(IfxEgtm_Pwm *pwm, uint32 chMask);
void IfxEgtm_Pwm_startSyncedGroups(IfxEgtm_Pwm *pwm, uint32 groupMask);
void IfxEgtm_Pwm_stopSyncedGroups(IfxEgtm_Pwm *pwm, uint32 groupMask);
void IfxEgtm_Pwm_stopChannelOutputs(IfxEgtm_Pwm *pwm, uint32 chMask);
void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm, uint32 chMask);
IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch);
void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm *pwm);

#endif /* IFXEGTM_PWM_H */
