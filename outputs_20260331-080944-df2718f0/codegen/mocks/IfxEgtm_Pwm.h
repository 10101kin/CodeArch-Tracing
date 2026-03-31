#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* ===== Enums ===== */

typedef enum
{
    IfxEgtm_Pwm_Alignment_edge = 0,
    IfxEgtm_Pwm_Alignment_center = 1
} IfxEgtm_Pwm_Alignment;

/* ===== Union: Clock source (use uint32 fields to avoid enum-conversion issues) ===== */

typedef union
{
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* ===== Structs ===== */

typedef struct
{
    IfxPort_OutputMode  mode;      /* must exist: tests read this */
    IfxPort_PadDriver   padDriver; /* must exist: tests read this */
    void               *pin;       /* placeholder to hold ToutMap reference if needed */
} IfxEgtm_Pwm_OutputConfig;

typedef struct
{
    float32 deadTime; /* must exist: tests read this */
} IfxEgtm_Pwm_DtmConfig;

typedef struct IfxEgtm_Pwm_Channel
{
    uint32                    timerCh;   /* SubModule_Ch placeholder */
    float32                   phase;
    float32                   duty;
    IfxEgtm_Pwm_DtmConfig    *dtm;
    IfxEgtm_Pwm_OutputConfig *output;
    void                     *mscOut;    /* MSC config pointer */
    void                     *interrupt; /* interrupt cfg */
} IfxEgtm_Pwm_Channel;

typedef struct
{
    Ifx_EGTM               *egtmSfr;
    uint32                  subModule;
    uint32                  cluster;
    IfxEgtm_Pwm_ClockSource clockSource;
    IfxEgtm_Pwm_Alignment   alignment;
    float32                 frequency;
    uint32                  numChannels;
    IfxEgtm_Pwm_Channel    *channels; /* array of channels */
    boolean                 syncStart;
    boolean                 syncUpdateEnabled;
    uint32                  dtmClockSource;
} IfxEgtm_Pwm_Config;

typedef struct
{
    Ifx_EGTM *egtmSfr;
    uint32     internal;
} IfxEgtm_Pwm;

/* ===== Pin map type and externs for symbols used by production ===== */

typedef struct { uint32 dummy; } IfxEgtm_Pwm_ToutMap;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT;

/* ===== Function declarations (primary PWM API used by production/mocks) ===== */

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *driver, const IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *driver, const float32 *duties, uint32 count);
void IfxEgtm_Pwm_setDeadtime(IfxEgtm_Pwm *driver, const float32 *dtRising, const float32 *dtFalling, uint32 count);

#endif /* IFXEGTM_PWM_H */
