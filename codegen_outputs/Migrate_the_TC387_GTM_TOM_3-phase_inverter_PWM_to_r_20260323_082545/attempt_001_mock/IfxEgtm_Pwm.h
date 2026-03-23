#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations */
/* Mock controls: call counts */
/* Mock controls: argument capture */
/* Pattern B: array capture for requestDuty */
/* Dead-time pointer capture (no dereference) */
/* Optional init() config field capture not available (opaque config) */
/* Mock controls: reset */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_DeadTime *requestDeadTime);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void);
IfxEgtm_Pwm_DeadTime* IfxEgtm_Pwm_Mock_GetLastArg_updateChannelsDeadTime_ptr(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif