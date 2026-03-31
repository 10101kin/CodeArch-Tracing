#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* ===== Clock source union (uint32 fields to avoid enum conversion issues) ===== */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* ===== Channel index enum (subset) ===== */
typedef enum {
    IfxEgtm_Atom_Ch_0 = 0,
    IfxEgtm_Atom_Ch_1,
    IfxEgtm_Atom_Ch_2,
    IfxEgtm_Atom_Ch_3,
    IfxEgtm_Atom_Ch_4,
    IfxEgtm_Atom_Ch_5,
    IfxEgtm_Atom_Ch_6,
    IfxEgtm_Atom_Ch_7
} IfxEgtm_Atom_Ch;

/* ===== PWM-related helper types ===== */
typedef struct {
    float32 dtRising;
    float32 dtFalling;
} IfxEgtm_Pwm_DeadTime;

typedef struct {
    Ifx_Priority  priority;
    IfxSrc_Tos    typeOfService;
    IfxEgtm_IrqMode mode;
} IfxEgtm_Pwm_InterruptConfig;

typedef struct {
    /* primary and complementary pins */
    void               *pin;                   /* typically points to IfxPort_Pin */
    void               *complementaryPin;      /* must exist to fix previous build errors */
    Ifx_ActiveState     polarity;              /* must exist to fix previous build errors */
    Ifx_ActiveState     complementaryPolarity; /* must exist to fix previous build errors */
    IfxPort_OutputMode  outputMode;            /* must exist to fix previous build errors */
} IfxEgtm_Pwm_OutputConfig;

typedef struct {
    IfxEgtm_Atom_Ch                 timerCh;     /* channel index */
    float32                         phase;
    float32                         duty;
    void                           *dtm;         /* dead-time module / config pointer */
    IfxEgtm_Pwm_OutputConfig       *output;      /* output configuration */
    void                           *mscOut;      /* MSC config pointer */
    IfxEgtm_Pwm_InterruptConfig    *interrupt;   /* interrupt config */
} IfxEgtm_Pwm_Channel;

typedef struct {
    Ifx_EGTM               *egtmSfr;            /* EGTM SFR base */
    uint32                  subModule;          /* ATOM submodule index */
    uint32                  cluster;            /* cluster index */
    IfxEgtm_Pwm_ClockSource clockSource;        /* clock source union */
    uint32                  alignment;          /* alignment mode */
    float32                 frequency;          /* PWM base frequency */
    uint32                  numChannels;        /* number of channels */
    IfxEgtm_Pwm_Channel    *channels;           /* channels array */
    boolean                 syncStart;          /* synchronized start */
    boolean                 syncUpdateEnabled;  /* synchronized update */
    uint32                  dtmClockSource;     /* DTM clock source */
} IfxEgtm_Pwm_Config;

typedef struct {
    IfxEgtm_Pwm_Config *config;
} IfxEgtm_Pwm;

/* ===== API ===== */
void    IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *cfg, Ifx_EGTM *egtm);
boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *driver, IfxEgtm_Pwm_Config *cfg);
void    IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *driver, const float32 *duties, uint32 numChannels);
void    IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *driver, const IfxEgtm_Pwm_DeadTime *dt, uint32 numChannels);
void    IfxEgtm_Pwm_start(IfxEgtm_Pwm *driver);
void    IfxEgtm_Pwm_stop(IfxEgtm_Pwm *driver);

/* Tout map type + required pin symbols */
typedef struct { uint32 map[4]; } IfxEgtm_Pwm_ToutMap;

extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT;
extern IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT;

#endif /* IFXEGTM_PWM_H */
