#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"
#include "IfxPort.h"  /* for IfxPort_* types if needed by production code */

/* Extern variable required by tests */
extern IfxGtm_Pwm g_gtmTom3PhaseInverterPwm;
/* Mock control */
/* Array capture helpers */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
uint32  IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
void    IfxGtm_Pwm_Mock_Reset(void);

#endif