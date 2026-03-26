#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint32, boolean, float32 types */

/* Function declarations */
/* Mock control functions */
/* Array argument capture accessors */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *requestPhase, float32 *requestDuty);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_requestDuty(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty_requestDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_requestDuty(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestPhase(uint32 index);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestDuty(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_requestPhase(uint32 callIdx, uint32 elemIdx);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_requestDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestPhase(void);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestDuty(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif /* IFXGTM_PWM_H */