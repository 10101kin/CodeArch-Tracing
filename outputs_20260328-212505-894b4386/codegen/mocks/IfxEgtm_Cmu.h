/* IfxEgtm_Cmu.h - CMU mock */
#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxPort.h"

/* Enums */
typedef enum {
    IfxEgtm_Cmu_Clk_0 = 0,
    IfxEgtm_Cmu_Clk_1,
    IfxEgtm_Cmu_Clk_2,
    IfxEgtm_Cmu_Clk_3,
    IfxEgtm_Cmu_Clk_4,
    IfxEgtm_Cmu_Clk_5,
    IfxEgtm_Cmu_Clk_6,
    IfxEgtm_Cmu_Clk_7
} IfxEgtm_Cmu_Clk;

typedef enum {
    IfxEgtm_Cmu_Eclk_0 = 0,
    IfxEgtm_Cmu_Eclk_1,
    IfxEgtm_Cmu_Eclk_2
} IfxEgtm_Cmu_Eclk;

typedef enum {
    IfxEgtm_Cmu_Fxclk_0 = 0,
    IfxEgtm_Cmu_Fxclk_1,
    IfxEgtm_Cmu_Fxclk_2,
    IfxEgtm_Cmu_Fxclk_3,
    IfxEgtm_Cmu_Fxclk_4
} IfxEgtm_Cmu_Fxclk;

typedef enum {
    IfxEgtm_Cmu_Tim_Filter_Clk_0 = 0,
    IfxEgtm_Cmu_Tim_Filter_Clk_1,
    IfxEgtm_Cmu_Tim_Filter_Clk_6,
    IfxEgtm_Cmu_Tim_Filter_Clk_7
} IfxEgtm_Cmu_Tim_Filter_Clk;

/* CLKEN macros */
#ifndef IFXEGTM_CMU_CLKEN_FXCLK
#define IFXEGTM_CMU_CLKEN_FXCLK (0x1u)
#endif
#ifndef IFXEGTM_CMU_CLKEN_CLK0
#define IFXEGTM_CMU_CLKEN_CLK0  (0x2u)
#endif

/* Function declarations */
void    IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask);
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *egtm);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm);
boolean IfxEgtm_Cmu_isClkClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk);
boolean IfxEgtm_Cmu_isEclkClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk eclk);
boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *egtm, IfxEgtm_Cmu_Fxclk fx);
void    IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, uint32 input);
void    IfxEgtm_Cmu_setClkCount(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clk, uint32 count);
void    IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, uint32 numerator, uint32 denominator);
void    IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *egtm, uint32 numerator, uint32 denominator);
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, boolean assumeEnabled);
float32 IfxEgtm_Cmu_getEclkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex);
float32 IfxEgtm_Cmu_getFxClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Fxclk clkIndex);
void    IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency);
void    IfxEgtm_Cmu_setEclkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Eclk clkIndex, float32 frequency);
void    IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency);

#endif /* IFXEGTM_CMU_H */
