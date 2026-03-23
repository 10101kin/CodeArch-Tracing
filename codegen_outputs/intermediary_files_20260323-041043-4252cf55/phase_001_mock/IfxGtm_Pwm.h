#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* Required APIs */
/* Mock controls */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
void    IfxGtm_Pwm_Mock_Reset(void);

#endif /* IFXGTM_PWM_H */