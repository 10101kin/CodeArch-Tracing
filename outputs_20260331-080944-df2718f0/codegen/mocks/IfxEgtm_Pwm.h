#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Enums and basic types first */
typedef enum
{
    IfxEgtm_Atom_Ch_0 = 0,
    IfxEgtm_Atom_Ch_1,
    IfxEgtm_Atom_Ch_2,
    IfxEgtm_Atom_Ch_3,
    IfxEgtm_Atom_Ch_4,
    IfxEgtm_Atom_Ch_5,
    IfxEgtm_Atom_Ch_6,
    IfxEgtm_Atom_Ch_7,
    IfxEgtm_Atom_Ch_8,
    IfxEgtm_Atom_Ch_9,
    IfxEgtm_Atom_Ch_10,
    IfxEgtm_Atom_Ch_11,
    IfxEgtm_Atom_Ch_12,
    IfxEgtm_Atom_Ch_13,
    IfxEgtm_Atom_Ch_14,
    IfxEgtm_Atom_Ch_15
} IfxEgtm_Atom_Ch;

/* Clock source union (uint32 fields to avoid enum conversion issues) */
typedef union
{
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* Forward struct declarations in order of dependency */
typedef struct IfxEgtm_Pwm_DtmConfig      IfxEgtm_Pwm_DtmConfig;
typedef struct IfxEgtm_Pwm_OutputConfig   IfxEgtm_Pwm_OutputConfig;
typedef struct IfxEgtm_Pwm_InterruptConfig IfxEgtm_Pwm_InterruptConfig;
typedef struct IfxEgtm_Pwm_ChannelConfig  IfxEgtm_Pwm_ChannelConfig;

/* DTM configuration */
struct IfxEgtm_Pwm_DtmConfig
{
    IfxEgtm_Cmu_Clk dtmClockSource; /* required field per rules */
    float32          deadTime;      /* previous errors referenced deadTime */
    boolean          fastShutOff;   /* previous errors referenced fastShutOff */
};

/* Output configuration; must include padDriver field */
struct IfxEgtm_Pwm_OutputConfig
{
    void               *pin;        /* TOUT map pointer (generic) */
    IfxPort_OutputMode  mode;       /* output mode */
    IfxPort_PadDriver   padDriver;  /* pad driver - missing previously */
};

/* Interrupt configuration (minimal) */
struct IfxEgtm_Pwm_InterruptConfig
{
    Ifx_Priority  priority;
    IfxSrc_Tos    isrProvider;
};

/* Channel configuration */
struct IfxEgtm_Pwm_ChannelConfig
{
    IfxEgtm_Atom_Ch              timerCh;   /* SubModule_Ch enum */
    float32                      phase;
    float32                      duty;
    IfxEgtm_Pwm_DtmConfig       *dtm;       /* pointer to DTM config */
    IfxEgtm_Pwm_OutputConfig    *output;    /* output pin configuration */
    void                        *mscOut;    /* MSC config pointer (void*) */
    IfxEgtm_Pwm_InterruptConfig *interrupt; /* interrupt configuration */
};

/* Handle and Config types */
typedef struct { uint32 dummy; } IfxEgtm_Pwm_Channel;
typedef struct { uint32 dummy; } IfxEgtm_Pwm;

typedef struct
{
    Ifx_EGTM                   *egtmSfr;          /* SFR pointer */
    uint32                      subModule;        /* atom instance id */
    uint32                      cluster;          /* cluster id */
    IfxEgtm_Pwm_ClockSource     clockSource;      /* union per rules */
    uint32                      alignment;        /* alignment mode (user-defined) */
    float32                     frequency;        /* PWM base frequency */
    uint32                      numChannels;      /* number of channels */
    IfxEgtm_Pwm_ChannelConfig  *channels;         /* pointer to channels array */
    boolean                     syncStart;        /* sync start enable */
    boolean                     syncUpdateEnabled;/* sync update enable */
    IfxEgtm_Cmu_Clk             dtmClockSource;   /* DTM clock source */
} IfxEgtm_Pwm_Config;

/* Tout map typedef (kept minimal) */
typedef struct { uint32 data[2]; } IfxEgtm_Pwm_ToutMap;

/* Function declarations (primary API) */
void    IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm);
boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *handle, const IfxEgtm_Pwm_Config *config);
void    IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *handle, const float32 *duties);
void    IfxEgtm_Pwm_updateChannelsDeadTimeImmediate(IfxEgtm_Pwm *handle, const float32 *dtRising, const float32 *dtFalling);

#endif /* IFXEGTM_PWM_H */
