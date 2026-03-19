#include "IfxGtm_Pwm.h"

/* Provide global instance expected by tests */
IfxGtm_Pwm g_gtmTom3PhaseInverterPwm; /* undefined content, only symbol needed */

static uint32 s_startSyncedChannels_count = 0;
static uint32 s_init_count = 0;
static uint32 s_initConfig_count = 0;

#define PWM_MOCK_MAX_ELEMENTS 8u
#define PWM_MOCK_MAX_HISTORY  32u
static uint32 s_updateChannelsDutyImmediate_count = 0;
static float32 s_update_lastArray[PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[PWM_MOCK_MAX_HISTORY][PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0u;

static uint32 s_initChannelConfig_count = 0;

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    (void)config; /* Do not dereference (Pattern D optional, fields not defined in mock) */
    s_init_count++;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)config;
    (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < PWM_MOCK_MAX_HISTORY) {
            uint32 idx = s_update_historyCount;
            for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) {
                s_update_history[idx][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

uint32  IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_update_historyCount && elemIdx < PWM_MOCK_MAX_ELEMENTS) {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) { return s_update_historyCount; }
uint32  IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }

void IfxGtm_Pwm_Mock_Reset(void) {
    s_startSyncedChannels_count = 0u;
    s_init_count = 0u;
    s_initConfig_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    for (uint32 i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) { s_update_lastArray[i] = 0.0f; }
    for (uint32 c = 0u; c < PWM_MOCK_MAX_HISTORY; c++) {
        for (uint32 i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) { s_update_history[c][i] = 0.0f; }
    }
    s_update_historyCount = 0u;
    s_initChannelConfig_count = 0u;
}
