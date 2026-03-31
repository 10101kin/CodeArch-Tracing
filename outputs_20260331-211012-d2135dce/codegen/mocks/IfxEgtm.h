#ifndef IFXEGTM_H
#define IFXEGTM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Placeholder external types used in structs */
typedef struct
{
    unsigned long wraTagId;
    unsigned long wrbTagId;
    unsigned long rdaTagId;
    unsigned long rdbTagId;
    unsigned char vmWrId;
    unsigned char vmRdId;
    unsigned char prsWrId;
    unsigned char prsRdId;
} IfxApApu_ApuConfig;{ uint32 dummy; } IfxApProt_ProtConfig;

/* Additional enums */
typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, 
    IfxEgtm_AeiBridgeOpMode_async = 1u  
} IfxEgtm_AeiBridgeOpMode;{
    IfxEgtm_ClusterClockDiv_disable    = 0u,
    IfxEgtm_ClusterClockDiv_enable     = 1u,
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u
} IfxEgtm_ClusterClockDiv;{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       
} IfxEgtm_ClApConfig;spendMode_hard = 1,
    IfxEgtm_SuspendMode_soft = 2
} IfxEgtm_SuspendMode;
#define IFXEGTM_NUM_CCM_OBJECTS 3
#endif

/* Structs */
typedef struct {
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_ClApConfig;{
    IfxApProt_ProtConfig proteConfig;
    IfxApApu_ApuConfig   apuConfig;
} IfxEgtm_CtrlApConfig;{
    IfxApApu_ApuConfig apuConfig;
} IfxEgtm_WrapApConfig;{
    IfxApProt_ProtConfig protseConfig;
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];
    IfxEgtm_CtrlApConfig ctrlApConfig;
    IfxEgtm_WrapApConfig wrapApConfig;
} IfxEgtm_ApConfig;{
    uint32 dummy; /* MSC fields omitted in mock */
} IfxEgtm_MscOut;

/* Functions (subset) */
void    IfxEgtm_enable(Ifx_EGTM *egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm);

#endif /* IFXEGTM_H */
