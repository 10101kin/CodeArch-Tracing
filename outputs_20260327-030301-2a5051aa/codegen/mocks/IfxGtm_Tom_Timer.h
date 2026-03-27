#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxStdIf.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"

/* Supporting low-level GTM/TOM SFR pointer types */
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

/* Enums used in TOM Timer */
typedef enum { IfxGtm_Tom_0 = 0, IfxGtm_Tom_1, IfxGtm_Tom_2, IfxGtm_Tom_3, IfxGtm_Tom_4, IfxGtm_Tom_5, IfxGtm_Tom_6, IfxGtm_Tom_7 } IfxGtm_Tom;

typedef enum { IfxGtm_Tom_Ch_0 = 0, IfxGtm_Tom_Ch_1, IfxGtm_Tom_Ch_2, IfxGtm_Tom_Ch_3, IfxGtm_Tom_Ch_4, IfxGtm_Tom_Ch_5, IfxGtm_Tom_Ch_6, IfxGtm_Tom_Ch_7, IfxGtm_Tom_Ch_8, IfxGtm_Tom_Ch_9, IfxGtm_Tom_Ch_10, IfxGtm_Tom_Ch_11, IfxGtm_Tom_Ch_12, IfxGtm_Tom_Ch_13, IfxGtm_Tom_Ch_14, IfxGtm_Tom_Ch_15 } IfxGtm_Tom_Ch;

typedef enum { IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 = 0, IfxGtm_Tom_Ch_ClkSrc_cmuClk0 = 1 } IfxGtm_Tom_Ch_ClkSrc;

typedef enum { IfxGtm_Dtm_Ch_0 = 0, IfxGtm_Dtm_Ch_1, IfxGtm_Dtm_Ch_2, IfxGtm_Dtm_Ch_3 } IfxGtm_Dtm_Ch;

/* Base structure */
typedef struct
{
    Ifx_TimerValue           period;         /* cached period */
    boolean                  triggerEnabled; /* trigger init flag */
    float32                  clockFreq;      /* input clock frequency */
    IfxStdIf_Timer_CountDir  countDir;       /* counting direction */
} IfxGtm_Tom_Timer_Base;

/* Driver handle */
typedef struct
{
    IfxGtm_Tom_Timer_Base base;                      /* base */
    Ifx_GTM              *gtm;                       /* GTM module */
    Ifx_GTM_TOM          *tom;                       /* TOM object */
    Ifx_GTM_TOM_TGC      *tgc[2];                    /* TGCs */
    IfxGtm_Tom            tomIndex;                  /* TOM index */
    IfxGtm_Tom_Ch         timerChannel;              /* timer channel */
    IfxGtm_Tom_Ch         triggerChannel;            /* trigger channel */
    uint16                channelsMask[2];           /* channel mask per TGC */
    Ifx_TimerValue        offset;                    /* initial offset */
    uint32                tgcGlobalControlDisableUpdate[2];
    uint32                tgcGlobalControlApplyUpdate[2];
    Ifx_GTM_CDTM_DTM     *dtm;                       /* DTM object */
    IfxGtm_Dtm_Ch         dtmChannel;                /* DTM channel */
} IfxGtm_Tom_Timer;

/* Config */
typedef struct
{
    IfxStdIf_Timer_Config   base;           /* StdIf timer config */
    Ifx_GTM                *gtm;            /* GTM module */
    IfxGtm_Tom              tom;            /* TOM index */
    IfxGtm_Tom_Ch           timerChannel;   /* TOM channel */
    IfxGtm_Tom_ToutMap     *triggerOut;     /* trigger output */
    IfxGtm_Tom_Ch_ClkSrc    clock;          /* clock source */
    IfxGtm_IrqMode          irqModeTimer;   /* timer IRQ mode */
    IfxGtm_IrqMode          irqModeTrigger; /* trigger IRQ mode */
    IfxGtm_Dtm_ClockSource  dtmClockSource; /* DTM clock */
    boolean                 initPins;       /* init pins */
    /* Compatibility field to fix previous build error */
    float32                 frequency;      /* direct frequency access */
} IfxGtm_Tom_Timer_Config;

/* Function declarations (full required set) */
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_setPeriod(IfxGtm_Tom_Timer *driver, Ifx_TimerValue period);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getOffset(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_acknowledgeTimerIrq(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_acknowledgeTriggerIrq(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel);
float32 IfxGtm_Tom_Timer_getFrequency(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getInputFrequency(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getResolution(IfxGtm_Tom_Timer *driver);
IfxGtm_Tom_ToutMap* IfxGtm_Tom_Timer_getTrigger(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_setFrequency(IfxGtm_Tom_Timer *driver, float32 frequency);
void    IfxGtm_Tom_Timer_setSingleMode(IfxGtm_Tom_Timer *driver, boolean enabled);
void    IfxGtm_Tom_Timer_setTrigger(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_ToutMap *triggerOut);
void    IfxGtm_Tom_Timer_stop(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);

/* StdIf adapter prototypes (use generic void* to avoid conflicts) */
void    IfxStdIf_Timer_run(void *driver);
void    IfxStdIf_Timer_disableUpdate(void *driver);
void    IfxStdIf_Timer_setPeriod(void *driver, float32 period);
void    IfxStdIf_Timer_applyUpdate(void *driver);

#endif /* IFXGTM_TOM_TIMER_H */
