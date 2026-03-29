/*
 * IfxEgtm_Cmu.h - EGTM CMU mock
 */
#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* CMU enums (verified) */
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
    IfxEgtm_Cmu_Tim_Filter_Clk_0,
    IfxEgtm_Cmu_Tim_Filter_Clk_1,
    IfxEgtm_Cmu_Tim_Filter_Clk_6,
    IfxEgtm_Cmu_Tim_Filter_Clk_7
} IfxEgtm_Cmu_Tim_Filter_Clk;

/* CMU CLKEN macros */
#ifndef IFXEGTM_CMU_CLKEN_FXCLK
#define IFXEGTM_CMU_CLKEN_FXCLK (1u << 0)
#endif
#ifndef IFXEGTM_CMU_CLKEN_CLK0
#define IFXEGTM_CMU_CLKEN_CLK0  (1u << 1)
#endif

/* Functions to mock from IfxEgtm_Cmu */
void    IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm);
void    IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator);
void    IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count);
/* Additional getters preserved for test compatibility (from previous mocks) */
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled);

#endif /* IFXEGTM_CMU_H */
