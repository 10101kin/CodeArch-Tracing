#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations */
/* Mock control: call counts */
/* Mock control: array value capture for updateChannelsDutyImmediate */
/* Mock control: reset all state */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif