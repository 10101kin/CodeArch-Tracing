/*
 * IfxEgtm.h - EGTM base peripheral mock header
 */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* ==========================================================
 * Enums (must precede structs)
 * ========================================================== */
typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u,
    IfxEgtm_AeiBridgeOpMode_async = 1u
} IfxEgtm_AeiBridgeOpMode;

typedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;

typedef enum
{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

typedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

typedef enum
{
    IfxEgtm_SuspendMode_none = 0,
    IfxEgtm_SuspendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;

/* ==========================================================
 * Helper peer-type shells required by verified struct layout
 * ========================================================== */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;
typedef uint32 IfxEgtm_Cfg_MscSet;
typedef uint32 IfxEgtm_Cfg_MscSetSignal;
typedef uint32 IfxEgtm_Cfg_MscModule;
typedef uint32 IfxEgtm_Cfg_MscSelect;

/* ==========================================================
 * Verified struct layouts (order preserved)
 * ========================================================== */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /* APU configuration */
} IfxEgtm_ClApConfig;

typedef struct
{
    IfxApProt_ProtConfig proteConfig;   /* PROTE configuration */
    IfxApApu_ApuConfig   apuConfig;     /* APU configuration */
} IfxEgtm_CtrlApConfig;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /* APU configuration */
} IfxEgtm_WrapApConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;  /* PROTSE configuration */
    IfxEgtm_ClApConfig   clApConfig[1]; /* IFXEGTM_NUM_CCM_OBJECTS (mocked as 1) */
    IfxEgtm_CtrlApConfig ctrlApConfig;  /* Ctrl PROT and APU configuration */
    IfxEgtm_WrapApConfig wrapApConfig;  /* Wrap APU configuration */
} IfxEgtm_ApConfig;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;       /* MSC set value */
    IfxEgtm_Cfg_MscSetSignal mscSetSignal; /* MSC set signal */
    IfxEgtm_Cfg_MscModule    mscModule;    /* MSC Module */
    IfxEgtm_Cfg_MscSelect    mscSelect;    /* MSC Select */
    IfxEgtm_MscAltInput      mscAltIn;     /* MSC Alt Input selection */
} IfxEgtm_MscOut;

/* ==========================================================
 * Function declarations (subset required by drivers-to-mock)
 * ========================================================== */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
