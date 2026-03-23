#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for base types */

/* Mock controls */
/* Pattern D capture for initModule/initModuleConfig */

/* ============= Function Declarations ============= */
void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config);
void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc);
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc);
uint32  IfxAdc_Tmadc_Mock_GetCallCount_initModule(void);
uint32  IfxAdc_Tmadc_Mock_GetCallCount_runModule(void);
uint32  IfxAdc_Tmadc_Mock_GetCallCount_initModuleConfig(void);
uint32  IfxAdc_Tmadc_Mock_GetLastArg_initModule_sampleRate(void);
uint32  IfxAdc_Tmadc_Mock_GetLastArg_initModule_numChannels(void);
uint32  IfxAdc_Tmadc_Mock_GetLastArg_initModuleConfig_sampleRate(void);
uint32  IfxAdc_Tmadc_Mock_GetLastArg_initModuleConfig_numChannels(void);
void    IfxAdc_Tmadc_Mock_Reset(void);

#endif /* IFXADC_TMADC_H */