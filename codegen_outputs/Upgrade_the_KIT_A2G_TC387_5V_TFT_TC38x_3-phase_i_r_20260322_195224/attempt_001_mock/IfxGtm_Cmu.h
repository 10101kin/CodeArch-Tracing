#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H
#ifndef IFX_GTM_CMU_ECLK_DEFINED

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
/* IfxGtm_Cmu_Eclk - from illd_types/Ifx_Types.h */

/* Mock controls */
/* ============= GTM CMU Enums (MUST be before function declarations) ============= */
/* DUPLICATE IfxGtm_Cmu_Clk REMOVED - defined in illd_types/Ifx_Types.h */
/* DUPLICATE IfxGtm_Cmu_Eclk REMOVED - defined in illd_types/Ifx_Types.h */
/* DUPLICATE IfxGtm_Cmu_Fxclk REMOVED - defined in illd_types/Ifx_Types.h */
/* IfxGtm_Cmu_Eclk only if not already in Ifx_Types.h */
/* ECLK guard fixed - see enum below */
#define IFX_GTM_CMU_ECLK_DEFINED
#define IFX_GTM_CMU_ECLK_DEFINED
/* GTM CMU Clock Enable Macros */
#define IFXGTM_CMU_CLKEN_CLK0   (1U << 0)
#define IFXGTM_CMU_CLKEN_CLK1   (1U << 1)
#define IFXGTM_CMU_CLKEN_CLK2   (1U << 2)
#define IFXGTM_CMU_CLKEN_CLK3   (1U << 3)
#define IFXGTM_CMU_CLKEN_CLK4   (1U << 4)
#define IFXGTM_CMU_CLKEN_CLK5   (1U << 5)
#define IFXGTM_CMU_CLKEN_CLK6   (1U << 6)
#define IFXGTM_CMU_CLKEN_CLK7   (1U << 7)
#define IFXGTM_CMU_CLKEN_FXCLK  (1U << 8)
#define IFXGTM_CMU_CLKEN_ALL    (0x1FFU)
/* Legacy macro names (used in some examples) */
#define IFXGTM_CMU_CLK0         IFXGTM_CMU_CLKEN_CLK0
/* GTM CMU frequency getter/setter functions (auto-injected for TDD) */

/* ============= Function Declarations ============= */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
uint32 IfxGtm_Cmu_Mock_GetCallCount_enableClocks(void);
uint32 IfxGtm_Cmu_Mock_GetLastArg_enableClocks_clkMask(void);
void   IfxGtm_Cmu_Mock_Reset(void);
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, boolean assumeEnabled);
float32 IfxGtm_Cmu_getFxclkFrequency(Ifx_GTM *gtm, uint8 fxclkIndex, boolean assumeEnabled);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);
void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency);
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);

#endif /* IFXGTM_CMU_H */