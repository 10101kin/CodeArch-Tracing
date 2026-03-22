#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */
#include "IfxEgtm.h"    /* Cross-dependency for EGTM handle type */

/* Mock controls */
/* Value capture getters */
/* Array capture and history for updateChannelsDutyImmediate */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity);
void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm_Channel *channel, void *data);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate_requestDuty(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate_requestDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate_requestDuty(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif /* IFXEGTM_PWM_H */