#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Forward declaration for pointer member used in config */
typedef struct IfxGtm_Tom_ToutMap IfxGtm_Tom_ToutMap;

/* Additional SFR-related opaque types used in structs */
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

typedef enum { IfxGtm_Tom_0 = 0, IfxGtm_Tom_1, IfxGtm_Tom_2, IfxGtm_Tom_3, IfxGtm_Tom_4, IfxGtm_Tom_5, IfxGtm_Tom_6, IfxGtm_Tom_7 } IfxGtm_Tom;

typedef enum { IfxGtm_Tom_Ch_0 = 0, IfxGtm_Tom_Ch_1, IfxGtm_Tom_Ch_2, IfxGtm_Tom_Ch_3, IfxGtm_Tom_Ch_4, IfxGtm_Tom_Ch_5, IfxGtm_Tom_Ch_6, IfxGtm_Tom_Ch_7, IfxGtm_Tom_Ch_8, IfxGtm_Tom_Ch_9, IfxGtm_Tom_Ch_10, IfxGtm_Tom_Ch_11, IfxGtm_Tom_Ch_12, IfxGtm_Tom_Ch_13, IfxGtm_Tom_Ch_14, IfxGtm_Tom_Ch_15 } IfxGtm_Tom_Ch;

typedef enum
{
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk1 = 1,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk0   = 2,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk1   = 3
} IfxGtm_Tom_Ch_ClkSrc;

/* StdIf Timer mock types */
typedef enum { IfxStdIf_Timer_CountDir_up = 0, IfxStdIf_Timer_CountDir_down = 1 } IfxStdIf_Timer_CountDir;

typedef struct
{
    float32                 frequency;   /* input frequency in Hz */
    IfxStdIf_Timer_CountDir countDir;    /* counting direction */
} IfxStdIf_Timer_Config;

/* IfxGtm_Tom_Timer_Base (from iLLD) */
typedef struct
{
    Ifx_TimerValue          period;               /* Timer period in ticks (cached value) */
    boolean                 triggerEnabled;       /* If TRUE, the trigger functionality is Initialised */
    float32                 clockFreq;            /* Timer input clock frequency (cached value) */
    IfxStdIf_Timer_CountDir countDir;             /* Timer counting mode */
} IfxGtm_Tom_Timer_Base;

/* IfxGtm_Tom_Timer (from iLLD) */
typedef struct
{
    IfxGtm_Tom_Timer_Base base;                                   /* Timer base structure */
    Ifx_GTM              *gtm;                                    /* Pointer to GTM module */
    Ifx_GTM_TOM          *tom;                                    /* Pointer to the TOM object */
    Ifx_GTM_TOM_TGC      *tgc[2];                                 /* Pointer to the TGC object */
    IfxGtm_Tom            tomIndex;                               /* Enum for TOM objects */
    IfxGtm_Tom_Ch         timerChannel;                           /* TOM channel used for the timer */
    IfxGtm_Tom_Ch         triggerChannel;                         /* TOM channel used for the trigger */
    uint16                channelsMask[2];                        /* Mask for channels to be modified together */
    Ifx_TimerValue        offset;                                 /* Timer initial offset in ticks */
    uint32                tgcGlobalControlDisableUpdate[2];       /* Cached value for TGC GLOB_CTR */
    uint32                tgcGlobalControlApplyUpdate[2];         /* Cached value for TGC GLOB_CTR */
    Ifx_GTM_CDTM_DTM     *dtm;                                    /* Pointer to DTM object used by TOM */
    uint8                 dtmChannel;                              /* DTM Channel */
} IfxGtm_Tom_Timer;

/* IfxGtm_Tom_Timer_Config (from iLLD) */
typedef struct
{
    IfxStdIf_Timer_Config  base;                 /* Standard interface timer configuration */
    Ifx_GTM               *gtm;                  /* Pointer to GTM module */
    IfxGtm_Tom             tom;                  /* Index of the TOM object used */
    IfxGtm_Tom_Ch          timerChannel;         /* TOM channel used for the timer */
    IfxGtm_Tom_ToutMap    *triggerOut;           /* TOM channel used for the trigger output */
    IfxGtm_Tom_Ch_ClkSrc   clock;                /* Timer input clock */
    IfxGtm_IrqMode         irqModeTimer;         /* Interrupt mode for the timer */
    IfxGtm_IrqMode         irqModeTrigger;       /* Interrupt mode for the trigger */
    IfxGtm_Dtm_ClockSource dtmClockSource;       /* DTM clock source */
    boolean                initPins;             /* TRUE : Initialize pins in driver */
} IfxGtm_Tom_Timer_Config;

/* Functions to mock */
void          IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
boolean       IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void          IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
void          IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
