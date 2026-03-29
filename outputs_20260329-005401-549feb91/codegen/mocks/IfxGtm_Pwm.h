#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Enums (define BEFORE structs that use them) */
typedef enum { IfxGtm_Pwm_Alignment_edge = 0, IfxGtm_Pwm_Alignment_center = 1 } IfxGtm_Pwm_Alignment;

typedef enum { IfxGtm_Pwm_ChannelState_running = 0, IfxGtm_Pwm_ChannelState_stopped = 1 } IfxGtm_Pwm_ChannelState;

typedef enum { IfxGtm_Pwm_ResetEvent_onCm0 = 0, IfxGtm_Pwm_ResetEvent_onTrigger = 1 } IfxGtm_Pwm_ResetEvent;

typedef enum { IfxGtm_Pwm_State_unknown = -1, IfxGtm_Pwm_State_init = 0, IfxGtm_Pwm_State_run = 1, IfxGtm_Pwm_State_stopped = 2, IfxGtm_Pwm_State_error = 3 } IfxGtm_Pwm_State;

typedef enum { IfxGtm_Pwm_SubModule_atom = 0, IfxGtm_Pwm_SubModule_tom = 1 } IfxGtm_Pwm_SubModule;

typedef enum {
    IfxGtm_Pwm_SubModule_Ch_0 = 0,
    IfxGtm_Pwm_SubModule_Ch_1,
    IfxGtm_Pwm_SubModule_Ch_2,
    IfxGtm_Pwm_SubModule_Ch_3,
    IfxGtm_Pwm_SubModule_Ch_4,
    IfxGtm_Pwm_SubModule_Ch_5,
    IfxGtm_Pwm_SubModule_Ch_6,
    IfxGtm_Pwm_SubModule_Ch_7,
    IfxGtm_Pwm_SubModule_Ch_8,
    IfxGtm_Pwm_SubModule_Ch_9,
    IfxGtm_Pwm_SubModule_Ch_10,
    IfxGtm_Pwm_SubModule_Ch_11,
    IfxGtm_Pwm_SubModule_Ch_12,
    IfxGtm_Pwm_SubModule_Ch_13,
    IfxGtm_Pwm_SubModule_Ch_14,
    IfxGtm_Pwm_SubModule_Ch_15
} IfxGtm_Pwm_SubModule_Ch;

typedef enum {
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

/* Structs / unions */
typedef struct { float32 rising; float32 falling; } IfxGtm_Pwm_DeadTime;

typedef union { /* ATOM/TOM map generalization for mock */ void *atom; void *tom; } IfxGtm_Pwm_ToutMap;

typedef struct {
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

typedef void (*IfxGtm_Pwm_callBack)(void);

typedef struct { IfxGtm_Pwm_DeadTime deadTime; } IfxGtm_Pwm_DtmConfig;

typedef struct {
    IfxGtm_IrqMode      mode;
    IfxSrc_Tos          isrProvider;
    Ifx_Priority        priority;
    IfxGtm_Pwm_callBack periodEvent;
    IfxGtm_Pwm_callBack dutyEvent;
} IfxGtm_Pwm_InterruptConfig;

typedef enum { IfxPort_OutputMode_pushPull = 0, IfxPort_OutputMode_openDrain = 1 } IfxPort_OutputMode; /* safety: also declared in IfxPort.h */

typedef enum { IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0 } IfxPort_PadDriver; /* safety: also declared in IfxPort.h */

typedef struct {
    IfxGtm_Pwm_ToutMap *pin;
    IfxGtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState     polarity;
    Ifx_ActiveState     complementaryPolarity;
    IfxPort_OutputMode  outputMode;
    IfxPort_PadDriver   padDriver;
} IfxGtm_Pwm_OutputConfig;

typedef struct {
    IfxGtm_Pwm_ChannelRegisters registers;
    uint32                      upenMask;
    IfxGtm_Pwm_callBack         periodEvent;
    IfxGtm_Pwm_callBack         dutyEvent;
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    uint32                      phaseTicks;
    uint32                      dutyTicks;
} IfxGtm_Pwm_Channel;

typedef struct {
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    float32                     phase;
    float32                     duty;
    IfxGtm_Pwm_DtmConfig       *dtm;
    IfxGtm_Pwm_OutputConfig    *output;
    void                       *mscOut; /* must exist even if NULL_PTR used */
    IfxGtm_Pwm_InterruptConfig *interrupt;
} IfxGtm_Pwm_ChannelConfig;

/* ClockSource union as uint32 fields per spec to avoid enum-conversion issues */
typedef union { uint32 atom; uint32 tom; } IfxGtm_Pwm_ClockSource;

typedef struct { void *ATOM; void *TOM; void *CDTM; } IfxGtm_Pwm_ClusterSFR;

typedef struct {
    volatile uint32 *reg0;
    volatile uint32 *reg1;
    uint32           upenMask0;
    uint32           upenMask1;
    volatile uint32 *endisCtrlReg0;
    volatile uint32 *endisCtrlReg1;
} IfxGtm_Pwm_GlobalControl;

typedef struct {
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

typedef struct {
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

typedef struct { IfxGtm_Pwm_ToutMap *outputPin; IfxPort_OutputMode  outputMode; IfxPort_PadDriver padDriver; } IfxGtm_Pwm_Pin;

/* Functions - must include at least the ones used by the module */
void IfxCpu_Irq_installInterruptHandler(void (*handler)(void), sint32 vectabNum, Ifx_Priority priority);

/* CMU helpers referenced by PWM init guard */
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);

/* Primary PWM driver functions (required) */
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 requestFrequency);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);

/* Additional PWM API (declared to match template; simple stubs are provided) */
void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 requestFrequency);
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, uint8 channel, Ifx_ActiveState polarity);
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, uint8 channel, float32 phase);
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, uint8 channel, float32 phase);
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, uint8 channel, float32 duty);
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, uint8 channel, float32 duty);
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, uint8 channel, IfxGtm_Pwm_DeadTime *deadTime);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 channelMask);
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 channelMask);
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);
void IfxGtm_Pwm_updateSyncedGroupsFrequency(IfxGtm_Pwm *pwm, float32 requestFrequency);
void IfxGtm_Pwm_updateChannelPulse(IfxGtm_Pwm *pwm, uint8 channel, float32 pulse);
void IfxGtm_Pwm_updateChannelPulseImmediate(IfxGtm_Pwm *pwm, uint8 channel, float32 pulse);
void IfxGtm_Pwm_updateChannelsPhase(IfxGtm_Pwm *pwm, float32 *phase);
void IfxGtm_Pwm_updateChannelsPulse(IfxGtm_Pwm *pwm, float32 *pulse);
void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_DeadTime *deadTime);
void IfxGtm_Pwm_updateChannelsPhaseImmediate(IfxGtm_Pwm *pwm, float32 *phase);
void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *pulse);
void IfxGtm_Pwm_interruptHandler(IfxGtm_Pwm *pwm);
IfxGtm_Pwm_ChannelState IfxGtm_Pwm_getChannelState(IfxGtm_Pwm *pwm, uint8 channel);
void IfxGtm_Pwm_stopChannelOutputs(IfxGtm_Pwm *pwm, uint32 channelMask);
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm, uint32 channelMask);

#endif /* IFXGTM_PWM_H */
