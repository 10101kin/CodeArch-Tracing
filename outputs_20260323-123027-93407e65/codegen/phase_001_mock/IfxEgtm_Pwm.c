#include "IfxEgtm_Pwm.h"

#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u

static uint32 s_init_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_startSyncedGroups_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_initConfig_count = 0;

/* Pattern D captured fields for init() */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Pattern D captured fields for initConfig() */
static float32 s_initConfig_lastFrequency = 0.0f;
static uint32  s_initConfig_lastNumChannels = 0u;
static uint32  s_initConfig_lastAlignment = 0u;
static uint32  s_initConfig_lastSyncStart = 0u;

/* initChannelConfig capture */
static float32 s_channelConfig_lastDuty = 0.0f;
static float32 s_channelConfig_lastFrequency = 0.0f;
static uint32  s_channelConfig_lastChannel = 0u;

/* Array capture for updateChannelsDutyImmediate */
static float32 s_update_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0u;

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    s_initChannelConfig_count++;
    if (channelConfig != NULL_PTR) {
        s_channelConfig_lastDuty      = channelConfig->duty;
        s_channelConfig_lastFrequency = channelConfig->frequency;
        s_channelConfig_lastChannel   = channelConfig->channel;
    }
}

void IfxEgtm_Pwm_startSyncedGroups(IfxEgtm_Pwm *pwm1, IfxEgtm_Pwm *pwm2)
{
    (void)pwm1;
    (void)pwm2;
    s_startSyncedGroups_count++;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY) {
            for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_update_history[s_update_historyCount][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    s_initConfig_count++;
    if (config != NULL_PTR) {
        s_initConfig_lastFrequency   = config->frequency;
        s_initConfig_lastNumChannels = config->numChannels;
        s_initConfig_lastAlignment   = config->alignment;
        s_initConfig_lastSyncStart   = config->syncStart;
    }
}

uint32  IfxEgtm_Pwm_Mock_GetCallCount_init(void)                       { return s_init_count; }
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void)          { return s_initChannelConfig_count; }
uint32  IfxEgtm_Pwm_Mock_GetCallCount_startSyncedGroups(void)          { return s_startSyncedGroups_count; }
uint32  IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void){ return s_updateChannelsDutyImmediate_count; }
uint32  IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void)                 { return s_initConfig_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void)    { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void)  { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void)    { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void)    { return s_init_lastSyncStart; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_initConfig_frequency(void)   { return s_initConfig_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void) { return s_initConfig_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_alignment(void)   { return s_initConfig_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void)   { return s_initConfig_lastSyncStart; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_duty(void)      { return s_channelConfig_lastDuty; }
float32 IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_frequency(void) { return s_channelConfig_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_channel(void)   { return s_channelConfig_lastChannel; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_update_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS)) {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_update_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_init_count = 0u;
    s_initChannelConfig_count = 0u;
    s_startSyncedGroups_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_initConfig_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    s_initConfig_lastFrequency = 0.0f;
    s_initConfig_lastNumChannels = 0u;
    s_initConfig_lastAlignment = 0u;
    s_initConfig_lastSyncStart = 0u;

    s_channelConfig_lastDuty = 0.0f;
    s_channelConfig_lastFrequency = 0.0f;
    s_channelConfig_lastChannel = 0u;

    for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
        s_update_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_HISTORY; i++) {
        for (j = 0u; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; j++) {
            s_update_history[i][j] = 0.0f;
        }
    }
    s_update_historyCount = 0u;
}
