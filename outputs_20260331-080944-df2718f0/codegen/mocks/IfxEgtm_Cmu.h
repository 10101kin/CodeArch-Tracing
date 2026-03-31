#ifndef IFXEGTM_CMU_H
#define IFXEGTM_CMU_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Enums (order first) */
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
    IfxEgtm_Cmu_Tim_Filter_Clk_0 = 0,
    IfxEgtm_Cmu_Tim_Filter_Clk_1 = 1,
    IfxEgtm_Cmu_Tim_Filter_Clk_6 = 6,
    IfxEgtm_Cmu_Tim_Filter_Clk_7 = 7
} IfxEgtm_Cmu_Tim_Filter_Clk;

/* Function declarations per template mapping and mandatory list */
void    IfxEgtm_Cmu_enable(Ifx_EGTM *module);
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module);

void    IfxEgtm_Cmu_enableClocks(Ifx_EGTM *module, uint32 mask);
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *module);
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *module);
boolean IfxEgtm_Cmu_isClkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk);
boolean IfxEgtm_Cmu_isEclkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk);
boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx);
void    IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, uint32 input);
void    IfxEgtm_Cmu_setClkCount(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, uint32 count);
void    IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, uint32 div);
void    IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *module, uint32 numerator, uint32 denominator);
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk);
float32 IfxEgtm_Cmu_getEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk);
float32 IfxEgtm_Cmu_getFxClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx);
void    IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency);
void    IfxEgtm_Cmu_setEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, float32 frequency);
/* For TC4xx mandatory: frequency-based setGclkFrequency */
void    IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *module, float32 frequency);

#endif /* IFXEGTM_CMU_H */
