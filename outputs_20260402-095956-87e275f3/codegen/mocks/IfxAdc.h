#ifndef IFXADC_H
#define IFXADC_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Minimal types required by used APIs */

typedef enum {
    IfxAdc_TmadcResultReg_0 = 0,
    IfxAdc_TmadcResultReg_1,
    IfxAdc_TmadcResultReg_2,
    IfxAdc_TmadcResultReg_3,
    IfxAdc_TmadcResultReg_4,
    IfxAdc_TmadcResultReg_5,
    IfxAdc_TmadcResultReg_6,
    IfxAdc_TmadcResultReg_7,
    IfxAdc_TmadcResultReg_8,
    IfxAdc_TmadcResultReg_9,
    IfxAdc_TmadcResultReg_10,
    IfxAdc_TmadcResultReg_11,
    IfxAdc_TmadcResultReg_12,
    IfxAdc_TmadcResultReg_13,
    IfxAdc_TmadcResultReg_14,
    IfxAdc_TmadcResultReg_15
} IfxAdc_TmadcResultReg;

/* Function declarations used by production */
boolean IfxAdc_isTmadcResultAvailable(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);
void    IfxAdc_clearTmadcResultFlag(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);

#endif /* IFXADC_H */
