#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"

/* ============= Type Definitions ============= */
struct IfxGtm_Pwm { uint32 __mock; };
/* IfxGtm_Pwm - defined in Ifx_Types.h */
struct IfxGtm_Pwm_Channel { uint32 __mock; };
/* IfxGtm_Pwm_Channel - defined in Ifx_Types.h */
struct IfxGtm_Pwm_ChannelConfig { uint32 __mock; };
/* IfxGtm_Pwm_ChannelConfig - defined in Ifx_Types.h */
struct IfxGtm_Pwm_ClusterSFR { uint32 __mock; };
/* IfxGtm_Pwm_ClusterSFR - defined in Ifx_Types.h */
/* IfxGtm_Pwm_Config - defined in Ifx_Types.h */

/* Minimal mock types to allow API usage and config capture */
/* Config struct with key fields for Pattern D capture */
/* API declarations */
/* Mock control */
/* Pattern D: capture config fields passed to init() */
/* Value-carrying argument capture */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR);
void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, IfxGtm_Pwm_DeadTime requestDeadTime);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, float32 requestDuty);
void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm_ClusterSFR *clusterSFR, IfxGtm_Pwm_SubModule subModule, IfxGtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity);
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_index(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_index(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_rising(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_falling(void);
uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void);
uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void);
uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif