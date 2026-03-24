#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_updateDutyImmediate_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_init_count = 0u;

/* Array capture with history for updateChannelsDutyImmediate */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_updateDutyImmediate_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDutyImmediate_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDutyImmediate_historyCount = 0u;

/* Init config capture (Pattern D) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_lastArray[i] = requestDuty[i];
        }
        if (s_updateDutyImmediate_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY) {
            uint32 idx = s_updateDutyImmediate_historyCount;
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateDutyImmediate_history[idx][i] = requestDuty[i];
            }
            s_updateDutyImmediate_historyCount++;
        }
    }
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
        /* Pattern D: capture key scalar fields for verification */
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

/* Mock controls */
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) {
    return s_updateDutyImmediate_count;
}

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDutyImmediate_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_updateDutyImmediate_historyCount && elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateDutyImmediate_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) {
    return s_updateDutyImmediate_historyCount;
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) {
    return s_startSyncedChannels_count;
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) {
    return s_init_count;
}

float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_updateDutyImmediate_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_init_count = 0u;

    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
        s_updateDutyImmediate_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_HISTORY; i++) {
        for (j = 0u; j < IFXGTM_PWM_MOCK_MAX_ELEMENTS; j++) {
            s_updateDutyImmediate_history[i][j] = 0.0f;
        }
    }
    s_updateDutyImmediate_historyCount = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
