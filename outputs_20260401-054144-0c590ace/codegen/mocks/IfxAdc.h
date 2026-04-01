/* IfxAdc mock (minimal for enableModule) */
#ifndef IFXADC_H
#define IFXADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"

typedef enum { IfxAdc_Apu_0, IfxAdc_Apu_1, IfxAdc_Apu_2, IfxAdc_Apu_3, IfxAdc_Apu_4, IfxAdc_Apu_5, IfxAdc_Apu_6, IfxAdc_Apu_7, IfxAdc_Apu_8, IfxAdc_Apu_9, IfxAdc_Apu_10, IfxAdc_Apu_11, IfxAdc_Apu_12, IfxAdc_Apu_13, IfxAdc_Apu_14, IfxAdc_Apu_15 } IfxAdc_Apu;

typedef enum { IfxAdc_GlobalResource_adc = 0, IfxAdc_GlobalResource_dsadc = 1, IfxAdc_GlobalResource_cdsp = 2 } IfxAdc_GlobalResource;

typedef enum { IfxAdc_ModuleState_enable = 0, IfxAdc_ModuleState_disable = 1 } IfxAdc_ModuleState;

typedef enum { IfxAdc_Status_success = 0, IfxAdc_Status_failure = 1 } IfxAdc_Status;

void IfxAdc_enableModule(Ifx_ADC *modSFR);

#endif /* IFXADC_H */
