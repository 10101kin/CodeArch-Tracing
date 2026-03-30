#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Basic timer-related typedefs/enums needed by structs */
typedef uint32 Ifx_TimerValue;

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct {
    float32 frequency;
} IfxStdIf_Timer_Config;

/* TOM-related minimal enums/typedefs */
typedef uint8 IfxGtm_Tom;
typedef uint8 IfxGtm_Tom_Ch;
typedef uint8 IfxGtm_Tom_Ch_ClkSrc;
typedef uint8 IfxGtm_Dtm_Ch;

typedef struct IfxGtm_Tom_ToutMap IfxGtm_Tom_ToutMap; /* forward for pointer usage */

/* Structures from iLLD mapping */
typedef struct
{
    Ifx_TimerValue          period;
    boolean                 triggerEnabled;
    float32                 clockFreq;
    IfxStdIf_Timer_CountDir countDir;
} IfxGtm_Tom_Timer_Base;

typedef struct
{
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

typedef struct
{
    IfxStdIf_Timer_Config  base;
    Ifx_GTM               *gtm;
    IfxGtm_Tom             tom;
    IfxGtm_Tom_Ch          timerChannel;
    IfxGtm_Tom_ToutMap    *triggerOut;
    IfxGtm_Tom_Ch_ClkSrc   clock;
    IfxGtm_IrqMode         irqModeTimer;
    IfxGtm_IrqMode         irqModeTrigger;
    IfxGtm_Dtm_ClockSource dtmClockSource;
    boolean                initPins;
} IfxGtm_Tom_Timer_Config;

/* Functions to mock (subset) */
void     IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
boolean  IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);
void     IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
void     IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
