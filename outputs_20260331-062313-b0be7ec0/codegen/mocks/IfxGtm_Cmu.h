#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* CMU enums */
typedef enum {
    IfxGtm_Cmu_Clk_0 = 0,
    IfxGtm_Cmu_Clk_1,
    IfxGtm_Cmu_Clk_2,
    IfxGtm_Cmu_Clk_3,
    IfxGtm_Cmu_Clk_4,
    IfxGtm_Cmu_Clk_5,
    IfxGtm_Cmu_Clk_6,
    IfxGtm_Cmu_Clk_7
} IfxGtm_Cmu_Clk;

typedef enum {
    IfxGtm_Cmu_Eclk_0 = 0,
    IfxGtm_Cmu_Eclk_1,
    IfxGtm_Cmu_Eclk_2
} IfxGtm_Cmu_Eclk;

typedef enum {
    IfxGtm_Cmu_Fxclk_0 = 0,
    IfxGtm_Cmu_Fxclk_1,
    IfxGtm_Cmu_Fxclk_2,
    IfxGtm_Cmu_Fxclk_3,
    IfxGtm_Cmu_Fxclk_4
} IfxGtm_Cmu_Fxclk;

typedef enum {
    IfxGtm_Cmu_Tim_Filter_Clk_0 = 0,
    IfxGtm_Cmu_Tim_Filter_Clk_1 = 1,
    IfxGtm_Cmu_Tim_Filter_Clk_6 = 6,
    IfxGtm_Cmu_Tim_Filter_Clk_7 = 7
} IfxGtm_Cmu_Tim_Filter_Clk;

#ifndef IFXGTM_CMU_CLKEN_FXCLK
# define IFXGTM_CMU_CLKEN_FXCLK (0x1u)
#endif
#ifndef IFXGTM_CMU_CLKEN_CLK0
# define IFXGTM_CMU_CLKEN_CLK0  (0x2u)
#endif

/* Mandatory CMU functions */
void    IfxGtm_Cmu_enable(Ifx_GTM *module);
boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *module);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *module, float32 frequency);
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *module, IfxGtm_Cmu_Clk clk, float32 frequency);
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *module, uint32 mask);

#endif /* IFXGTM_CMU_H */
