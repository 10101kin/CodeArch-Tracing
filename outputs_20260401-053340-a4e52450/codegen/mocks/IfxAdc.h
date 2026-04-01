#ifndef IFXADC_H
#define IFXADC_H
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"

/* Only the enums/types needed by functions used here */
typedef enum {
    IfxAdc_TmadcResultReg_0 = 0, IfxAdc_TmadcResultReg_1, IfxAdc_TmadcResultReg_2, IfxAdc_TmadcResultReg_3,
    IfxAdc_TmadcResultReg_4, IfxAdc_TmadcResultReg_5, IfxAdc_TmadcResultReg_6, IfxAdc_TmadcResultReg_7,
    IfxAdc_TmadcResultReg_8, IfxAdc_TmadcResultReg_9, IfxAdc_TmadcResultReg_10, IfxAdc_TmadcResultReg_11,
    IfxAdc_TmadcResultReg_12, IfxAdc_TmadcResultReg_13, IfxAdc_TmadcResultReg_14, IfxAdc_TmadcResultReg_15
} IfxAdc_TmadcResultReg;

/* Forward SFR subset typedefs for TMADC (defined fully in IfxAdc_Tmadc.h) */
typedef struct Ifx_ADC_TMADC Ifx_ADC_TMADC;

/* Functions from drivers-to-mock */
void    IfxAdc_clearTmadcResultFlag(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);
void    IfxAdc_enableModule(Ifx_ADC *modSFR);
boolean IfxAdc_isTmadcResultAvailable(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);

#endif /* IFXADC_H */
