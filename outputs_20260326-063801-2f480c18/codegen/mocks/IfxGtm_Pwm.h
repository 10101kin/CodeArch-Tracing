#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Forward SFR pointer types used by ClusterSFR */
typedef struct { uint32 r; } Ifx_GTM_ATOM;
typedef struct { uint32 r; } Ifx_GTM_TOM;
typedef struct { uint32 r; } Ifx_GTM_CDTM;

typedef void (*IfxGtm_Pwm_callBack)(void *arg);

typedef struct {
    float32 rising;
    float32 falling;
} IfxGtm_Pwm_DeadTime;

/* Minimal Tout maps to represent a pin selection */
typedef struct { uint8 module; uint8 channel; uint8 pad; } IfxGtm_Atom_ToutMap;
typedef struct { uint8 module; uint8 channel; uint8 pad; } IfxGtm_Tom_ToutMap;

typedef union {
    IfxGtm_Atom_ToutMap atom;
    IfxGtm_Tom_ToutMap  tom;
} IfxGtm_Pwm_ToutMap;

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

typedef enum {
    IfxPort_OutputMode_pushPull = 0,
    IfxPort_OutputMode_openDrain = 1
} IfxPort_OutputMode; /* duplicated in IfxPort.h as well; used here by reference only */

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2
} IfxPort_PadDriver; /* duplicated in IfxPort.h as well; used here by reference only */

typedef struct
{
    IfxGtm_Pwm_ToutMap *pin;
    IfxGtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState     polarity;
    Ifx_ActiveState     complementaryPolarity;
    IfxPort_OutputMode  outputMode;
    IfxPort_PadDriver   padDriver;
} IfxGtm_Pwm_OutputConfig;

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

typedef struct IfxGtm_Trig_MscOut_s { uint32 dummy; } IfxGtm_Trig_MscOut;

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
    IfxGtm_Trig_MscOut         *mscOut;   /* keep field for TC3xx completeness */
    IfxGtm_Pwm_InterruptConfig *interrupt;
} IfxGtm_Pwm_ChannelConfig;

typedef union
{
    IfxGtm_Cmu_Clk   atom;
    IfxGtm_Cmu_Fxclk tom;
} IfxGtm_Pwm_ClockSource;

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

typedef struct
{
    IfxGtm_Pwm_Channel   *channels;
    uint32                 channelCount;
    Ifx_GTM               *gtm;
    float32                frequency;
    IfxGtm_Pwm_ClockSource clockSource;
    IfxGtm_Pwm_GlobalControl global;
} IfxGtm_Pwm;

typedef enum {
    IfxGtm_Pwm_Alignment_edge   = 0,
    IfxGtm_Pwm_Alignment_center = 1
} IfxGtm_Pwm_Alignment;

typedef enum {
    IfxGtm_Pwm_ChannelState_running = 0,
    IfxGtm_Pwm_ChannelState_stopped
} IfxGtm_Pwm_ChannelState;

typedef enum {
    IfxGtm_Pwm_ResetEvent_onCm0     = 0,
    IfxGtm_Pwm_ResetEvent_onTrigger = 1
} IfxGtm_Pwm_ResetEvent;

typedef enum {
    IfxGtm_Pwm_State_unknown = -1,
    IfxGtm_Pwm_State_init    = 0,
    IfxGtm_Pwm_State_run     = 1,
    IfxGtm_Pwm_State_stopped = 2,
    IfxGtm_Pwm_State_error   = 3
} IfxGtm_Pwm_State;

typedef enum {
    IfxGtm_Pwm_SubModule_atom = 0,
    IfxGtm_Pwm_SubModule_tom  = 1
} IfxGtm_Pwm_SubModule;

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

typedef struct
{
    Ifx_GTM               *gtm;
    IfxGtm_Pwm_ClockSource clockSource;
    IfxGtm_Pwm_Alignment   alignment;
    float32                frequency;
    IfxGtm_Pwm_ChannelConfig *channelConfig;
    uint32                 channelCount;
} IfxGtm_Pwm_Config;

typedef struct
{
    IfxGtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode  outputMode;
    IfxPort_PadDriver   padDriver;
} IfxGtm_Pwm_Pin;

/* API declarations (subset as per template) */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), uint32 vectabNum, Ifx_Priority priority);
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 freq);
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 frequency);
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, Ifx_ActiveState pol);
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, IfxGtm_Pwm_DeadTime *dt);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *chCfg);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask);
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask);
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask);
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 mask);

#ifdef __cplusplus
}
#endif

#endif /* IFXGTM_PWM_H */
