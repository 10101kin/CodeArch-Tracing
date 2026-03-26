/* IfxGtm_Pwm types + functions */
#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"

/* Callback type used by PWM driver */
typedef void (*IfxGtm_Pwm_callBack)(void *);

/* Dead-time */
typedef struct {
    float32 rising;  /* Relative rise time */
    float32 falling; /* Relative fall time */
} IfxGtm_Pwm_DeadTime;

/* Underlying ToutMaps (mock) */
typedef struct { uint8 dummy; } IfxGtm_Atom_ToutMap;
typedef struct { uint8 dummy; } IfxGtm_Tom_ToutMap;

/* Pwm ToutMap union */
typedef union {
    IfxGtm_Atom_ToutMap atom; /* ATOM map */
    IfxGtm_Tom_ToutMap  tom;  /* TOM map  */
} IfxGtm_Pwm_ToutMap;

/* Channel SFR pointer aggregates */
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

/* DTM configuration */
typedef struct {
    IfxGtm_Pwm_DeadTime deadTime;
} IfxGtm_Pwm_DtmConfig;

/* Interrupt configuration */
typedef struct {
    IfxGtm_IrqMode      mode;
    IfxSrc_Tos          isrProvider;
    Ifx_Priority        priority;
    IfxGtm_Pwm_callBack periodEvent;
    IfxGtm_Pwm_callBack dutyEvent;
} IfxGtm_Pwm_InterruptConfig;

/* Output pin configuration */
typedef struct {
    IfxGtm_Pwm_ToutMap *pin;
    IfxGtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState     polarity;
    Ifx_ActiveState     complementaryPolarity;
    IfxPort_OutputMode  outputMode;
    IfxPort_PadDriver   padDriver;
} IfxGtm_Pwm_OutputConfig;

/* Submodule and channel enums */
typedef enum {
    IfxGtm_Pwm_SubModule_atom = 0,
    IfxGtm_Pwm_SubModule_tom  = 1
} IfxGtm_Pwm_SubModule;

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
    IfxGtm_Pwm_SubModule_Ch_9
} IfxGtm_Pwm_SubModule_Ch;

/* Channel state */
typedef enum {
    IfxGtm_Pwm_ChannelState_running = 0,
    IfxGtm_Pwm_ChannelState_stopped
} IfxGtm_Pwm_ChannelState;

/* Reset event */
typedef enum {
    IfxGtm_Pwm_ResetEvent_onCm0 = 0,
    IfxGtm_Pwm_ResetEvent_onTrigger = 1
} IfxGtm_Pwm_ResetEvent;

/* Module state */
typedef enum {
    IfxGtm_Pwm_State_unknown = -1,
    IfxGtm_Pwm_State_init    = 0,
    IfxGtm_Pwm_State_run     = 1,
    IfxGtm_Pwm_State_stopped = 2,
    IfxGtm_Pwm_State_error   = 3
} IfxGtm_Pwm_State;

/* Alignment */
typedef enum {
    IfxGtm_Pwm_Alignment_edge   = 0,
    IfxGtm_Pwm_Alignment_center = 1
} IfxGtm_Pwm_Alignment;

/* Sync channel index (subset) */
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
    IfxGtm_Pwm_SyncChannelIndex_9
} IfxGtm_Pwm_SyncChannelIndex;

/* PWM channel */
typedef struct {
    IfxGtm_Pwm_ChannelRegisters registers;
    uint32                      upenMask;
    IfxGtm_Pwm_callBack         periodEvent;
    IfxGtm_Pwm_callBack         dutyEvent;
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    uint32                      phaseTicks;
    uint32                      dutyTicks;
} IfxGtm_Pwm_Channel;

/* Forward/mock types for SFR clusters */
typedef struct { uint32 r; } Ifx_GTM_ATOM;
typedef struct { uint32 r; } Ifx_GTM_TOM;
typedef struct { uint32 r; } Ifx_GTM_CDTM;

typedef struct {
    Ifx_GTM_ATOM *ATOM;
    Ifx_GTM_TOM  *TOM;
    Ifx_GTM_CDTM *CDTM;
} IfxGtm_Pwm_ClusterSFR;

/* Global control */
typedef struct {
    volatile uint32 *reg0;
    volatile uint32 *reg1;
    uint32           upenMask0;
    uint32           upenMask1;
    volatile uint32 *endisCtrlReg0;
    volatile uint32 *endisCtrlReg1;
} IfxGtm_Pwm_GlobalControl;

/* Pin helper */
typedef struct {
    IfxGtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode  outputMode;
    IfxPort_PadDriver   padDriver;
} IfxGtm_Pwm_Pin;

/* Clock source */
typedef union {
    IfxGtm_Cmu_Clk   atom;
    IfxGtm_Cmu_Fxclk tom;
} IfxGtm_Pwm_ClockSource;

/* MSC out mock */
typedef struct { uint32 dummy; } IfxGtm_Trig_MscOut;

/* Channel configuration */
typedef struct {
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    float32                     phase;
    float32                     duty;
    IfxGtm_Pwm_DtmConfig       *dtm;
    IfxGtm_Pwm_OutputConfig    *output;
#ifndef DEVICE_TC33X
    IfxGtm_Trig_MscOut         *mscOut;
#endif
    IfxGtm_Pwm_InterruptConfig *interrupt;
} IfxGtm_Pwm_ChannelConfig;

/* PWM object and configuration (minimal mock representations) */
typedef struct {
    IfxGtm_Pwm_ClockSource clock;
    IfxGtm_Pwm_State       state;
    void                  *internal; /* reserved */
} IfxGtm_Pwm;

typedef struct {
    Ifx_GTM               *gtm;
    IfxGtm_Pwm_ClockSource clock;
    IfxGtm_Pwm_Alignment   alignment;
    float32                frequency;
    uint32                 channelsMask;
} IfxGtm_Pwm_Config;

/* Function declarations required by template */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), sint32 priority, IfxSrc_Tos service);
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 freq);
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 frequency);
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, Ifx_ActiveState polarity);
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, IfxGtm_Pwm_DeadTime *deadTime);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *chCfg, IfxGtm_Pwm_Config *cfg);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 channelMask);
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 channelMask);
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);

#endif /* IFXGTM_PWM_H */
