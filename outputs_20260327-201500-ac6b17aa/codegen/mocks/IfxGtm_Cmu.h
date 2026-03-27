/* Mock IfxGtm_Cmu.h - CMU enums and functions */
#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* CLKEN macros */
#ifndef IFXGTM_CMU_CLKEN_FXCLK
#define IFXGTM_CMU_CLKEN_FXCLK (0x1u)
#endif
#ifndef IFXGTM_CMU_CLKEN_CLK0
#define IFXGTM_CMU_CLKEN_CLK0  (0x2u)
#endif

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
    IfxGtm_Cmu_Tim_Filter_Clk_0 = 0,
    IfxGtm_Cmu_Tim_Filter_Clk_1 = 1,
    IfxGtm_Cmu_Tim_Filter_Clk_6 = 6,
    IfxGtm_Cmu_Tim_Filter_Clk_7 = 7
} IfxGtm_Cmu_Tim_Filter_Clk;

/* DTM Clock Source (include cmuClock0 to fix prior build error) */
typedef enum
{
    IfxGtm_Dtm_ClockSource_cmuClock0 = 0,
    IfxGtm_Dtm_ClockSource_cmuClock1 = 1,
    IfxGtm_Dtm_ClockSource_cmuClock2 = 2,
    IfxGtm_Dtm_ClockSource_cmuClock3 = 3,
    IfxGtm_Dtm_ClockSource_cmuClock4 = 4,
    IfxGtm_Dtm_ClockSource_cmuClock5 = 5,
    IfxGtm_Dtm_ClockSource_cmuClock6 = 6,
    IfxGtm_Dtm_ClockSource_cmuClock7 = 7,
    IfxGtm_Dtm_ClockSource_fxClock0  = 16,
    IfxGtm_Dtm_ClockSource_fxClock1  = 17,
    IfxGtm_Dtm_ClockSource_fxClock2  = 18,
    IfxGtm_Dtm_ClockSource_fxClock3  = 19,
    IfxGtm_Dtm_ClockSource_fxClock4  = 20
} IfxGtm_Dtm_ClockSource;

/* Functions used by enable-guard pattern */
void    IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, boolean assumeEnabled);
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clkIndex, float32 frequency);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 frequency);

#endif /* IFXGTM_CMU_H */
