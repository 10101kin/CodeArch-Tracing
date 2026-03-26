#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Function declarations */
/* Mock control and capture functions */
/* Duty array capture */
/* Frequency capture */
/* Dead-time capture (rising/falling) */
/* init() config capture */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_DeadTime *requestDeadTime);
void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 requestFrequency);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
uint32 IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDeadTimeImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateFrequencyImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_updateFrequencyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDeadTimeImmediate_rising(uint32 index);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDeadTimeImmediate_falling(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDeadTimeImmediate_rising(uint32 callIdx, uint32 elemIdx);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDeadTimeImmediate_falling(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDeadTimeImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif