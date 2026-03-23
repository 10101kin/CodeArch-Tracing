#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Function declarations (types provided via Ifx_Types.h opaque typedefs) */
/* Mock controls */
/* Array value capture for update duty (Pattern B) */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_stopSyncedChannels(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_stopSyncedChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_init(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
void    IfxEgtm_Pwm_Mock_Reset(void);

#endif