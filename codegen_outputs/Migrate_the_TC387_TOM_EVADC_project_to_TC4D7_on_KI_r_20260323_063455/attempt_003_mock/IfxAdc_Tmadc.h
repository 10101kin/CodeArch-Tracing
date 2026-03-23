#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Opaque forward decls keep types out of mock to avoid conflicts */
/* See Ifx_Types.h for typedefs of IfxAdc_Tmadc*, IfxAdc_Tmadc_Config*, etc. */
/* API declarations */
/* Mock control - call counts */
/* Reset */

/* ============= Function Declarations ============= */
void IfxAdc_Tmadc_initChannel(IfxAdc_Tmadc_Ch *channel, IfxAdc_Tmadc_ChConfig *config);
void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc);
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc);
void IfxAdc_Tmadc_initChannelConfig(IfxAdc_Tmadc_ChConfig *config, Ifx_ADC *adc);
void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config);
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initChannel(void);
uint32 IfxAdc_Tmadc_Mock_GetCallCount_runModule(void);
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig(void);
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxAdc_Tmadc_Mock_GetCallCount_initModule(void);
void IfxAdc_Tmadc_Mock_Reset(void);

#endif /* IFXADC_TMADC_H */