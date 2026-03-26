#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums as per iLLD */
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
    IfxGtm_Cmu_Tim_Filter_Clk_0 = 0, /* CMU_CLK0 */
    IfxGtm_Cmu_Tim_Filter_Clk_1 = 1, /* CMU_CLK1 */
    IfxGtm_Cmu_Tim_Filter_Clk_6 = 2, /* CMU_CLK6 */
    IfxGtm_Cmu_Tim_Filter_Clk_7 = 3  /* CMU_CLK7 */
} IfxGtm_Cmu_Tim_Filter_Clk;

/* Additional GTM enums/macros required by PWM config */
typedef enum {
    IfxGtm_Dtm_ClockSource_cmuClock0 = 0,
    IfxGtm_Dtm_ClockSource_cmuClock1 = 1,
    IfxGtm_Dtm_ClockSource_cmuClock2 = 2
} IfxGtm_Dtm_ClockSource;

#ifndef IFXGTM_CMU_CLKEN_FXCLK
#define IFXGTM_CMU_CLKEN_FXCLK  (1u << 22)
#endif
#ifndef IFXGTM_CMU_CLKEN_CLK0
#define IFXGTM_CMU_CLKEN_CLK0   (2u << 0)
#endif

/* Functions (exact signatures provided) */
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);
/* Also declare setClkFrequency as required by template */
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq);

#endif /* IFXGTM_CMU_H */
