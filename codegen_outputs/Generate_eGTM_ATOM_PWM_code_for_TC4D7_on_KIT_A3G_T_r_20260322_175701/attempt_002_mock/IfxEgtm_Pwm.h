#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declarations */
/* Mock controls: call counts */
/* Mock controls: Pattern D config field capture (if available) */
/* Mock controls: reset */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif