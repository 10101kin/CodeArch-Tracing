/* IfxGtm_Tom_Timer types + functions */
#ifndef IFXGTM_TOM_TIMER_H
#define IFXGTM_TOM_TIMER_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Support types used by Timer */
typedef uint32 Ifx_TimerValue;

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1,
    IfxStdIf_Timer_CountDir_upDown = 2
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

/* IfxGtm_Tom_Timer_Base definition */
typedef struct {
    Ifx_TimerValue          period;               /* Timer period in ticks (cached value) */
    boolean                 triggerEnabled;       /* If TRUE, trigger functionality is Initialized */
    float32                 clockFreq;            /* Timer input clock frequency (cached value) */
    IfxStdIf_Timer_CountDir countDir;             /* Timer counting mode */
} IfxGtm_Tom_Timer_Base;

/* Driver object and configuration (minimal mock representations) */
typedef struct {
    IfxGtm_Tom_Timer_Base base;
    void *internal; /* reserved */
} IfxGtm_Tom_Timer;

typedef struct {
    Ifx_GTM *gtm;
    float32  frequency;
    uint32   channelsMask;
} IfxGtm_Tom_Timer_Config;

/* Function declarations (as per template requirements) */
void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm);
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config);

/* StdIf interface declarations used by template */
void IfxGtm_Tom_Timer_stdIfTimerInit(void *stdif, IfxGtm_Tom_Timer *driver);
void IfxStdIf_Timer_run(void *stdif);
void IfxStdIf_Timer_disableUpdate(void *stdif);
void IfxStdIf_Timer_setPeriod(void *stdif, Ifx_TimerValue period);
void IfxStdIf_Timer_applyUpdate(void *stdif);

/* Additional Timer APIs referenced by template */
uint32 IfxGtm_Tom_Timer_getOffset(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_acknowledgeTimerIrq(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_acknowledgeTriggerIrq(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel);
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getFrequency(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getInputFrequency(IfxGtm_Tom_Timer *driver);
Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver);
float32 IfxGtm_Tom_Timer_getResolution(IfxGtm_Tom_Timer *driver);
uint32 IfxGtm_Tom_Timer_getTrigger(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver);
void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver);

#endif /* IFXGTM_TOM_TIMER_H */
