#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_updateChannelsDuty_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateChannelsDeadTime_count = 0;

/* Argument capture storage */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_updateDuty_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDuty_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDuty_historyCount = 0u;
static IfxEgtm_Pwm_DeadTime *s_updateDeadTime_lastPtr = NULL_PTR;

/* iLLD stubs */
void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR) {
    (void)config; (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config) {
    (void)pwm; (void)channels; (void)config;
    s_init_count++;
    /* Pattern D: Opaque config; no field capture performed */
}

void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY) {
            for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateDuty_history[s_updateDuty_historyCount][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_DeadTime *requestDeadTime) {
    (void)pwm; (void)requestDeadTime;
    s_updateChannelsDeadTime_count++;
    s_updateDeadTime_lastPtr = requestDeadTime;
}

/* Mock controls */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void) { return s_updateChannelsDeadTime_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index) {
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < IFXEGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void) {
    return s_updateDuty_historyCount;
}

IfxEgtm_Pwm_DeadTime* IfxEgtm_Pwm_Mock_GetLastArg_updateChannelsDeadTime_ptr(void) {
    return s_updateDeadTime_lastPtr;
}

void IfxEgtm_Pwm_Mock_Reset(void) {
    s_initChannelConfig_count = 0u;
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_updateChannelsDuty_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateChannelsDeadTime_count = 0u;
    for (uint32 i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
        s_updateDuty_lastArray[i] = 0.0f;
    }
    for (uint32 c = 0u; c < IFXEGTM_PWM_MOCK_MAX_HISTORY; c++) {
        for (uint32 i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateDuty_history[c][i] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0u;
    s_updateDeadTime_lastPtr = NULL_PTR;
}
