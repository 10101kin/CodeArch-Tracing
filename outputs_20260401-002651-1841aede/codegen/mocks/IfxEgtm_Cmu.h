#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "IfxEgtm.h"

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

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM * egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count);
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM * egtm, uint32 numerator, uint32 denominator);
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM * egtm, uint32 clkMask);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM * egtm);

#endif /* IFXEGTM_CMU_H */
