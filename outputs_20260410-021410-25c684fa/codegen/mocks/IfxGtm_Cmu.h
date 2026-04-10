/* IfxGtm_Cmu.h - GTM CMU types + functions */
#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums */
typedef enum
{
    IfxGtm_Cmu_Clk_0 = 0,
    IfxGtm_Cmu_Clk_1,
    IfxGtm_Cmu_Clk_2,
    IfxGtm_Cmu_Clk_3,
    IfxGtm_Cmu_Clk_4,
    IfxGtm_Cmu_Clk_5,
    IfxGtm_Cmu_Clk_6,
    IfxGtm_Cmu_Clk_7
} IfxGtm_Cmu_Clk;

typedef enum
{
    IfxGtm_Cmu_Eclk_0 = 0,
    IfxGtm_Cmu_Eclk_1,
    IfxGtm_Cmu_Eclk_2
} IfxGtm_Cmu_Eclk;

typedef enum
{
    IfxGtm_Cmu_Fxclk_0 = 0,
    IfxGtm_Cmu_Fxclk_1,
    IfxGtm_Cmu_Fxclk_2,
    IfxGtm_Cmu_Fxclk_3,
    IfxGtm_Cmu_Fxclk_4
} IfxGtm_Cmu_Fxclk;

typedef enum
{
    IfxGtm_Cmu_Tim_Filter_Clk_0,  /* CMU_CLK0 */
    IfxGtm_Cmu_Tim_Filter_Clk_1,  /* CMU_CLK1 */
    IfxGtm_Cmu_Tim_Filter_Clk_6,  /* CMU_CLK6 */
    IfxGtm_Cmu_Tim_Filter_Clk_7   /* CMU_CLK7 */
} IfxGtm_Cmu_Tim_Filter_Clk;

/* Enable masks */
#ifndef IFXGTM_CMU_CLKEN_FXCLK
# define IFXGTM_CMU_CLKEN_FXCLK (0x00000001u)
#endif
#ifndef IFXGTM_CMU_CLKEN_CLK0
# define IFXGTM_CMU_CLKEN_CLK0  (0x00000002u)
#endif
#ifndef IFXGTM_CMU_CLKEN_ALL
# define IFXGTM_CMU_CLKEN_ALL   (0xFFFFFFFFu)
#endif

/* Functions required by template */
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency);
/* Mandatory CMU mock functions */
void    IfxGtm_Cmu_enable(Ifx_GTM *module);
boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module);

#endif /* IFXGTM_CMU_H */
