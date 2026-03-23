#ifndef IFXADC_TMADCH

#include "illd_types/Ifx_Types.h"  /* REQUIRED for basic types */

#define IFXADC_TMADCH
/* API declarations */
/* Mock control: call counts */
/* Pattern D capture for initModule(config) */
/* Mock control */

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
float32 IfxAdc_Tmadc_Mock_GetLastArg_initModule_frequency(void);
uint32  IfxAdc_Tmadc_Mock_GetLastArg_initModule_channelCount(void);
void IfxAdc_Tmadc_Mock_Reset(void);

#endif /* IFXADC_TMADCH */