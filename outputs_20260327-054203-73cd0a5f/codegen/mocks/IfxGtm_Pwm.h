#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"

/* Enums for Pwm driver */
typedef enum {
    IfxGtm_Pwm_Alignment_edge = 0,
    IfxGtm_Pwm_Alignment_center
} IfxGtm_Pwm_Alignment;

typedef enum {
    IfxGtm_Pwm_SubModule_atom = 0,
    IfxGtm_Pwm_SubModule_tom  = 1
} IfxGtm_Pwm_SubModule;

typedef enum {
    IfxGtm_Pwm_SubModule_Ch_0 = 0,
    IfxGtm_Pwm_SubModule_Ch_1,
    IfxGtm_Pwm_SubModule_Ch_2,
    IfxGtm_Pwm_SubModule_Ch_3,
    IfxGtm_Pwm_SubModule_Ch_4,
    IfxGtm_Pwm_SubModule_Ch_5,
    IfxGtm_Pwm_SubModule_Ch_6,
    IfxGtm_Pwm_SubModule_Ch_7,
    IfxGtm_Pwm_SubModule_Ch_8,
    IfxGtm_Pwm_SubModule_Ch_9,
    IfxGtm_Pwm_SubModule_Ch_10,
    IfxGtm_Pwm_SubModule_Ch_11,
    IfxGtm_Pwm_SubModule_Ch_12,
    IfxGtm_Pwm_SubModule_Ch_13,
    IfxGtm_Pwm_SubModule_Ch_14,
    IfxGtm_Pwm_SubModule_Ch_15
} IfxGtm_Pwm_SubModule_Ch;

typedef enum {
    IfxGtm_Pwm_State_stopped = 0,
    IfxGtm_Pwm_State_running
} IfxGtm_Pwm_State;

/* Avoid cross-enum conversion issues: union fields use uint32 */
typedef union {
    uint32 atom; /* accept IfxGtm_Cmu_Fxclk_x or IfxGtm_Cmu_Clk_x */
    uint32 tom;  /* accept IfxGtm_Cmu_Clk_x */
} IfxGtm_Pwm_ClockSource;

/* Basic structures referenced by ChannelConfig */
typedef struct {
    uint32 dummy;
} IfxGtm_Pwm_DtmConfig;

typedef struct {
    uint32 dummy;
} IfxGtm_Pwm_InterruptConfig;

typedef struct {
    uint32 dummy;
} IfxGtm_Pwm_OutputConfig;

/* ChannelConfig completeness */
typedef struct {
    IfxGtm_Pwm_SubModule_Ch     timerCh;
    float32                     phase;
    float32                     duty;
    IfxGtm_Pwm_DtmConfig       *dtm;
    IfxGtm_Pwm_OutputConfig    *output;
    void                       *mscOut;     /* MSC config pointer */
    IfxGtm_Pwm_InterruptConfig *interrupt;
} IfxGtm_Pwm_ChannelConfig;

/* ToutMap type used by production/pin symbols */
typedef struct {
    uint32 module;  /* atom/tom id */
    uint32 channel; /* channel id */
    uint32 tout;    /* TOUT index */
} IfxGtm_Pwm_ToutMap;

/* Config completeness */
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

/* Common missing stub often required by builds */
void IfxGtm_Pwm_initChannelConfig(void *cfg);

#endif /* IFXGTM_PWM_H */
