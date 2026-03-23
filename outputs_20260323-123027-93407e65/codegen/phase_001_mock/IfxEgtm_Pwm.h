#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for base types */

/* API declarations (exact signatures) */
/* Mock controls */
/* Pattern D capture getters for init()/initConfig() */
/* ChannelConfig capture */
/* Array capture for updateChannelsDutyImmediate */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_startSyncedGroups(IfxEgtm_Pwm *pwm1, IfxEgtm_Pwm *pwm2);
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_startSyncedGroups(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_initConfig_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_duty(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_channel(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
void    IfxEgtm_Pwm_Mock_Reset(void);

#endif /* IFXEGTM_PWM_H */