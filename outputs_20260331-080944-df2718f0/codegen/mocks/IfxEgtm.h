/* Mock IfxEgtm.h (TC4xx EGTM base) */
#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Peer typedefs/macros needed by verified structs */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS 2u
#endif

typedef uint32 IfxEgtm_Cfg_MscSet;
typedef uint32 IfxEgtm_Cfg_MscSetSignal;
typedef uint32 IfxEgtm_Cfg_MscModule;
typedef uint32 IfxEgtm_Cfg_MscSelect;

/* Enums */
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

/* Verified struct definitions (with peer placeholders above) */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_ClApConfig;

typedef struct
{
    IfxApProt_ProtConfig proteConfig;   /**< \brief PROTE configuration */
    IfxApApu_ApuConfig   apuConfig;     /**< \brief APU configuration */
} IfxEgtm_CtrlApConfig;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_WrapApConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;                              /**< \brief PROTSE configuration */
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];       /**< \brief Cluster APU configuration */
    IfxEgtm_CtrlApConfig ctrlApConfig;                              /**< \brief Ctrl PROT and APU configuration */
    IfxEgtm_WrapApConfig wrapApConfig;                              /**< \brief Wrap APU configuration */
} IfxEgtm_ApConfig;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;        /**< \brief MSC set value */
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;  /**< \brief MSC set signal */
    IfxEgtm_Cfg_MscModule    mscModule;     /**< \brief MSC Module */
    IfxEgtm_Cfg_MscSelect    mscSelect;     /**< \brief MSC Select */
    IfxEgtm_MscAltInput      mscAltIn;      /**< \brief MSC Alt Input selection */
} IfxEgtm_MscOut;

/* Function declarations (subset required by module/tests) */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
