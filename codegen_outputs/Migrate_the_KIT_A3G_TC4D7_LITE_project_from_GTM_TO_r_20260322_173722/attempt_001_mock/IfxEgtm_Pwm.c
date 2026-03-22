#include "IfxEgtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

static uint32  s_update_count = 0;
static float32 s_update_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0;

static uint32 s_initConfig_count = 0;
static uint32 s_startSynced_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_init_count = 0;

/* Config capture */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

uint32  IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_updateChannelsDutyImmediate(void) { return s_update_count; }
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_update_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) { return s_update_historyCount; }

uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_startSyncedChannels(void) { return s_startSynced_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_init(void) { return s_init_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxEgtm_Pwm_Mock_Reset(void) {
    uint32 i, j;
    s_update_count = 0u;
    for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) { s_update_lastArray[i] = 0.0f; }
    for (i = 0u; i < MOCK_MAX_HISTORY; i++) {
        for (j = 0u; j < MOCK_MAX_ELEMENTS; j++) { s_update_history[i][j] = 0.0f; }
    }
    s_update_historyCount = 0u;

    s_initConfig_count = 0u;
    s_startSynced_count = 0u;
    s_initChannelConfig_count = 0u;
    s_init_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}

/* ── auto-injected missing function implementations ── */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(void) { /* auto-stub */}
void IfxEgtm_Pwm_initConfig(void) { /* auto-stub */}
void IfxEgtm_Pwm_startSyncedChannels(void) { /* auto-stub */}
void IfxEgtm_Pwm_initChannelConfig(void) { /* auto-stub */}
void IfxEgtm_Pwm_init(void) { /* auto-stub */}
