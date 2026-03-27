#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"       /* IfxGtm_IrqMode */
#include "IfxGtm_Cmu.h"   /* IfxGtm_Dtm_ClockSource */
#include "IfxGtm_PinMap.h"/* IfxGtm_Tom_ToutMap */

/* Basic timer-related typedefs */
typedef uint32 Ifx_TimerValue;

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down,
    IfxStdIf_Timer_CountDir_upAndDown
} IfxStdIf_Timer_CountDir;

/* Minimal StdIf Timer Config used by driver */
typedef struct {
    float32                 frequency;
    Ifx_TimerValue          period;
    boolean                 enabled;
    IfxStdIf_Timer_CountDir countDir;
} IfxStdIf_Timer_Config;

/* TOM identifiers */
typedef enum {
    IfxGtm_Tom_0 = 0, IfxGtm_Tom_1, IfxGtm_Tom_2, IfxGtm_Tom_3, IfxGtm_Tom_4, IfxGtm_Tom_5, IfxGtm_Tom_6, IfxGtm_Tom_7
} IfxGtm_Tom;

typedef enum {
    IfxGtm_Tom_Ch_0 = 0, IfxGtm_Tom_Ch_1, IfxGtm_Tom_Ch_2, IfxGtm_Tom_Ch_3,
    IfxGtm_Tom_Ch_4, IfxGtm_Tom_Ch_5, IfxGtm_Tom_Ch_6, IfxGtm_Tom_Ch_7,
    IfxGtm_Tom_Ch_8, IfxGtm_Tom_Ch_9, IfxGtm_Tom_Ch_10, IfxGtm_Tom_Ch_11,
    IfxGtm_Tom_Ch_12, IfxGtm_Tom_Ch_13, IfxGtm_Tom_Ch_14, IfxGtm_Tom_Ch_15
} IfxGtm_Tom_Ch;

typedef enum {
    /* CMU Clk 0..7 */
    IfxGtm_Tom_Ch_ClkSrc_cmuClk0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk1,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk2,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk3,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk4,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk5,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk6,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk7,
    /* FXCLK 0..4 */
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk1,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk2,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk3,
    IfxGtm_Tom_Ch_ClkSrc_cmuFxclk4
} IfxGtm_Tom_Ch_ClkSrc;

/* Forward SFR-like stubbed structs used by driver */
typedef struct { uint32 dummy; } Ifx_GTM_TOM;
typedef struct { uint32 dummy; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 dummy; } Ifx_GTM_CDTM_DTM;

typedef enum { IfxGtm_Dtm_Ch_0 = 0 } IfxGtm_Dtm_Ch;

/* Driver base and handle structs (layout from iLLD) */
typedef struct {
    Ifx_TimerValue           period;
    boolean                  triggerEnabled;
    float32                  clockFreq;
    IfxStdIf_Timer_CountDir  countDir;
} IfxGtm_Tom_Timer_Base;

typedef struct {
    IfxGtm_Tom_Timer_Base base;
    Ifx_GTM              *gtm;
    Ifx_GTM_TOM          *tom;
    Ifx_GTM_TOM_TGC      *tgc[2];
    IfxGtm_Tom            tomIndex;
    IfxGtm_Tom_Ch         timerChannel;
    IfxGtm_Tom_Ch         triggerChannel;
    uint16                channelsMask[2];
    Ifx_TimerValue        offset;
    uint32                tgcGlobalControlDisableUpdate[2];
    uint32                tgcGlobalControlApplyUpdate[2];
    Ifx_GTM_CDTM_DTM     *dtm;
    IfxGtm_Dtm_Ch         dtmChannel;
} IfxGtm_Tom_Timer;

typedef struct {
    IfxStdIf_Timer_Config   base;
    Ifx_GTM                *gtm;
    IfxGtm_Tom              tom;
    IfxGtm_Tom_Ch           timerChannel;
    IfxGtm_Tom_ToutMap     *triggerOut;
    IfxGtm_Tom_Ch_ClkSrc    clock;
    IfxGtm_IrqMode          irqModeTimer;
    IfxGtm_IrqMode          irqModeTrigger;
    IfxGtm_Dtm_ClockSource  dtmClockSource;
    boolean                 initPins;
} IfxGtm_Tom_Timer_Config;

/* Functions to mock (exact signatures provided) */
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
void    IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void    IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void    IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
