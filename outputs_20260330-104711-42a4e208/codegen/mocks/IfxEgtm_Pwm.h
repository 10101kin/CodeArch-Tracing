/* IfxEgtm_Pwm mock */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* VERIFIED TYPE DEFINITIONS — emit in dependency order */

typedef enum
{
    IfxEgtm_Pwm_Alignment_edge   = 0,
    IfxEgtm_Pwm_Alignment_center = 1
} IfxEgtm_Pwm_Alignment;

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
    IfxEgtm_Pwm_ChannelState_running = 0,
    IfxEgtm_Pwm_ChannelState_stopped
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

typedef enum
{
    IfxEgtm_Dtm_ClockSource_systemClock,
    IfxEgtm_Dtm_ClockSource_cmuClock0,
    IfxEgtm_Dtm_ClockSource_cmuClock1,
    IfxEgtm_Dtm_ClockSource_cmuClock2
} IfxEgtm_Dtm_ClockSource;

typedef void (*IfxEgtm_Pwm_callBack)(void *data);

/* Simplified ToutMap type for mocks (single-owner here) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ToutMap;

/* Dead time and related configs */
typedef struct
{
    float32 rising;
    float32 falling;
} IfxEgtm_Pwm_DeadTime;

typedef struct IfxEgtm_Pwm_FastShutoffConfig
{
    uint32 dummy; /* placeholder for shutoff config */
} IfxEgtm_Pwm_FastShutoffConfig;

typedef struct
{
    IfxEgtm_Pwm_DeadTime           deadTime;
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;
} IfxEgtm_Pwm_DtmConfig;

typedef struct
{
    IfxEgtm_IrqMode      mode;
    IfxSrc_Tos           isrProvider;
    Ifx_Priority         priority;
    IfxSrc_VmId          vmId;
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
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    float32                      phase;
    float32                      duty;
    IfxEgtm_Pwm_DtmConfig       *dtm;
    IfxEgtm_Pwm_OutputConfig    *output;
    IfxEgtm_MscOut              *mscOut;
    IfxEgtm_Pwm_InterruptConfig *interrupt;
} IfxEgtm_Pwm_ChannelConfig;

typedef struct
{
    volatile Ifx_UReg_32Bit *reg0;
    volatile Ifx_UReg_32Bit *reg1;
    uint32                   upenMask0;
    uint32                   upenMask1;
    volatile Ifx_UReg_32Bit *endisCtrlReg0;
    volatile Ifx_UReg_32Bit *endisCtrlReg1;
} IfxEgtm_Pwm_GlobalControl;

/* Clock source union: uint32 fields to avoid enum-conversion warnings */
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
    boolean                    highResEnable;   /* present for compatibility */
    boolean                    dtmHighResEnable;/* present for compatibility */
    boolean                    syncUpdateEnabled;
    boolean                    syncStart;
} IfxEgtm_Pwm_Config;

typedef struct
{
    IfxEgtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_Pin;

/* Function declarations (subset sufficient for mocks + required ones) */
void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), int priority);
void IfxCpu_enableInterrupts(void);

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);

/* Commonly used additional PWM APIs (declared for link completeness) */
void IfxEgtm_Pwm_updateFrequency(IfxEgtm_Pwm *pwm, float32 frequency);
void IfxEgtm_Pwm_updateFrequencyImmediate(IfxEgtm_Pwm *pwm, float32 frequency);
void IfxEgtm_Pwm_updateChannelDuty(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxEgtm_Pwm_updateChannelDutyImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxEgtm_Pwm_updateChannelPhase(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxEgtm_Pwm_updateChannelPhaseImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm, uint32 mask);
void IfxEgtm_Pwm_stopSyncedChannels(IfxEgtm_Pwm *pwm, uint32 mask);
void IfxEgtm_Pwm_startSyncedGroups(IfxEgtm_Pwm *pwm, uint32 mask);
void IfxEgtm_Pwm_stopSyncedGroups(IfxEgtm_Pwm *pwm, uint32 mask);
void IfxEgtm_Pwm_setChannelPolarity(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, Ifx_ActiveState pol);
void IfxEgtm_Pwm_updateChannelDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall);
void IfxEgtm_Pwm_updateChannelDeadTimeImmediate(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch, float32 rise, float32 fall);
void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm *pwm);
IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch ch);
void IfxEgtm_Pwm_stopChannelOutputs(IfxEgtm_Pwm *pwm, uint32 mask);
void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm, uint32 mask);

/* Pin symbol externs (to fix prior build conflict, type is IfxEgtm_Pwm_ToutMap) */
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT;

#endif /* IFXEGTM_PWM_H */
