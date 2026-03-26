/* IfxGtm_Pwm high-level PWM driver mock */
#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* Forward minimal SFR types used in ClusterSFR */
typedef struct { uint32 reserved; } Ifx_GTM_ATOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM;

/* Tout map types used by union */
typedef struct { uint8 atom; uint8 channel; } IfxGtm_Atom_ToutMap;
typedef struct { uint8 tom;  uint8 channel; } IfxGtm_Tom_ToutMap;

typedef void (*IfxGtm_Pwm_callBack)(void *arg);

/* Dead time */
typedef struct
{
    float32 rising;        /* Relative rise time */
    float32 falling;       /* Relative fall time */
} IfxGtm_Pwm_DeadTime;

/* Tout map union */
typedef union
{
    IfxGtm_Atom_ToutMap atom;       /* ATOM map */
    IfxGtm_Tom_ToutMap  tom;        /* TOM map */
} IfxGtm_Pwm_ToutMap;

/* Channel SFR pointers */
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

/* DTM config */
typedef struct
{
    IfxGtm_Pwm_DeadTime deadTime;       /* Dead time in seconds */
} IfxGtm_Pwm_DtmConfig;

/* Interrupt config */
typedef struct
{
    IfxGtm_IrqMode      mode;              /* IRQ mode */
    IfxSrc_Tos          isrProvider;       /* Type of service */
    Ifx_Priority        priority;          /* Priority */
    IfxGtm_Pwm_callBack periodEvent;       /* Period callback */
    IfxGtm_Pwm_callBack dutyEvent;         /* Duty callback */
} IfxGtm_Pwm_InterruptConfig;

/* Output config */
typedef struct
{
    IfxGtm_Pwm_ToutMap *pin;                        /* Output pin */
    IfxGtm_Pwm_ToutMap *complementaryPin;           /* Complementary output pin (_N) */
    Ifx_ActiveState     polarity;                   /* Active low/high */
    Ifx_ActiveState     complementaryPolarity;      /* Active low/high */
    IfxPort_OutputMode  outputMode;                 /* Output mode */
    IfxPort_PadDriver   padDriver;                  /* Pad driver */
} IfxGtm_Pwm_OutputConfig;

/* Submodule channel enum (0..9 sufficient for mocks) */
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

/* Channel object */
typedef struct
{
    IfxGtm_Pwm_ChannelRegisters registers;   /* Pointers to channel SFRs */
    uint32                      upenMask;    /* Update enable mask */
    IfxGtm_Pwm_callBack         periodEvent; /* CCU0 callback */
    IfxGtm_Pwm_callBack         dutyEvent;   /* CCU1 callback */
    IfxGtm_Pwm_SubModule_Ch     timerCh;     /* Channel Index */
    uint32                      phaseTicks;  /* phase ticks */
    uint32                      dutyTicks;   /* duty ticks */
} IfxGtm_Pwm_Channel;

/* Channel config */
typedef struct
{
    IfxGtm_Pwm_SubModule_Ch     timerCh;         /* Channel Index */
    float32                     phase;           /* Initial phase (rad) */
    float32                     duty;            /* Duty in % */
    IfxGtm_Pwm_DtmConfig       *dtm;             /* Dead time config */
    IfxGtm_Pwm_OutputConfig    *output;          /* Pin connections */
#ifndef DEVICE_TC33X
    void                       *mscOut;          /* MSC Output configuration (mocked as void*) */
#endif
    IfxGtm_Pwm_InterruptConfig *interrupt;       /* Interrupt config */
} IfxGtm_Pwm_ChannelConfig;

/* Clock source */
typedef enum {
    IfxGtm_Cmu_Clk_0 = 0,
    IfxGtm_Cmu_Clk_1,
    IfxGtm_Cmu_Clk_2,
    IfxGtm_Cmu_Clk_3,
    IfxGtm_Cmu_Clk_4,
    IfxGtm_Cmu_Clk_5,
    IfxGtm_Cmu_Clk_6,
    IfxGtm_Cmu_Clk_7
} IfxGtm_Cmu_Clk;

typedef enum {
    IfxGtm_Cmu_Fxclk_0 = 0,
    IfxGtm_Cmu_Fxclk_1,
    IfxGtm_Cmu_Fxclk_2,
    IfxGtm_Cmu_Fxclk_3,
    IfxGtm_Cmu_Fxclk_4
} IfxGtm_Cmu_Fxclk;

typedef union
{
    IfxGtm_Cmu_Clk   atom;       /* Clock source for ATOM */
    IfxGtm_Cmu_Fxclk tom;        /* Clock source for TOM */
} IfxGtm_Pwm_ClockSource;

/* Cluster SFR */
typedef struct
{
    Ifx_GTM_ATOM *ATOM;       /* Cluster SFR */
    Ifx_GTM_TOM  *TOM;        /* Cluster SFR */
    Ifx_GTM_CDTM *CDTM;       /* Cluster SFR */
} IfxGtm_Pwm_ClusterSFR;

/* Global control */
typedef struct
{
    volatile uint32 *reg0;          /* AGC/TGC0/1 GLB_CTRL */
    volatile uint32 *reg1;          /* TGC1_GLB_CTRL if spanning 2 TGCs */
    uint32           upenMask0;     /* UPEN mask reg0 */
    uint32           upenMask1;     /* UPEN mask reg1 */
    volatile uint32 *endisCtrlReg0; /* ENDIS CTRL reg0 */
    volatile uint32 *endisCtrlReg1; /* ENDIS CTRL reg1 */
} IfxGtm_Pwm_GlobalControl;

/* Pin helper */
typedef struct
{
    IfxGtm_Pwm_ToutMap *outputPin;  /* Output pin */
    IfxPort_OutputMode  outputMode; /* Output mode */
    IfxPort_PadDriver   padDriver;  /* Pad driver */
} IfxGtm_Pwm_Pin;

/* Alignment */
typedef enum
{
    IfxGtm_Pwm_Alignment_edge   = 0,
    IfxGtm_Pwm_Alignment_center = 1
} IfxGtm_Pwm_Alignment;

/* Channel state */
typedef enum
{
    IfxGtm_Pwm_ChannelState_running = 0,
    IfxGtm_Pwm_ChannelState_stopped
} IfxGtm_Pwm_ChannelState;

/* Reset event */
typedef enum
{
    IfxGtm_Pwm_ResetEvent_onCm0     = 0,
    IfxGtm_Pwm_ResetEvent_onTrigger = 1
} IfxGtm_Pwm_ResetEvent;

/* State */
typedef enum
{
    IfxGtm_Pwm_State_unknown = -1,
    IfxGtm_Pwm_State_init    = 0,
    IfxGtm_Pwm_State_run     = 1,
    IfxGtm_Pwm_State_stopped = 2,
    IfxGtm_Pwm_State_error   = 3
} IfxGtm_Pwm_State;

/* Submodule */
typedef enum
{
    IfxGtm_Pwm_SubModule_atom = 0,
    IfxGtm_Pwm_SubModule_tom  = 1
} IfxGtm_Pwm_SubModule;

/* Sync channel index (0..9) */
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

/* Top-level PWM handle and config (minimal) */
typedef struct { Ifx_GTM *gtm; IfxGtm_Pwm_State state; } IfxGtm_Pwm;

typedef struct
{
    Ifx_GTM                 *gtm;
    IfxGtm_Pwm_Alignment     alignment;
    float32                  frequency;
    IfxGtm_Pwm_ClockSource   clock;
    IfxGtm_Pwm_GlobalControl globalControl;
    IfxGtm_Pwm_ClusterSFR    cluster;
    uint32                   numChannels;
} IfxGtm_Pwm_Config;

/* Declarations requested by template */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), uint32 priority);
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 freq);
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateFrequency(IfxGtm_Pwm *pwm, float32 freq);
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm *pwm, uint32 mask, Ifx_ActiveState polarity);
void IfxGtm_Pwm_updateChannelPhase(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelPhaseImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 phase);
void IfxGtm_Pwm_updateChannelDuty(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, float32 duty);
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SubModule_Ch ch, IfxGtm_Pwm_DeadTime *dt);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *chCfg, IfxGtm_Pwm_Config *cfg);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask);
void IfxGtm_Pwm_stopSyncedChannels(IfxGtm_Pwm *pwm, uint32 mask);
void IfxGtm_Pwm_startSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);
void IfxGtm_Pwm_stopSyncedGroups(IfxGtm_Pwm *pwm, uint32 groupMask);

#endif /* IFXGTM_PWM_H */
