#ifndef IFXADC_H
#define IFXADC_H

#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxAdc_Tmadc.h" /* brings Ifx_ADC_TMADC and related types */

/* Enums subset required for prototypes and common usage */
typedef enum { IfxAdc_Apu_0 = 0, IfxAdc_Apu_1, IfxAdc_Apu_2, IfxAdc_Apu_3, IfxAdc_Apu_4, IfxAdc_Apu_5, IfxAdc_Apu_6, IfxAdc_Apu_7, IfxAdc_Apu_8, IfxAdc_Apu_9, IfxAdc_Apu_10, IfxAdc_Apu_11, IfxAdc_Apu_12, IfxAdc_Apu_13, IfxAdc_Apu_14 } IfxAdc_Apu;

typedef enum { IfxAdc_GlobalResource_adc = 0, IfxAdc_GlobalResource_dsadc = 1 } IfxAdc_GlobalResource;

typedef enum { IfxAdc_ModuleState_enable = 0, IfxAdc_ModuleState_disable = 1 } IfxAdc_ModuleState;

typedef enum { IfxAdc_Status_success = 0, IfxAdc_Status_failure = 1 } IfxAdc_Status;

/* TMADC-specific enums already in IfxAdc_Tmadc.h; re-use those */

/* API subset used by production */
void    IfxAdc_clearTmadcResultFlag(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);
void    IfxAdc_enableModule(Ifx_ADC *modSFR);
boolean IfxAdc_isTmadcResultAvailable(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum);

#endif /* IFXADC_H */
