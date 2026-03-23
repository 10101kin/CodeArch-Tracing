#include "IfxEgtm_Pwm.h"

#define PWM_MOCK_MAX_ELEMENTS 8u
#define PWM_MOCK_MAX_HISTORY  32u

static uint32 s_initChannelConfig_count = 0;
static uint32 s_interruptHandler_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateChannelsDuty_count = 0;
static uint32 s_setChannelPolarity_count = 0;

/* Pattern D capture for init(config) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Pattern B capture for updateChannelsDuty */
static float32 s_update_lastArray[PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[PWM_MOCK_MAX_HISTORY][PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0u;

/* Pattern C capture for setChannelPolarity */
static uint32 s_last_setChannelPolarity_subModule = 0u;
static uint32 s_last_setChannelPolarity_channel = 0u;
static uint32 s_last_setChannelPolarity_polarity = 0u;

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm_Channel *channel, void *data) {
    (void)channel;
    (void)data;
    s_interruptHandler_count++;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR) {
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    s_init_count++;
    /* Pattern D: capture known key fields if provided */
    if (config != NULL_PTR) {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < PWM_MOCK_MAX_HISTORY) {
            for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) {
                s_update_history[s_update_historyCount][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity) {
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_last_setChannelPolarity_subModule = (uint32)subModule;
    s_last_setChannelPolarity_channel = (uint32)channel;
    s_last_setChannelPolarity_polarity = (uint32)polarity;
}

uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index) {
    return (index < PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_update_historyCount && elemIdx < PWM_MOCK_MAX_ELEMENTS) {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void) { return s_update_historyCount; }

uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_last_setChannelPolarity_subModule; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_last_setChannelPolarity_channel; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_last_setChannelPolarity_polarity; }

void IfxEgtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_initChannelConfig_count = 0u;
    s_interruptHandler_count = 0u;
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateChannelsDuty_count = 0u;
    s_setChannelPolarity_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) {
        s_update_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < PWM_MOCK_MAX_HISTORY; i++) {
        for (j = 0u; j < PWM_MOCK_MAX_ELEMENTS; j++) {
            s_update_history[i][j] = 0.0f;
        }
    }
    s_update_historyCount = 0u;

    s_last_setChannelPolarity_subModule = 0u;
    s_last_setChannelPolarity_channel = 0u;
    s_last_setChannelPolarity_polarity = 0u;
}
