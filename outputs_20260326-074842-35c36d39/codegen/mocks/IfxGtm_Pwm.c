#include "IfxGtm_Pwm.h"

/* Limits for array capture */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_updateChannelsDuty_count = 0;
static uint32 s_init_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_startChannelOutputs_count = 0;
static uint32 s_updateChannelsPulseImmediate_count = 0;

/* Array captures: updateChannelsDuty (duty only) */
static float32 s_updateChannelsDuty_lastDuty[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateChannelsDuty_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateChannelsDuty_historyCount = 0;

/* Array captures: updateChannelsPulseImmediate (phase + duty) */
static float32 s_updateChannelsPulseImmediate_lastPhase[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateChannelsPulseImmediate_lastDuty[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateChannelsPulseImmediate_history_phase[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static float32 s_updateChannelsPulseImmediate_history_duty [IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateChannelsPulseImmediate_historyCount = 0;

void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateChannelsDuty_lastDuty[i] = requestDuty[i];
        }
        if (s_updateChannelsDuty_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY) {
            for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateChannelsDuty_history[s_updateChannelsDuty_historyCount][i] = requestDuty[i];
            }
            s_updateChannelsDuty_historyCount++;
        }
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm; (void)channels; (void)config;
    s_init_count++;
    /* Pattern D exception intentionally skipped (unknown real fields) */
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
        for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateChannelsPulseImmediate_lastPhase[i] = requestPhase[i];
        }
    }
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateChannelsPulseImmediate_lastDuty[i] = requestDuty[i];
        }
    }
    if (s_updateChannelsPulseImmediate_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY) {
        uint32 i;
        for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateChannelsPulseImmediate_history_phase[s_updateChannelsPulseImmediate_historyCount][i] = (requestPhase != NULL_PTR) ? requestPhase[i] : 0.0f;
            s_updateChannelsPulseImmediate_history_duty [s_updateChannelsPulseImmediate_historyCount][i] = (requestDuty  != NULL_PTR) ? requestDuty[i]  : 0.0f;
        }
        s_updateChannelsPulseImmediate_historyCount++;
    }
}

/* Mock controls */
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty_duty(uint32 index) {
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateChannelsDuty_lastDuty[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty_duty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < IFXGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateChannelsDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty_duty(void) { return s_updateChannelsDuty_historyCount; }

uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }

uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsPulseImmediate(void) { return s_updateChannelsPulseImmediate_count; }
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_phase(uint32 index) {
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateChannelsPulseImmediate_lastPhase[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_phase(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < IFXGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateChannelsPulseImmediate_history_phase[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_phase(void) { return s_updateChannelsPulseImmediate_historyCount; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsPulseImmediate_duty(uint32 index) {
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateChannelsPulseImmediate_lastDuty[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsPulseImmediate_duty(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < IFXGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateChannelsPulseImmediate_history_duty[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPulseImmediate_duty(void) { return s_updateChannelsPulseImmediate_historyCount; }

void IfxGtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_updateChannelsDuty_count = 0;
    s_init_count = 0;
    s_initChannelConfig_count = 0;
    s_initConfig_count = 0;
    s_startChannelOutputs_count = 0;
    s_updateChannelsPulseImmediate_count = 0;

    for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
        s_updateChannelsDuty_lastDuty[i] = 0.0f;
        s_updateChannelsPulseImmediate_lastPhase[i] = 0.0f;
        s_updateChannelsPulseImmediate_lastDuty[i] = 0.0f;
    }
    for (j = 0; j < IFXGTM_PWM_MOCK_MAX_HISTORY; j++) {
        for (i = 0; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateChannelsDuty_history[j][i] = 0.0f;
            s_updateChannelsPulseImmediate_history_phase[j][i] = 0.0f;
            s_updateChannelsPulseImmediate_history_duty[j][i] = 0.0f;
        }
    }
    s_updateChannelsDuty_historyCount = 0;
    s_updateChannelsPulseImmediate_historyCount = 0;
}
