#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* CMU clock enable macros */
#ifndef IFXEGTM_CMU_CLKEN_FXCLK
#define IFXEGTM_CMU_CLKEN_FXCLK (1u << 0)
#endif
#ifndef IFXEGTM_CMU_CLKEN_CLK0
#define IFXEGTM_CMU_CLKEN_CLK0  (1u << 8)
#endif

/* Enums as required */
typedef enum { IfxEgtm_Cmu_Clk_0 = 0, IfxEgtm_Cmu_Clk_1, IfxEgtm_Cmu_Clk_2, IfxEgtm_Cmu_Clk_3, IfxEgtm_Cmu_Clk_4, IfxEgtm_Cmu_Clk_5, IfxEgtm_Cmu_Clk_6, IfxEgtm_Cmu_Clk_7 } IfxEgtm_Cmu_Clk;
typedef enum { IfxEgtm_Cmu_Eclk_0 = 0, IfxEgtm_Cmu_Eclk_1, IfxEgtm_Cmu_Eclk_2 } IfxEgtm_Cmu_Eclk;
typedef enum { IfxEgtm_Cmu_Fxclk_0 = 0, IfxEgtm_Cmu_Fxclk_1, IfxEgtm_Cmu_Fxclk_2, IfxEgtm_Cmu_Fxclk_3, IfxEgtm_Cmu_Fxclk_4 } IfxEgtm_Cmu_Fxclk;
/* Tim filter clock: per requirement include 3 values (0,1,6) */
typedef enum { IfxEgtm_Cmu_Tim_Filter_Clk_0 = 0, IfxEgtm_Cmu_Tim_Filter_Clk_1 = 1, IfxEgtm_Cmu_Tim_Filter_Clk_6 = 6 } IfxEgtm_Cmu_Tim_Filter_Clk;

/* Function declarations (subset to mock) */
void    IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask);
void    IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm);
void    IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, uint32 numerator, uint32 denominator);

#endif /* IFXEGTM_CMU_H */
