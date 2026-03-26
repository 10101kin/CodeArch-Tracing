#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Support types typically from StdIf */
typedef uint32 Ifx_TimerValue;
typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down,
    IfxStdIf_Timer_CountDir_upAndDown
} IfxStdIf_Timer_CountDir;

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

/* IfxGtm_Tom_Timer_Base per iLLD layout */
typedef struct
{
    Ifx_TimerValue              period;        /* Timer period in ticks */
    boolean                     triggerEnabled;/* Trigger initialised flag */
    float32                     clockFreq;     /* Timer input clock frequency */
    IfxStdIf_Timer_CountDir     countDir;      /* Count direction */
} IfxGtm_Tom_Timer_Base;

/* Driver object */
typedef struct
{
    IfxGtm_Tom_Timer_Base base;
    Ifx_GTM              *gtm;
    uint32                channelMask;
    float32               frequency;
} IfxGtm_Tom_Timer;

/* Configuration object */
typedef struct
{
    Ifx_GTM                  *gtm;
    float32                   frequency;
    IfxStdIf_Timer_CountDir   countDir;
    boolean                   triggerEnabled;
} IfxGtm_Tom_Timer_Config;

/* Function declarations (subset required + extras declared) */
void    IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);

void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel);
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);

/* Additional declarations to match typical iLLD header (no stubs required unless used) */
void IfxGtm_Tom_Timer_stdIfTimerInit(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Timer_Config *config);
void IfxStdIf_Timer_run(void *stdif);
void IfxStdIf_Timer_disableUpdate(void *stdif);
void IfxStdIf_Timer_setPeriod(void *stdif, float32 period);
void IfxStdIf_Timer_applyUpdate(void *stdif);
float32 IfxGtm_Tom_Timer_getOffset(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_acknowledgeTimerIrq(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_acknowledgeTriggerIrq(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getFrequency(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getInputFrequency(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getResolution(IfxGtm_Tom_Timer *driver);
uint32 IfxGtm_Tom_Timer_getTrigger(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
