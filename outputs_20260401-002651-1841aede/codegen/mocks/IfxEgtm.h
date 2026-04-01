#ifndef IFXEGTM_H
#define IFXEGTM_H


typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync,
    IfxEgtm_AeiBridgeOpMode_async
} IfxEgtm_AeiBridgeOpMode;

typedef enum {
    IfxEgtm_ClusterClockDiv_disable,
    IfxEgtm_ClusterClockDiv_enable,
    IfxEgtm_ClusterClockDiv_enableDiv2
} IfxEgtm_ClusterClockDiv;

typedef enum {
    IfxEgtm_IrqMode_level,
    IfxEgtm_IrqMode_pulse,
    IfxEgtm_IrqMode_pulseNotify,
    IfxEgtm_IrqMode_singlePulse
} IfxEgtm_IrqMode;

typedef enum {
    IfxEgtm_MscAltInput_low,
    IfxEgtm_MscAltInput_lowext,
    IfxEgtm_MscAltInput_high,
    IfxEgtm_MscAltInput_highext
} IfxEgtm_MscAltInput;

typedef enum {
    IfxEgtm_SuspendMode_none,
    IfxEgtm_SuspendMode_hard,
    IfxEgtm_SuspendMode_soft
} IfxEgtm_SuspendMode;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_ClApConfig;

typedef struct
{
    IfxApProt_ProtConfig proteConfig;       /**< \brief PROTE configuration */
    IfxApApu_ApuConfig   apuConfig;         /**< \brief APU configuration */
} IfxEgtm_CtrlApConfig;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_WrapApConfig;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             /**< \brief MSC set value */
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       /**< \brief MSC set signal */
    IfxEgtm_Cfg_MscModule    mscModule;          /**< \brief MSC Module */
    IfxEgtm_Cfg_MscSelect    mscSelect;          /**< \brief MSC Select */
    IfxEgtm_MscAltInput      mscAltIn;           /**< \brief MSC Alt Input selection */
} IfxEgtm_MscOut;

void IfxEgtm_enable(Ifx_EGTM * egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM * egtm);

#endif /* IFXEGTM_H */
