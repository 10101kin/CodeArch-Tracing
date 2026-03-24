#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declarations */
/* Mock controls: call counts */
/* Mock controls: return value (non-void) */
/* Mock controls: last-argument capture getters */
/* Array capture for duty update (immediate) */
/* Array capture for duty update (shadow) */
/* Scalar capture for setChannelPolarity */
/* Optional Pattern D placeholders for init() config fields (remain 0 if unknown) */
/* Mock controls: reset */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch channel);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_DeadTime *requestDeadTime);
void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity);
void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_getChannelState(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
void IfxEgtm_Pwm_Mock_SetReturn_getChannelState(IfxEgtm_Pwm_ChannelState value);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_getChannelState_channel(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif /* IFXEGTM_PWM_H */