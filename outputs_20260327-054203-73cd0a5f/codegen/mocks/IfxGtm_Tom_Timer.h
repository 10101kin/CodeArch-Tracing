#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Standard interface and basic typedefs needed by Timer */
typedef uint32 Ifx_TimerValue;

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct {
    float32 frequency;   /* Hz */
    uint32  numChannels; /* number of channels (mock extension) */
} IfxStdIf_Timer_Config;

/* Minimal GTM TOM related SFR typedefs used as pointers in structs */
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

/* Enums used by timer config/structs */
typedef enum {
    IfxGtm_Tom_0 = 0,
    IfxGtm_Tom_1 = 1,
    IfxGtm_Tom_2 = 2,
    IfxGtm_Tom_3 = 3,
    IfxGtm_Tom_4 = 4,
    IfxGtm_Tom_5 = 5,
    IfxGtm_Tom_6 = 6,
    IfxGtm_Tom_7 = 7
} IfxGtm_Tom;

typedef enum {
    IfxGtm_Tom_Ch_0 = 0,
    IfxGtm_Tom_Ch_1,
    IfxGtm_Tom_Ch_2,
    IfxGtm_Tom_Ch_3,
    IfxGtm_Tom_Ch_4,
    IfxGtm_Tom_Ch_5,
    IfxGtm_Tom_Ch_6,
    IfxGtm_Tom_Ch_7,
    IfxGtm_Tom_Ch_8,
    IfxGtm_Tom_Ch_9,
    IfxGtm_Tom_Ch_10,
    IfxGtm_Tom_Ch_11,
    IfxGtm_Tom_Ch_12,
    IfxGtm_Tom_Ch_13,
    IfxGtm_Tom_Ch_14,
    IfxGtm_Tom_Ch_15
} IfxGtm_Tom_Ch;

typedef enum {
    IfxGtm_Tom_Ch_ClkSrc_cmuClk0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk = 1
} IfxGtm_Tom_Ch_ClkSrc;

typedef enum {
    IfxGtm_Dtm_Ch_0 = 0,
    IfxGtm_Dtm_Ch_1,
    IfxGtm_Dtm_Ch_2,
    IfxGtm_Dtm_Ch_3,
    IfxGtm_Dtm_Ch_4,
    IfxGtm_Dtm_Ch_5,
    IfxGtm_Dtm_Ch_6,
    IfxGtm_Dtm_Ch_7
} IfxGtm_Dtm_Ch;

/* Minimal TOM pin map type used in Timer config */
typedef struct IfxGtm_Tom_ToutMap {
    uint8 tom;        /* TOM index */
    uint8 channel;    /* Channel index */
    uint8 padDriver;  /* Pad driver cfg */
    uint8 out;        /* Output idx */
} IfxGtm_Tom_ToutMap;
typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* Timer base structure (matches iLLD layout) */
typedef struct IfxGtm_Tom_Timer_Base {
    Ifx_TimerValue          period;               /* cached period */
    boolean                 triggerEnabled;       /* trigger initialized */
    float32                 clockFreq;            /* input clock frequency */
    IfxStdIf_Timer_CountDir countDir;             /* count mode */
} IfxGtm_Tom_Timer_Base;

/* Timer driver object (fields aligned with iLLD) */
typedef struct IfxGtm_Tom_Timer {
    IfxGtm_Tom_Timer_Base base;                                   /* Timer base structure */
    Ifx_GTM              *gtm;                                    /* Pointer to GTM module */
    Ifx_GTM_TOM          *tom;                                    /* Pointer to the TOM object */
    Ifx_GTM_TOM_TGC      *tgc[2];                                 /* Pointer to the TGC object */
    IfxGtm_Tom            tomIndex;                               /* Enum for TOM objects */
    IfxGtm_Tom_Ch         timerChannel;                           /* TOM channel for timer */
    IfxGtm_Tom_Ch         triggerChannel;                         /* TOM channel for trigger */
    uint16                channelsMask[2];                        /* mask for grouped channels */
    Ifx_TimerValue        offset;                                 /* initial offset */
    uint32                tgcGlobalControlDisableUpdate[2];       /* cached GLOB_CTR */
    uint32                tgcGlobalControlApplyUpdate[2];         /* cached GLOB_CTR */
    Ifx_GTM_CDTM_DTM     *dtm;                                    /* Pointer to DTM object */
    IfxGtm_Dtm_Ch         dtmChannel;                             /* DTM channel */
} IfxGtm_Tom_Timer;

/* Timer configuration (aligned + mock extensions for tests) */
typedef struct IfxGtm_Tom_Timer_Config {
    IfxStdIf_Timer_Config  base;                 /* Standard interface config */
    Ifx_GTM               *gtm;                  /* Pointer to GTM module */
    IfxGtm_Tom             tom;                  /* TOM index */
    IfxGtm_Tom_Ch          timerChannel;         /* Timer TOM channel */
    IfxGtm_Tom_ToutMapP    triggerOut;           /* Trigger output channel */
    IfxGtm_Tom_Ch_ClkSrc   clock;                /* Timer input clock */
    IfxGtm_IrqMode         irqModeTimer;         /* IRQ mode timer */
    IfxGtm_IrqMode         irqModeTrigger;       /* IRQ mode trigger */
    IfxGtm_Dtm_ClockSource dtmClockSource;       /* DTM clock source */
    boolean                initPins;             /* init pins flag */
    /* Mock extensions to support spy capture */
    float32                frequency;            /* Hz */
    uint32                 numChannels;          /* channels */
} IfxGtm_Tom_Timer_Config;

/* Functions from DRIVERS TO MOCK */
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
