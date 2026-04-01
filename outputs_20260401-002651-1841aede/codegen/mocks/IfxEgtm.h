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
    IfxApProt_ProtConfig protseConfig;                              /**< \brief PROTSE configuration */
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];       /**< \brief Cluster APU configuration */
    IfxEgtm_CtrlApConfig ctrlApConfig;                              /**< \brief Ctrl PROT and APU configuration */
    IfxEgtm_WrapApConfig wrapApConfig;                              /**< \brief Wrap APU configuration */
} IfxEgtm_ApConfig;

typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             /**< \brief MSC set value */
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       /**< \brief MSC set signal */
    IfxEgtm_Cfg_MscModule    mscModule;          /**< \brief MSC Module */
    IfxEgtm_Cfg_MscSelect    mscSelect;          /**< \brief MSC Select */
    IfxEgtm_MscAltInput      mscAltIn;           /**< \brief MSC Alt Input selection */
} IfxEgtm_MscOut;

typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, /**< \brief AEI bridge operates in sync_bridge mode */
    IfxEgtm_AeiBridgeOpMode_async = 1u  /**< \brief AEI bridge operates in async_bridge mode */
} IfxEgtm_AeiBridgeOpMode;

typedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u, /**< \brief Cluster is disabled */
    IfxEgtm_ClusterClockDiv_enable     = 1u, /**< \brief Cluster is enabled without clock divider */
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u  /**< \brief Cluster is enabled with clock divider 2 */
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
    IfxEgtm_SuspendMode_none = 0,  /**< \brief No suspend */
    IfxEgtm_SuspendMode_hard = 1,  /**< \brief Hard Suspend */
    IfxEgtm_SuspendMode_soft = 2   /**< \brief Soft Suspend */
} IfxEgtm_SuspendMode;

void IfxEgtm_enable(Ifx_EGTM * egtm);
boolean IfxEgtm_isEnabled(Ifx_EGTM * egtm);

#endif /* IFXEGTM_H */
