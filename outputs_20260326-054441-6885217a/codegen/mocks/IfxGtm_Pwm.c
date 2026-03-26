#include "IfxGtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_startChannelOutputs_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_init_count = 0;
static uint32 s_updateChannelsDeadTimeImmediate_count = 0;
static uint32 s_updateFrequencyImmediate_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count = 0;

/* Duty array capture */
static float32 s_duty_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_duty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_duty_historyCount = 0;

/* Dead-time capture */
static float32 s_dead_rising_last[MOCK_MAX_ELEMENTS] = {0};
static float32 s_dead_falling_last[MOCK_MAX_ELEMENTS] = {0};
static float32 s_dead_rising_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static float32 s_dead_falling_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_dead_historyCount = 0;

/* Frequency capture */
static float32 s_updateFrequency_last = 0.0f;

/* init() capture */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startChannelOutputs_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_duty_lastArray[i] = requestDuty[i];
        }
        if (s_duty_historyCount < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_duty_history[s_duty_historyCount][i] = requestDuty[i];
            }
            s_duty_historyCount++;
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
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
}

void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_DeadTime *requestDeadTime) {
    (void)pwm;
    s_updateChannelsDeadTimeImmediate_count++;
    if (requestDeadTime != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_dead_rising_last[i]  = requestDeadTime[i].rising;
            s_dead_falling_last[i] = requestDeadTime[i].falling;
        }
        if (s_dead_historyCount < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_dead_rising_history[s_dead_historyCount][i]  = requestDeadTime[i].rising;
                s_dead_falling_history[s_dead_historyCount][i] = requestDeadTime[i].falling;
            }
            s_dead_historyCount++;
        }
    }
}

void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 requestFrequency) {
    (void)pwm;
    s_updateFrequencyImmediate_count++;
    s_updateFrequency_last = requestFrequency;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)config;
    (void)gtmSFR;
    s_initConfig_count++;
}

/* Mock controls */
uint32 IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDeadTimeImmediate(void) { return s_updateChannelsDeadTimeImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateFrequencyImmediate(void) { return s_updateFrequencyImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_duty_lastArray[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_duty_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_duty_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) { return s_duty_historyCount; }

float32 IfxGtm_Pwm_Mock_GetLastArg_updateFrequencyImmediate(void) { return s_updateFrequency_last; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDeadTimeImmediate_rising(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_dead_rising_last[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDeadTimeImmediate_falling(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_dead_falling_last[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDeadTimeImmediate_rising(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_dead_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_dead_rising_history[callIdx][elemIdx];
    }
    return 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDeadTimeImmediate_falling(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_dead_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_dead_falling_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDeadTimeImmediate(void) { return s_dead_historyCount; }

float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void) {
    s_startChannelOutputs_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_init_count = 0u;
    s_updateChannelsDeadTimeImmediate_count = 0u;
    s_updateFrequencyImmediate_count = 0u;
    s_initChannelConfig_count = 0u;
    s_initConfig_count = 0u;

    {
        uint32 i, j;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_duty_lastArray[i] = 0.0f;
            s_dead_rising_last[i] = 0.0f;
            s_dead_falling_last[i] = 0.0f;
        }
        for (i = 0u; i < MOCK_MAX_HISTORY; i++) {
            for (j = 0u; j < MOCK_MAX_ELEMENTS; j++) {
                s_duty_history[i][j] = 0.0f;
                s_dead_rising_history[i][j] = 0.0f;
                s_dead_falling_history[i][j] = 0.0f;
            }
        }
        s_duty_historyCount = 0u;
        s_dead_historyCount = 0u;
    }

    s_updateFrequency_last = 0.0f;
    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
