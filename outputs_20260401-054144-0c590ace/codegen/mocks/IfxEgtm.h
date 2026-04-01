/* IfxEgtm mock */
#ifndef IFXEGTM_H
#define IFXEGTM_H
#include "mock_egtm_atom_tmadc_consolidated.h"

/* Minimal SFR sub-structure stubs used by other headers */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM_AGC;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_CDTM_DTM;

typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync = 0,
    IfxEgtm_AeiBridgeOpMode_async = 1
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable = 0,
    IfxEgtm_ClusterClockDiv_enable = 1,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2
} IfxEgtm_ClusterClockDiv;

typedef enum {
    IfxEgtm_IrqMode_level = 0,
    IfxEgtm_IrqMode_pulse = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

typedef enum {
    IfxEgtm_MscAltInput_low = 0,
    IfxEgtm_MscAltInput_lowext = 1,
    IfxEgtm_MscAltInput_high = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef enum {
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

typedef enum {
    IfxEgtm_Cluster_0 = 0,
    IfxEgtm_Cluster_1 = 1,
    IfxEgtm_Cluster_2 = 2
} IfxEgtm_Cluster;

/* Additional minimal enums for DTM used by PWM types */
typedef enum {
    IfxEgtm_Dtm_ShutoffInput_a = 0,
    IfxEgtm_Dtm_ShutoffInput_b = 1
} IfxEgtm_Dtm_ShutoffInput;

typedef enum {
    IfxEgtm_Dtm_SignalLevel_low = 0,
    IfxEgtm_Dtm_SignalLevel_high = 1
} IfxEgtm_Dtm_SignalLevel;

/* Minimal EGTM MSC out struct referenced by PWM channel config */
typedef struct {
    uint32 dummy;
} IfxEgtm_MscOut;

/* Minimal Atom channel and DTM types for Atom_Timer */
typedef enum {
    IfxEgtm_Atom_Ch_0 = 0,
    IfxEgtm_Atom_Ch_1 = 1
} IfxEgtm_Atom_Ch;

typedef enum {
    IfxEgtm_Atom_Ch_ClkSrc_cmuClk0 = 0,
    IfxEgtm_Atom_Ch_ClkSrc_cmuClk1 = 1
} IfxEgtm_Atom_Ch_ClkSrc;

typedef enum {
    IfxEgtm_Dtm_ClockSource_systemClock = 0,
    IfxEgtm_Dtm_ClockSource_cmuClock0 = 1,
    IfxEgtm_Dtm_ClockSource_cmuClock1 = 2,
    IfxEgtm_Dtm_ClockSource_cmuClock2 = 3
} IfxEgtm_Dtm_ClockSource;

typedef enum {
    IfxEgtm_Dtm_Ch_0 = 0,
    IfxEgtm_Dtm_Ch_1 = 1
} IfxEgtm_Dtm_Ch;

/* Trigger-related minimal enums */
typedef enum {
    IfxEgtm_TrigSource_0 = 0,
    IfxEgtm_TrigSource_1 = 1
} IfxEgtm_TrigSource;

typedef enum {
    IfxEgtm_TrigChannel_0 = 0,
    IfxEgtm_TrigChannel_1 = 1
} IfxEgtm_TrigChannel;

typedef enum {
    IfxEgtm_Cfg_AdcTriggerSignal_0 = 0,
    IfxEgtm_Cfg_AdcTriggerSignal_1 = 1
} IfxEgtm_Cfg_AdcTriggerSignal;

/* Functions */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
