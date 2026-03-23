#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for float32, uint32, Ifx_EGTM, Ifx_EGTM_CLS */

/* ============= Type Definitions ============= */
typedef struct { uint32 reserved; } IfxEgtm_Pwm_ChannelConfig;
typedef struct { uint32 reserved; } IfxEgtm_Pwm_Channel;
/* IfxEgtm_Pwm_Config - from illd_types/Ifx_Types.h */
/* IfxEgtm_Pwm - from illd_types/Ifx_Types.h */
typedef enum {
    IfxEgtm_Pwm_SubModule_atom = 0
} IfxEgtm_Pwm_SubModule;
typedef enum {
    IfxEgtm_Pwm_SubModule_Ch_0 = 0,
    IfxEgtm_Pwm_SubModule_Ch_1 = 1
} IfxEgtm_Pwm_SubModule_Ch;

/* Minimal PWM-related structs/enums to support mocks */
/* iLLD function declarations to mock */
/* Mock control: call counts */
/* Mock control: value captures */
/* Pattern D: capture key config fields from init(config) */
/* Pattern B: array capture for updateChannelsDuty */
/* Pattern C: enum/integer capture for setChannelPolarity */

/* ============= Function Declarations ============= */
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig);
void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm_Channel *channel, void *data);
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config);
void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm);
void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty);
void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void);
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void);
void IfxEgtm_Pwm_Mock_Reset(void);

#endif