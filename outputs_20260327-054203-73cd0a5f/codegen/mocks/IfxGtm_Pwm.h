#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxStdIf.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Forward/simple structs for ToutMap; keep ownership here as requested */
typedef struct { uint32 u; } IfxGtm_Pwm_ToutMap;

/* Enums first */
typedef enum { IfxGtm_Pwm_Alignment_edge = 0, IfxGtm_Pwm_Alignment_center } IfxGtm_Pwm_Alignment;

typedef enum { IfxGtm_Pwm_SubModule_tom = 0, IfxGtm_Pwm_SubModule_atom = 1 } IfxGtm_Pwm_SubModule;

typedef enum { IfxGtm_Pwm_SubModule_Ch_0 = 0, IfxGtm_Pwm_SubModule_Ch_1, IfxGtm_Pwm_SubModule_Ch_2, IfxGtm_Pwm_SubModule_Ch_3, IfxGtm_Pwm_SubModule_Ch_4, IfxGtm_Pwm_SubModule_Ch_5, IfxGtm_Pwm_SubModule_Ch_6, IfxGtm_Pwm_SubModule_Ch_7 } IfxGtm_Pwm_SubModule_Ch;

typedef enum { IfxGtm_Pwm_State_stopped = 0, IfxGtm_Pwm_State_running } IfxGtm_Pwm_State;

/* Simple structs and config helpers */
typedef struct { IfxGtm_Dtm_ClockSource clockSource; } IfxGtm_Pwm_DtmConfig;

typedef struct { uint8 irqPriority; IfxSrc_Tos isrProvider; } IfxGtm_Pwm_InterruptConfig;

typedef struct {
    IfxPort_OutputMode outputMode;
    IfxPort_PadDriver  padDriver;
} IfxGtm_Pwm_OutputConfig;

typedef struct { uint8 port; uint8 pin; } IfxGtm_Pwm_Pin;

/* Channel and ChannelConfig (complete fields as required) */
typedef struct {
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    float32                      phase;
    float32                      duty;
    IfxGtm_Pwm_DtmConfig       *dtm;
    IfxGtm_Pwm_OutputConfig    *output;
    void                        *mscOut;
    IfxGtm_Pwm_InterruptConfig *interrupt;
} IfxGtm_Pwm_ChannelConfig;

typedef struct {
    IfxGtm_Pwm_SubModule_Ch ch;
} IfxGtm_Pwm_Channel;

/* ClockSource union (uint32 fields to avoid enum-conversion issues) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxGtm_Pwm_ClockSource;

/* Complete Config struct as required */
typedef struct {
    Ifx_GTM                   *gtmSfr;
    IfxGtm_Pwm_SubModule       subModule;
    IfxGtm_Cluster             cluster;
    IfxGtm_Pwm_ClockSource     clockSource;
    IfxGtm_Pwm_Alignment       alignment;
    float32                    frequency;
    IfxGtm_Pwm_ChannelConfig  *channels;
    uint32                     numChannels;
    boolean                    syncStart;
    boolean                    syncUpdateEnabled;
    IfxGtm_Dtm_ClockSource     dtmClockSource;
} IfxGtm_Pwm_Config;

/* Driver handle (minimal) */
typedef struct { IfxGtm_Pwm_State state; } IfxGtm_Pwm;

/* Function declarations used by tests/build */
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *config);

#endif /* IFXGTM_PWM_H */
