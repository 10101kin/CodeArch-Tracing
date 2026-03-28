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

/* DTM Clock Source (mock) */
typedef enum {
    IfxGtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxGtm_Dtm_ClockSource_cmuClk1,
    IfxGtm_Dtm_ClockSource_cmuClk2,
    IfxGtm_Dtm_ClockSource_cmuClk3,
    IfxGtm_Dtm_ClockSource_cmuClk4,
    IfxGtm_Dtm_ClockSource_cmuClk5,
    IfxGtm_Dtm_ClockSource_cmuClk6,
    IfxGtm_Dtm_ClockSource_cmuClk7,
    IfxGtm_Dtm_ClockSource_fxclk
} IfxGtm_Dtm_ClockSource;

/* CMU enable macros */
#ifndef IFXGTM_TOM_CMU_CLKEN_FXCLK
#define IFXGTM_TOM_CMU_CLKEN_FXCLK (0x1u << 0)
#endif
#ifndef IFXGTM_TOM_CMU_CLKEN_CLK0
#define IFXGTM_TOM_CMU_CLKEN_CLK0  (0x1u << 1)
#endif

/* API declarations (we only stub enableClocks with counters; others provided minimal if needed by build) */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);

/* Optional helpers (no counters; provided to satisfy potential references) */
float32 IfxGtm_Cmu_getClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk);
float32 IfxGtm_Cmu_getEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk);
float32 IfxGtm_Cmu_getFxClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk);
float32 IfxGtm_Cmu_getGclkFrequency(Ifx_GTM *gtm);
float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *gtm);
boolean IfxGtm_Cmu_isClkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk);
boolean IfxGtm_Cmu_isEclkClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk);
boolean IfxGtm_Cmu_isFxClockEnabled(Ifx_GTM *gtm, IfxGtm_Cmu_Fxclk fxclk);
void    IfxGtm_Cmu_selectClkInput(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk);
void    IfxGtm_Cmu_setClkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Clk clk, float32 freq);
void    IfxGtm_Cmu_setEclkFrequency(Ifx_GTM *gtm, IfxGtm_Cmu_Eclk eclk, float32 freq);
void    IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *gtm, float32 freq);

#endif /* IFXGTM_CMU_H */
