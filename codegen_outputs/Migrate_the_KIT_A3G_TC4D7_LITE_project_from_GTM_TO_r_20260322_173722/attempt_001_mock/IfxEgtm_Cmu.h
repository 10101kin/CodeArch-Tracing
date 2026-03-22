#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "illd_types/Ifx_Types.h"

/* ============= Type Definitions ============= */
typedef enum {
    IfxEgtm_Cmu_Clk_0 = 0,
    IfxEgtm_Cmu_Clk_1 = 1,
    IfxEgtm_Cmu_Clk_2 = 2
} IfxEgtm_Cmu_Clk;

/* Minimal enum for CMU clock index */
/* Mock controls */
/* EGTM CMU Clock Enable Macros (auto-injected for TDD) */
#define IFXEGTM_CMU_CLKEN_CLK0   (1U << 0)
#define IFXEGTM_CMU_CLKEN_CLK1   (1U << 1)
#define IFXEGTM_CMU_CLKEN_CLK2   (1U << 2)
#define IFXEGTM_CMU_CLKEN_CLK3   (1U << 3)
#define IFXEGTM_CMU_CLKEN_CLK4   (1U << 4)
#define IFXEGTM_CMU_CLKEN_CLK5   (1U << 5)
#define IFXEGTM_CMU_CLKEN_CLK6   (1U << 6)
#define IFXEGTM_CMU_CLKEN_CLK7   (1U << 7)
#define IFXEGTM_CMU_CLKEN_FXCLK  (1U << 8)
#define IFXEGTM_CMU_CLKEN_ALL    (0x1FFU)
/* EGTM CMU frequency functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask);
void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean useGlobal);
void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count);
uint32 IfxEgtm_Cmu_Mock_GetCallCount_enableClocks(void);
uint32 IfxEgtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void);
uint32 IfxEgtm_Cmu_Mock_GetCallCount_selectClkInput(void);
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_clkIndex(void);
uint32 IfxEgtm_Cmu_Mock_GetLastArg_selectClkInput_useGlobal(void);
uint32 IfxEgtm_Cmu_Mock_GetCallCount_setClkCount(void);
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setClkCount_clkIndex(void);
uint32 IfxEgtm_Cmu_Mock_GetLastArg_setClkCount_count(void);
void   IfxEgtm_Cmu_Mock_Reset(void);
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm);
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency);
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask);

#endif