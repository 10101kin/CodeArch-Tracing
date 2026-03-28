#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* CMU clock enums */
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
    IfxGtm_Cmu_Tim_Filter_Clk_0,  /* FLT_CNT counts with CMU_CLK0 */
    IfxGtm_Cmu_Tim_Filter_Clk_1,  /* FLT_CNT counts with CMU_CLK1 */
    IfxGtm_Cmu_Tim_Filter_Clk_6,  /* FLT_CNT counts with CMU_CLK6 */
    IfxGtm_Cmu_Tim_Filter_Clk_7   /* Some devices allow CMU_CLK7 for filter */
} IfxGtm_Cmu_Tim_Filter_Clk;

/* DTM Clock Source enum (used by PWM config) */
typedef enum
{
    IfxGtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxGtm_Dtm_ClockSource_cmuClk1 = 1,
    IfxGtm_Dtm_ClockSource_fxclk   = 2
} IfxGtm_Dtm_ClockSource;

/* CMU enable macros */
#ifndef IFXGTM_CMU_CLKEN_FXCLK
#define IFXGTM_CMU_CLKEN_FXCLK (0x1u << 0)
#endif
#ifndef IFXGTM_CMU_CLKEN_CLK0
#define IFXGTM_CMU_CLKEN_CLK0  (0x1u << 8)
#endif

/* Function declarations (subset needed by production) */
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency);

#endif /* IFXGTM_CMU_H */
