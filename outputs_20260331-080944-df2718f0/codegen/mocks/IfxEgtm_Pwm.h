/* Mock IfxEgtm_Pwm.h for TC4xx */
#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* ClockSource union with uint32 fields to avoid enum conversion issues */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* Minimal alignment/submodule placeholders to satisfy structure fields */
typedef enum { IfxEgtm_Pwm_Alignment_left = 0, IfxEgtm_Pwm_Alignment_center = 1 } IfxEgtm_Pwm_Alignment;

typedef struct IfxEgtm_Pwm_Channel
{
    uint32   timerCh;   /* SubModule_Ch enum placeholder */
    float32  phase;
    float32  duty;
    void    *dtm;       /* DTM pointer */
    void    *output;    /* Port/pin output */
    void    *mscOut;    /* MSC config pointer */
    void    *interrupt; /* Interrupt config pointer */
} IfxEgtm_Pwm_Channel;

/* Handle and Config structs */
typedef struct IfxEgtm_Pwm
{
    Ifx_EGTM *egtmSfr;
    uint32    subModule;          /* placeholder */
    uint32    cluster;            /* placeholder */
    IfxEgtm_Pwm_ClockSource clockSource;
    IfxEgtm_Pwm_Alignment   alignment;
    float32   frequency;
    uint32    numChannels;
    IfxEgtm_Pwm_Channel *channels;  /* pointer to channels array */
    boolean   syncStart;
    boolean   syncUpdateEnabled;
    uint32    dtmClockSource;     /* placeholder for DTM clock source */
} IfxEgtm_Pwm;

typedef struct IfxEgtm_Pwm_Config
{
    Ifx_EGTM *egtmSfr;
    uint32    subModule;
    uint32    cluster;
    IfxEgtm_Pwm_ClockSource clockSource;
    IfxEgtm_Pwm_Alignment   alignment;
    float32   frequency;
    uint32    numChannels;
    IfxEgtm_Pwm_Channel *channels;
    boolean   syncStart;
    boolean   syncUpdateEnabled;
    uint32    dtmClockSource;
} IfxEgtm_Pwm_Config;

/* Tout map typedef */
typedef struct {
    uint32 dummy;
} IfxEgtm_Pwm_ToutMap;

/* Function declarations */
void    IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm);
boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *handle, const IfxEgtm_Pwm_Config *config);
void    IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *handle, const float32 *duties, uint32 numChannels);
void    IfxEgtm_Pwm_setDeadtime(IfxEgtm_Pwm *handle, const float32 *dtRise, const float32 *dtFall, uint32 numChannels);
void    IfxEgtm_Pwm_start(IfxEgtm_Pwm *handle, boolean sync);
void    IfxEgtm_Pwm_stop(IfxEgtm_Pwm *handle, boolean sync);

#endif /* IFXEGTM_PWM_H */
