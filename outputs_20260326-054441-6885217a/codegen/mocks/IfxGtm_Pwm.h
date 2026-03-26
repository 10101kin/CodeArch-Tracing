#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
typedef struct IfxGtm_Pwm              IfxGtm_Pwm;
typedef struct IfxGtm_Pwm_Channel      IfxGtm_Pwm_Channel;
typedef struct IfxGtm_Pwm_Config       IfxGtm_Pwm_Config;
typedef struct IfxGtm_Pwm_ChannelConfig IfxGtm_Pwm_ChannelConfig;
typedef struct IfxGtm_Pwm_DeadTime     IfxGtm_Pwm_DeadTime;
/* IfxGtm_Pwm_Alignment - from illd_types/Ifx_Types.h */
struct IfxGtm_Pwm_Config {
    float32              frequency;    /* desired PWM frequency */
    uint32               numChannels;  /* number of channels configured */
    IfxGtm_Pwm_Alignment alignment;    /* alignment mode */
    uint32               syncStart;    /* boolean-like: 0/1 */
};

/* Forward declarations for PWM types used by the API */
/* Minimal config/enums to enable Pattern D captures */
/* API declarations (exact signatures) */
/* Mock controls: call counts */
/* Mock controls: value captures */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
/* Pattern D captures for init() and initConfig() */
/* Mock reset */

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
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_initConfig_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif