/* IfxGtm_Tom_Timer types + functions */
#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Supporting types typically from StdIf */
typedef uint32 Ifx_TimerValue;

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down,
    IfxStdIf_Timer_CountDir_upAndDown
} IfxStdIf_Timer_CountDir;

/* TOM Channel enum (subset 0..15) */
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

/* Base from iLLD */
typedef struct
{
    Ifx_TimerValue          period;               /* cached period in ticks */
    boolean                 triggerEnabled;       /* trigger init flag */
    float32                 clockFreq;            /* input clock frequency */
    IfxStdIf_Timer_CountDir countDir;             /* count direction */
} IfxGtm_Tom_Timer_Base;

/* Driver object (minimal mock) */
typedef struct
{
    IfxGtm_Tom_Timer_Base base;
    Ifx_GTM              *gtm;
    uint32                channelMask;
} IfxGtm_Tom_Timer;

/* Config object (minimal mock) */
typedef struct
{
    Ifx_GTM      *gtm;
    float32       frequency;
    uint32        channelMask;
    IfxStdIf_Timer_CountDir countDir;
} IfxGtm_Tom_Timer_Config;

/* Declarations required */
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);

/* StdIf mock declarations (not implemented in stub unless used) */
void IfxGtm_Tom_Timer_stdIfTimerInit(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Timer_Config *config);
void IfxStdIf_Timer_run(void *driver);
void IfxStdIf_Timer_disableUpdate(void *driver);
void IfxStdIf_Timer_setPeriod(void *driver, Ifx_TimerValue period);
void IfxStdIf_Timer_applyUpdate(void *driver);

/* Additional API surface per template */
void IfxGtm_Tom_Timer_getOffset(void);
void IfxGtm_Tom_Timer_acknowledgeTimerIrq(void);
void IfxGtm_Tom_Timer_acknowledgeTriggerIrq(void);
void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel);
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_getFrequency(void);
void IfxGtm_Tom_Timer_getInputFrequency(void);
void IfxGtm_Tom_Timer_getPeriod(void);
void IfxGtm_Tom_Timer_getResolution(void);
void IfxGtm_Tom_Timer_getTrigger(void);

#endif /* IFXGTM_TOM_TIMER_H */
