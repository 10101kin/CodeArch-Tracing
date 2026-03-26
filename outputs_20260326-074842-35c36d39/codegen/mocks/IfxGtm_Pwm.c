#include "IfxGtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_updateChannelsDuty_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_startChannelOutputs_count = 0u;
static uint32 s_updateChannelsPulseImmediate_count = 0u;

/* Array captures for updateChannelsDuty */
static float32 s_updateDuty_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0u;

/* Array captures for updateChannelsPulseImmediate */
static float32 s_updatePulse_lastPhase[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updatePulse_lastDuty[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updatePulse_historyPhase[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static float32 s_updatePulse_historyDuty[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updatePulse_historyCountPhase = 0u;
static uint32  s_updatePulse_historyCountDuty  = 0u;

void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_updateDuty_history[s_updateDuty_historyCount][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    (void)config; /* Pattern D: no field capture due to unknown layout in mock */
    s_init_count++;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)config; (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startChannelOutputs_count++;
}

void IfxGtm_Pwm_updateChannelsPulseImmediate(IfxGtm_Pwm *pwm, float32 *requestPhase, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsPulseImmediate_count++;
    if (requestPhase != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updatePulse_lastPhase[i] = requestPhase[i];
        }
        if (s_updatePulse_historyCountPhase < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_updatePulse_historyPhase[s_updatePulse_historyCountPhase][i] = requestPhase[i];
            }
            s_updatePulse_historyCountPhase++;
        }
    }
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updatePulse_lastDuty[i] = requestDuty[i];
        }
        if (s_updatePulse_historyCountDuty < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_updatePulse_historyDuty[s_updatePulse_historyCountDuty][i] = requestDuty[i];
            }
            s_updatePulse_historyCountDuty++;
        }
    }
}

/* Mock control implementations */
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate(void) { return s_updateChannelsPulseImmediate_count; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_requestDuty(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty_requestDuty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_updateDuty_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_requestDuty(void) {
    return s_updateDuty_historyCount;
}

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestPhase(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_updatePulse_lastPhase[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_requestDuty(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_updatePulse_lastDuty[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_requestPhase(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_updatePulse_historyCountPhase && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_updatePulse_historyPhase[callIdx][elemIdx];
    }
    return 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_requestDuty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_updatePulse_historyCountDuty && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_updatePulse_historyDuty[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestPhase(void) {
    return s_updatePulse_historyCountPhase;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_requestDuty(void) {
    return s_updatePulse_historyCountDuty;
}

void IfxGtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_updateChannelsDuty_count = 0u;
    s_init_count = 0u;
    s_initChannelConfig_count = 0u;
    s_initConfig_count = 0u;
    s_startChannelOutputs_count = 0u;
    s_updateChannelsPulseImmediate_count = 0u;

    for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
        s_updateDuty_lastArray[i] = 0.0f;
        s_updatePulse_lastPhase[i] = 0.0f;
        s_updatePulse_lastDuty[i] = 0.0f;
    }
    for (j = 0u; j < MOCK_MAX_HISTORY; j++) {
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updateDuty_history[j][i] = 0.0f;
            s_updatePulse_historyPhase[j][i] = 0.0f;
            s_updatePulse_historyDuty[j][i] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0u;
    s_updatePulse_historyCountPhase = 0u;
    s_updatePulse_historyCountDuty = 0u;
}
