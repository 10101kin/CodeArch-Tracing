/* IfxEgtm_Pwm mock header */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Ensure all enum typedefs before structs */

typedef enum {
    IfxEgtm_Pwm_Alignment_edge = 0,
    IfxEgtm_Pwm_Alignment_center = 1
} IfxEgtm_Pwm_Alignment;
/* Compatibility alias for previous error */
#ifndef IfxEgtm_Pwm_Alignment_centerAlignedSymmetric
#define IfxEgtm_Pwm_Alignment_centerAlignedSymmetric IfxEgtm_Pwm_Alignment_center
#endif

typedef enum {
    IfxEgtm_Pwm_ChannelState_running = 0
} IfxEgtm_Pwm_ChannelState;

typedef enum {
    IfxEgtm_Pwm_ResetEvent_onCm0 = 0,
    IfxEgtm_Pwm_ResetEvent_onTrigger = 1
} IfxEgtm_Pwm_ResetEvent;

typedef enum {
    IfxEgtm_Pwm_State_unknown = -1,
    IfxEgtm_Pwm_State_init = 0,
    IfxEgtm_Pwm_State_run = 1,
    IfxEgtm_Pwm_State_stopped = 2,
    IfxEgtm_Pwm_State_error = 3
} IfxEgtm_Pwm_State;

typedef enum {
    IfxEgtm_Pwm_SubModule_atom = 0,
    IfxEgtm_Pwm_SubModule_tom  = 1
} IfxEgtm_Pwm_SubModule;

typedef enum {
    IfxEgtm_Pwm_SubModule_Ch_0 = 0,
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

/* Forward config-related enums from other headers (compatibility) */
#ifndef IfxEgtm_Dtm_ClockSource
typedef enum {
    IfxEgtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxEgtm_Dtm_ClockSource_cmuClk1 = 1,
    IfxEgtm_Dtm_ClockSource_cmuClk2 = 2
} IfxEgtm_Dtm_ClockSource;
/* compatibility alias for previous error */
#define IfxEgtm_Dtm_ClockSource_cmuClock0 IfxEgtm_Dtm_ClockSource_cmuClk0
#endif

#ifndef IfxSrc_VmId
typedef uint32 IfxSrc_VmId;
#endif

/* Additional enums required by structures */
typedef enum {
    IfxEgtm_Dtm_ShutoffInput_0 = 0,
    IfxEgtm_Dtm_ShutoffInput_1 = 1
} IfxEgtm_Dtm_ShutoffInput;

typedef enum {
    IfxEgtm_Dtm_SignalLevel_low = 0,
    IfxEgtm_Dtm_SignalLevel_high = 1
} IfxEgtm_Dtm_SignalLevel;

/* Callback type */
typedef void (*IfxEgtm_Pwm_callBack)(void *);

/* TOUT map unions (ATOM/TOM/HRPWM) */
typedef struct { uint32 dummy; } IfxEgtm_Atom_ToutMap;
typedef struct { uint32 dummy; } IfxEgtm_Tom_ToutMap;
typedef struct { uint32 dummy; } IfxEgtm_Hrpwm_Out;

typedef union {
    IfxEgtm_Atom_ToutMap atom;
    IfxEgtm_Tom_ToutMap  tom;
    /* HRPWM available on some devices; keep for compatibility */
    IfxEgtm_Hrpwm_Out    hrpwm;
} IfxEgtm_Pwm_ToutMap;

/* DeadTime */
typedef struct {
    float32 rising;
    float32 falling;
} IfxEgtm_Pwm_DeadTime;

/* Fast shutoff */
typedef struct {
    IfxEgtm_Dtm_ShutoffInput inputSignal;
    boolean                  invertInputSignal;
    IfxEgtm_Dtm_SignalLevel  offState;
    IfxEgtm_Dtm_SignalLevel  complementaryOffState;
} IfxEgtm_Pwm_FastShutoffConfig;

/* DTM config */
typedef struct {
    IfxEgtm_Pwm_DeadTime           deadTime;
    IfxEgtm_Pwm_FastShutoffConfig *fastShutOff;
} IfxEgtm_Pwm_DtmConfig;

/* Interrupt config */
typedef struct {
    IfxEgtm_IrqMode      mode;
    IfxSrc_Tos           isrProvider;
    Ifx_Priority         priority;
    IfxSrc_VmId          vmId;
    IfxEgtm_Pwm_callBack periodEvent;
    IfxEgtm_Pwm_callBack dutyEvent;
} IfxEgtm_Pwm_InterruptConfig;

/* Output config */
typedef struct {
    IfxEgtm_Pwm_ToutMap *pin;
    IfxEgtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState      polarity;
    Ifx_ActiveState      complementaryPolarity;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_OutputConfig;

/* Channel registers */
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

/* Clock source union (use uint32 fields to avoid enum-conversion issues) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* Channel config */
typedef struct {
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    float32                      phase;
    float32                      duty;
    IfxEgtm_Pwm_DtmConfig       *dtm;
    IfxEgtm_Pwm_OutputConfig    *output;
    void                        *mscOut;    /* MSC config pointer */
    IfxEgtm_Pwm_InterruptConfig *interrupt;
} IfxEgtm_Pwm_ChannelConfig;

/* Channel state */
typedef struct {
    IfxEgtm_Pwm_ChannelRegisters registers;
    uint32                       upenMask;
    IfxEgtm_Pwm_callBack         periodEvent;
    IfxEgtm_Pwm_callBack         dutyEvent;
    IfxEgtm_Pwm_SubModule_Ch     timerCh;
    uint32                       phaseTicks;
    uint32                       dutyTicks;
} IfxEgtm_Pwm_Channel;

/* Global control */
typedef struct {
    volatile Ifx_UReg_32Bit *reg0;
    volatile Ifx_UReg_32Bit *reg1;
    uint32                   upenMask0;
    uint32                   upenMask1;
    volatile Ifx_UReg_32Bit *endisCtrlReg0;
    volatile Ifx_UReg_32Bit *endisCtrlReg1;
} IfxEgtm_Pwm_GlobalControl;

/* Pin config */
typedef struct {
    IfxEgtm_Pwm_ToutMap *outputPin;
    IfxPort_OutputMode   outputMode;
    IfxPort_PadDriver    padDriver;
} IfxEgtm_Pwm_Pin;

/* PWM config */
typedef struct {
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

/* PWM object */
typedef struct {
    Ifx_EGTM                 *egtmSFR;
    void                     *clusterSFR; /* Ifx_EGTM_CLS* placeholder */
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

/* CPU/IRQ helper prototypes that appear in iLLD */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int priority);
void IfxCpu_enableInterrupts(void);

/* Function declarations (subset required by tests + key APIs) */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);

/* Compatibility alias: some code used wrong enum prefix */
#ifndef IfxEgtm_Pwm_IrqMode_pulseNotify
#define IfxEgtm_Pwm_IrqMode_pulseNotify IfxEgtm_IrqMode_pulseNotify
#endif

#endif /* IFXEGTM_PWM_H */
