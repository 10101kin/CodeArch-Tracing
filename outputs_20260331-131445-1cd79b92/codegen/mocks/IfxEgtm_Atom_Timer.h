#ifndef IFXEGTM_ATOM_TIMER_H
#define IFXEGTM_ATOM_TIMER_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Placeholder basic channel/dtm types */
typedef uint32 IfxEgtm_Atom_Ch;
typedef uint32 IfxEgtm_Dtm_Ch;
typedef uint32 IfxEgtm_Atom_Ch_ClkSrc;

/* Enum as required */
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

/* Timer driver state */
typedef struct {
    Ifx_EGTM                   *egtm;
    Ifx_EGTM_CLS_ATOM          *atom;
    Ifx_EGTM_CLS_ATOM_AGC      *agc;
    IfxEgtm_Cluster             clsIndex;
    IfxEgtm_Atom_Ch             timerChannel;
    IfxEgtm_Atom_Ch             triggerChannel;
    uint16                      channelsMask;
    uint32                      offset;
    Ifx_EGTM_CLS_CDTM_DTM      *dtm;
    IfxEgtm_Dtm_Ch              dtmChannel;
    uint32                      agcDisableUpdate;
    uint32                      agcApplyUpdate;
    uint32                      period;
    boolean                     triggerEnabled;
    float32                     clockFreq;
    IfxEgtm_Atom_Timer_CountDir countDir;
} IfxEgtm_Atom_Timer;

/* Timer config */
typedef struct {
    Ifx_EGTM                    *egtm;
    IfxEgtm_Cluster              cluster;
    IfxEgtm_Atom_Ch              timerChannel;
    /* triggerOut uses Atom ToutMap from Pwm header; forward use as void* here to avoid circular */
    void                        *triggerOut;
    IfxEgtm_Atom_Ch_ClkSrc       clock;
    IfxEgtm_Dtm_ClockSource      dtmClockSource;
    float32                      frequency;
    float32                      minResolution;
    float32                      startOffset;
    IfxEgtm_Atom_Timer_CountDir  countDir;
    IfxEgtm_Atom_Timer_Interrupt interrupt;
    IfxEgtm_Atom_Timer_Trigger   trigger;
    boolean                      initPins;
} IfxEgtm_Atom_Timer_Config;

/* External CPU/IRQ helpers */
void IfxCpu_Irq_installInterruptHandler(void (*isr)(void), int vectabNum, int priority);

/* Functions to mock */
boolean IfxEgtm_Atom_Timer_setFrequency(IfxEgtm_Atom_Timer *driver, float32 frequency);
void    IfxEgtm_Atom_Timer_setTrigger(IfxEgtm_Atom_Timer *driver, uint32 triggerPoint);
void    IfxEgtm_Atom_Timer_run(IfxEgtm_Atom_Timer *driver);
uint32  IfxEgtm_Atom_Timer_getPeriod(IfxEgtm_Atom_Timer *driver);

#endif /* IFXEGTM_ATOM_TIMER_H */
