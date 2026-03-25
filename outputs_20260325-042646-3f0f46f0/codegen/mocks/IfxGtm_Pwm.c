#include "IfxGtm_Pwm.h"

static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_updateDutyImmediate_count = 0;

/* Array capture buffers for updateChannelsDutyImmediate */
#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

static float32 s_updateDutyImmediate_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDutyImmediate_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDutyImmediate_historyCount = 0;

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)config;
    (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    (void)config;
    s_init_count++;
    /* Pattern D note: Config field capture intentionally omitted (opaque in mock) */
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0; i < MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_lastArray[i] = requestDuty[i];
        }
        if (s_updateDutyImmediate_historyCount < MOCK_MAX_HISTORY) {
            uint32 j;
            for (j = 0; j < MOCK_MAX_ELEMENTS; j++) {
                s_updateDutyImmediate_history[s_updateDutyImmediate_historyCount][j] = requestDuty[j];
            }
            s_updateDutyImmediate_historyCount++;
        }
    }
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateDutyImmediate_count; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDutyImmediate_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_updateDutyImmediate_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_updateDutyImmediate_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) {
    return s_updateDutyImmediate_historyCount;
}

void IfxGtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_initConfig_count = 0;
    s_init_count = 0;
    s_startSyncedChannels_count = 0;
    s_initChannelConfig_count = 0;
    s_updateDutyImmediate_count = 0;

    for (i = 0; i < MOCK_MAX_ELEMENTS; i++) {
        s_updateDutyImmediate_lastArray[i] = 0.0f;
    }
    for (i = 0; i < MOCK_MAX_HISTORY; i++) {
        for (j = 0; j < MOCK_MAX_ELEMENTS; j++) {
            s_updateDutyImmediate_history[i][j] = 0.0f;
        }
    }
    s_updateDutyImmediate_historyCount = 0;
}
