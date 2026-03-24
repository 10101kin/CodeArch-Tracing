#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_updateChannelsDutyImmediate_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_init_count = 0u;

/* Array capture with history (Pattern B) */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_update_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0u;

/* Config captures (Pattern D) */
static float32 s_init_lastFrequency   = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment   = 0u;
static uint32  s_init_lastSyncStart   = 0u;

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY) {
            uint32 idx = s_update_historyCount;
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_update_history[idx][i] = requestDuty[i];
            }
            s_update_historyCount++;
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
        /* Pattern D: capture selected config scalars for verification */
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

/* Mock controls */
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if ((callIdx < s_update_historyCount) && (elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS)) {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) { return s_update_historyCount; }

uint32  IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }

uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void)   { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void)   { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void)   { return s_init_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void) {
    s_updateChannelsDutyImmediate_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_init_count = 0u;

    {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = 0.0f;
        }
    }
    {
        uint32 r, c;
        for (r = 0u; r < IFXGTM_PWM_MOCK_MAX_HISTORY; r++) {
            for (c = 0u; c < IFXGTM_PWM_MOCK_MAX_ELEMENTS; c++) {
                s_update_history[r][c] = 0.0f;
            }
        }
    }
    s_update_historyCount = 0u;

    s_init_lastFrequency   = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment   = 0u;
    s_init_lastSyncStart   = 0u;
}
