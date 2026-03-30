#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"

/* Supporting GTM TOM types (single-owner here) */
typedef enum {
    IfxGtm_Tom_0 = 0,
    IfxGtm_Tom_1 = 1,
    IfxGtm_Tom_2 = 2,
    IfxGtm_Tom_3 = 3,
    IfxGtm_Tom_4 = 4,
    IfxGtm_Tom_5 = 5
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
    IfxGtm_Tom_Ch_ClkSrc_cmuClock0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuClock1 = 1,
    IfxGtm_Tom_Ch_ClkSrc_cmuClock2 = 2
} IfxGtm_Tom_Ch_ClkSrc;

typedef enum {
    IfxGtm_Dtm_Ch_0 = 0,
    IfxGtm_Dtm_Ch_1 = 1
} IfxGtm_Dtm_Ch;

/* SFR object stubs (types only) */
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

/* StdIf timer types used in mapping */
typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct {
    float32                 frequency;
    IfxStdIf_Timer_CountDir countDir;
} IfxStdIf_Timer_Config;

/* TOM TOUT types used by config */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
    uint8  tom;
    uint8  channel;
} IfxGtm_Tom_ToutMap;

typedef const IfxGtm_Tom_ToutMap * IfxGtm_Tom_ToutMapP;

typedef struct
{
    Ifx_TimerValue          period;               /**< Timer period in ticks (cached value) */
    boolean                 triggerEnabled;       /**< Trigger functionality initialized */
    float32                 clockFreq;            /**< Timer input clock frequency (cached value) */
    IfxStdIf_Timer_CountDir countDir;             /**< Timer counting mode */
} IfxGtm_Tom_Timer_Base;

typedef struct
{
    IfxGtm_Tom_Timer_Base base;                                   /**< Timer base structure */
    Ifx_GTM              *gtm;                                    /**< Pointer to GTM module */
    Ifx_GTM_TOM          *tom;                                    /**< Pointer to the TOM object */
    Ifx_GTM_TOM_TGC      *tgc[2];                                 /**< Pointer to the TGC object */
    IfxGtm_Tom            tomIndex;                               /**< Enum for TOM objects */
    IfxGtm_Tom_Ch         timerChannel;                           /**< TOM channel used for the timer */
    IfxGtm_Tom_Ch         triggerChannel;                         /**< TOM channel used for the trigger */
    uint16                channelsMask[2];                        /**< Mask for channels to be modified together */
    Ifx_TimerValue        offset;                                 /**< Timer initial offset in ticks */
    uint32                tgcGlobalControlDisableUpdate[2];       /**< Cached value for TGC GLOB_CTR */
    uint32                tgcGlobalControlApplyUpdate[2];         /**< Cached value for TGC GLOB_CTR */
    Ifx_GTM_CDTM_DTM     *dtm;                                    /**< Pointer to DTM object used by TOM */
    IfxGtm_Dtm_Ch         dtmChannel;                             /**< DTM Channel */
} IfxGtm_Tom_Timer;

typedef struct
{
    IfxStdIf_Timer_Config  base;                 /**< Standard interface timer configuration */
    Ifx_GTM               *gtm;                  /**< Pointer to GTM module */
    IfxGtm_Tom             tom;                  /**< Index of the TOM object used */
    IfxGtm_Tom_Ch          timerChannel;         /**< TOM channel used for the timer */
    IfxGtm_Tom_ToutMap    *triggerOut;           /**< TOM channel used for the trigger output */
    IfxGtm_Tom_Ch_ClkSrc   clock;                /**< Timer input clock */
    IfxGtm_IrqMode         irqModeTimer;         /**< Interrupt mode for the timer */
    IfxGtm_IrqMode         irqModeTrigger;       /**< Interrupt mode for the trigger */
    IfxGtm_Dtm_ClockSource dtmClockSource;       /**< DTM clock source */
    boolean                initPins;             /**< TRUE: Initialize pins in driver */
} IfxGtm_Tom_Timer_Config;

/* API (full list from template; only subset is spied) */
void           IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean        IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void           IfxGtm_Tom_Timer_stdIfTimerInit(void);
void           IfxStdIf_Timer_run(void);
void           IfxStdIf_Timer_disableUpdate(void);
void           IfxStdIf_Timer_setPeriod(void);
void           IfxStdIf_Timer_applyUpdate(void);
Ifx_TimerValue IfxGtm_Tom_Timer_getOffset(IfxGtm_Tom_Timer *driver);
void           IfxGtm_Tom_Timer_acknowledgeTimerIrq(void);
void           IfxGtm_Tom_Timer_acknowledgeTriggerIrq(void);
void           IfxGtm_Tom_Timer_addToChannelMask(void);
void           IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_TTimer *driver); /* compatibility alias will be corrected below */
void           IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
float32        IfxGtm_Tom_Timer_getFrequency(void);
float32        IfxGtm_Tom_Timer_getInputFrequency(void);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
float32        IfxGtm_Tom_Timer_getResolution(void);
void           IfxGtm_Tom_Timer_getTrigger(void);
void           IfxGtm_Tom_Timer_run(void);
void           IfxGtm_Tom_Timer_setFrequency(void);
void           IfxGtm_Tom_Timer_setPeriod(void);
void           IfxGtm_Tom_Timer_setSingleMode(void);
void           IfxGtm_Tom_Timer_setTrigger(void);
void           IfxGtm_Tom_Timer_stdIfTimerInit(void);
void           IfxGtm_Tom_Timer_stop(void);
void           IfxGtm_Tom_Timer_updateInputFrequency(void);

/* Correct prototype for applyUpdate (as required to mock) */
void           IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
