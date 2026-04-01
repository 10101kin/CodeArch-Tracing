#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

/* from IfxEgtm.h */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_ClApConfig;

/* from IfxEgtm.h */
typedef struct
{
    IfxApProt_ProtConfig proteConfig;       /**< \brief PROTE configuration */
    IfxApApu_ApuConfig   apuConfig;         /**< \brief APU configuration */
} IfxEgtm_CtrlApConfig;

/* from IfxEgtm.h */
typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU configuration */
} IfxEgtm_WrapApConfig;

/* from IfxEgtm.h */
typedef struct
{
    IfxApProt_ProtConfig protseConfig;                              /**< \brief PROTSE configuration */
    IfxEgtm_ClApConfig   clApConfig[IFXEGTM_NUM_CCM_OBJECTS];       /**< \brief Cluster APU configuration */
    IfxEgtm_CtrlApConfig ctrlApConfig;                              /**< \brief Ctrl PROT and APU configuration */
    IfxEgtm_WrapApConfig wrapApConfig;                              /**< \brief Wrap APU configuration */
} IfxEgtm_ApConfig;

/* from IfxEgtm.h */
typedef struct
{
    IfxEgtm_Cfg_MscSet       mscSet;             /**< \brief MSC set value */
    IfxEgtm_Cfg_MscSetSignal mscSetSignal;       /**< \brief MSC set signal */
    IfxEgtm_Cfg_MscModule    mscModule;          /**< \brief MSC Module */
    IfxEgtm_Cfg_MscSelect    mscSelect;          /**< \brief MSC Select */
    IfxEgtm_MscAltInput      mscAltIn;           /**< \brief MSC Alt Input selection */
} IfxEgtm_MscOut;

/* from IfxEgtm.h */
typedef enum
{
    IfxEgtm_AeiBridgeOpMode_sync  = 0u, /**< \brief AEI bridge operates in sync_bridge mode */
    IfxEgtm_AeiBridgeOpMode_async = 1u  /**< \brief AEI bridge operates in async_bridge mode */
} IfxEgtm_AeiBridgeOpMode;

/* from IfxEgtm.h */
typedef enum
{
    IfxEgtm_ClusterClockDiv_disable    = 0u, /**< \brief Cluster is disabled */
    IfxEgtm_ClusterClockDiv_enable     = 1u, /**< \brief Cluster is enabled without clock divider */
    IfxEgtm_ClusterClockDiv_enableDiv2 = 2u  /**< \brief Cluster is enabled with clock divider 2 */
} IfxEgtm_ClusterClockDiv;

/* from IfxEgtm.h */
typedef enum
{
    IfxEgtm_IrqMode_level       = 0,
    IfxEgtm_IrqMode_pulse       = 1,
    IfxEgtm_IrqMode_pulseNotify = 2,
    IfxEgtm_IrqMode_singlePulse = 3
} IfxEgtm_IrqMode;

/* from IfxEgtm.h */
typedef enum
{
    IfxEgtm_MscAltInput_low     = 0,
    IfxEgtm_MscAltInput_lowext  = 1,
    IfxEgtm_MscAltInput_high    = 2,
    IfxEgtm_MscAltInput_highext = 3
} IfxEgtm_MscAltInput;

/* from IfxEgtm.h */
typedef enum
{
    IfxEgtm_SuspendMode_none = 0,  /**< \brief No suspend */
    IfxEgtm_SuspendMode_hard = 1,  /**< \brief Hard Suspend */
    IfxEgtm_SuspendMode_soft = 2   /**< \brief Soft Suspend */
} IfxEgtm_SuspendMode;

/* from IfxEgtm.h */
typedef enum {
    IfxEgtm_AeiBridgeOpMode_sync,
    IfxEgtm_AeiBridgeOpMode_async
} IfxEgtm_AeiBridgeOpMode;

/* from IfxEgtm.h */
typedef enum {
    IfxEgtm_ClusterClockDiv_disable,
    IfxEgtm_ClusterClockDiv_enable,
    IfxEgtm_ClusterClockDiv_enableDiv2
} IfxEgtm_ClusterClockDiv;

/* from IfxEgtm.h */
typedef enum {
    IfxEgtm_IrqMode_level,
    IfxEgtm_IrqMode_pulse,
    IfxEgtm_IrqMode_pulseNotify,
    IfxEgtm_IrqMode_singlePulse
} IfxEgtm_IrqMode;

/* from IfxEgtm.h */
typedef enum {
    IfxEgtm_MscAltInput_low,
    IfxEgtm_MscAltInput_lowext,
    IfxEgtm_MscAltInput_high,
    IfxEgtm_MscAltInput_highext
} IfxEgtm_MscAltInput;

/* from IfxEgtm.h */
typedef enum {
    IfxEgtm_SuspendMode_none,
    IfxEgtm_SuspendMode_hard,
    IfxEgtm_SuspendMode_soft
} IfxEgtm_SuspendMode;

typedef enum {
    IfxEgtm_Cmu_Clk_0,
    IfxEgtm_Cmu_Clk_1,
    IfxEgtm_Cmu_Clk_2,
    IfxEgtm_Cmu_Clk_3,
    IfxEgtm_Cmu_Clk_4,
    IfxEgtm_Cmu_Clk_5,
    IfxEgtm_Cmu_Clk_6,
    IfxEgtm_Cmu_Clk_7
} IfxEgtm_Cmu_Clk;

typedef enum {
    IfxEgtm_Cmu_Eclk_0,
    IfxEgtm_Cmu_Eclk_1,
    IfxEgtm_Cmu_Eclk_2
} IfxEgtm_Cmu_Eclk;

typedef enum {
    IfxEgtm_Cmu_Fxclk_0,
    IfxEgtm_Cmu_Fxclk_1,
    IfxEgtm_Cmu_Fxclk_2,
    IfxEgtm_Cmu_Fxclk_3,
    IfxEgtm_Cmu_Fxclk_4
} IfxEgtm_Cmu_Fxclk;

typedef enum {
    IfxEgtm_Cmu_Tim_Filter_Clk_0,
    IfxEgtm_Cmu_Tim_Filter_Clk_1,
    IfxEgtm_Cmu_Tim_Filter_Clk_6
} IfxEgtm_Cmu_Tim_Filter_Clk;

typedef enum
{
    IfxEgtm_Cmu_Clk_0 = 0,
    IfxEgtm_Cmu_Clk_1,
    IfxEgtm_Cmu_Clk_2,
    IfxEgtm_Cmu_Clk_3,
    IfxEgtm_Cmu_Clk_4,
    IfxEgtm_Cmu_Clk_5,
    IfxEgtm_Cmu_Clk_6,
    IfxEgtm_Cmu_Clk_7
} IfxEgtm_Cmu_Clk;

typedef enum
{
    IfxEgtm_Cmu_Eclk_0 = 0,
    IfxEgtm_Cmu_Eclk_1,
    IfxEgtm_Cmu_Eclk_2
} IfxEgtm_Cmu_Eclk;

typedef enum
{
    IfxEgtm_Cmu_Fxclk_0 = 0,
    IfxEgtm_Cmu_Fxclk_1,
    IfxEgtm_Cmu_Fxclk_2,
    IfxEgtm_Cmu_Fxclk_3,
    IfxEgtm_Cmu_Fxclk_4
} IfxEgtm_Cmu_Fxclk;

typedef enum
{
    IfxEgtm_Cmu_Tim_Filter_Clk_0,  /**< \brief FLT_CNT counts with CMU_CLK0 */
    IfxEgtm_Cmu_Tim_Filter_Clk_1,  /**< \brief FLT_CNT counts with CMU_CLK1 */
    IfxEgtm_Cmu_Tim_Filter_Clk_6,  /**< \brief FLT_CNT counts with CMU_CLK6 */
    IfxEgtm_Cmu_Tim_Filter_Clk_7   /**< \brief FLT_CNT counts with CMU_CLK7 */
} IfxEgtm_Cmu_Tim_Filter_Clk;

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM * egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM * egtm);
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM * egtm, uint32 numerator, uint32 denominator);
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM * egtm, uint32 clkMask);

#endif /* IFXEGTM_CMU_H */
