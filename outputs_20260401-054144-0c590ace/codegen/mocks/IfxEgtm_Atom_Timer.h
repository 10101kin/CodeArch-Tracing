#ifndef IFXEGTM_ATOM_TIMER_H
#define IFXEGTM_ATOM_TIMER_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* CountDir enum (as requested: 2 values) */
typedef enum {
    IfxEgtm_Atom_Timer_CountDir_up,
    IfxEgtm_Atom_Timer_CountDir_upAndDown
} IfxEgtm_Atom_Timer_CountDir;

/* Interrupt config */
typedef struct {
    Ifx_Priority    isrPriority;
    IfxSrc_Tos      isrProvider;
    IfxEgtm_IrqMode irqMode;
    IfxSrc_VmId     vmId;
} IfxEgtm_Atom_Timer_Interrupt;

/* Trigger config */
typedef struct {
    boolean                      enabled;
    uint32                       triggerPoint;
    IfxPort_OutputMode           outputMode;
    IfxPort_PadDriver            outputDriver;
    boolean                      risingEdgeAtPeriod;
    boolean                      outputEnabled;
    IfxEgtm_Atom_Timer_Interrupt interrupt;
} IfxEgtm_Atom_Timer_Trigger;

/* Driver object */
typedef struct {
    Ifx_EGTM                   *egtm;
    Ifx_EGTM_CLS_ATOM          *atom;
    Ifx_EGTM_CLS_ATOM_AGC      *agc;
    IfxEgtm_Cluster             clsIndex;
    uint32                      timerChannel;   /* placeholder for IfxEgtm_Atom_Ch */
    uint32                      triggerChannel; /* placeholder for IfxEgtm_Atom_Ch */
    uint16                      channelsMask;
    uint32                      offset;
    Ifx_EGTM_CLS_CDTM_DTM      *dtm;
    uint32                      dtmChannel;     /* placeholder for IfxEgtm_Dtm_Ch */
    uint32                      agcDisableUpdate;
    uint32                      agcApplyUpdate;
    uint32                      period;
    boolean                     triggerEnabled;
    float32                     clockFreq;
    IfxEgtm_Atom_Timer_CountDir countDir;
} IfxEgtm_Atom_Timer;

/* Config */
typedef struct {
    Ifx_EGTM                    *egtm;
    IfxEgtm_Cluster              cluster;
    uint32                       timerChannel;    /* placeholder for IfxEgtm_Atom_Ch */
    IfxEgtm_Atom_ToutMap        *triggerOut;
    uint32                       clock;           /* placeholder for IfxEgtm_Atom_Ch_ClkSrc */
    IfxEgtm_Dtm_ClockSource      dtmClockSource;
    float32                      frequency;
    float32                      minResolution;
    float32                      startOffset;
    IfxEgtm_Atom_Timer_CountDir  countDir;
    IfxEgtm_Atom_Timer_Interrupt interrupt;
    IfxEgtm_Atom_Timer_Trigger   trigger;
    boolean                      initPins;
} IfxEgtm_Atom_Timer_Config;

/* Required API (subset) */
boolean IfxEgtm_Atom_Timer_setFrequency(IfxEgtm_Atom_Timer *driver, float32 frequency);
void    IfxEgtm_Atom_Timer_setTrigger(IfxEgtm_Atom_Timer *driver, uint32 triggerPoint);
void    IfxEgtm_Atom_Timer_run(IfxEgtm_Atom_Timer *driver);

#endif /* IFXEGTM_ATOM_TIMER_H */
