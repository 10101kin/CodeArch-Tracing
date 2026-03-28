#ifndef IFXGTM_CMU_H
#define IFXGTM_CMU_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* CMU clock enums */
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

/* DTM Clock Source (used by Timer/Pwm config) */
typedef enum {
    IfxGtm_Dtm_ClockSource_cmuClk0 = 0,
    IfxGtm_Dtm_ClockSource_cmuClk1,
    IfxGtm_Dtm_ClockSource_cmuClk2,
    IfxGtm_Dtm_ClockSource_cmuClk3,
    IfxGtm_Dtm_ClockSource_cmuClk4,
    IfxGtm_Dtm_ClockSource_cmuClk5,
    IfxGtm_Dtm_ClockSource_cmuClk6,
    IfxGtm_Dtm_ClockSource_cmuClk7,
    IfxGtm_Dtm_ClockSource_cmuFxclk0,
    IfxGtm_Dtm_ClockSource_cmuFxclk1,
    IfxGtm_Dtm_ClockSource_cmuFxclk2,
    IfxGtm_Dtm_ClockSource_cmuFxclk3,
    IfxGtm_Dtm_ClockSource_cmuFxclk4
} IfxGtm_Dtm_ClockSource;

#ifndef IFXGTM_TOM_CMU_CLKEN_FXCLK
#define IFXGTM_TOM_CMU_CLKEN_FXCLK (0x1U)
#endif
#ifndef IFXGTM_TOM_CMU_CLKEN_CLK0
#define IFXGTM_TOM_CMU_CLKEN_CLK0  (0x2U)
#endif

/* Functions to mock */
void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask);

#endif /* IFXGTM_CMU_H */
