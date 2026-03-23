#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_initConfig_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_startChannelOutputs_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_getChannelState_count = 0u;
static uint32 s_updateChannelsDutyImmediate_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_updateChannelsDeadTime_count = 0u;
static uint32 s_setChannelPolarity_count = 0u;
static uint32 s_updateChannelsDuty_count = 0u;

/* Return values */
static IfxEgtm_Pwm_ChannelState s_getChannelState_ret = (IfxEgtm_Pwm_ChannelState)0;

/* Last-argument captures */
static uint32 s_getChannelState_lastChannel = 0u;
static uint32 s_setChannelPolarity_lastSubModule = 0u;
static uint32 s_setChannelPolarity_lastChannel = 0u;
static uint32 s_setChannelPolarity_lastPolarity = 0u;

/* Optional Pattern D captured values for init() (left at 0 unless known fields are captured externally) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Array capture machinery */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u

static float32 s_updateDutyImmediate_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDutyImmediate_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDutyImmediate_historyCount = 0u;

static float32 s_updateDuty_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDuty_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR) {
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    /* Pattern D: Do NOT dereference unknown config fields here to avoid dependency on real iLLD types */
    (void)config;
    s_init_count++;
}

void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm) {
    (void)pwm;
    s_startChannelOutputs_count++;
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch channel) {
    (void)pwm;
    s_getChannelState_count++;
    s_getChannelState_lastChannel = (uint32)channel;
    return s_getChannelState_ret;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_lastArray[i] = requestDuty[i];
        }
        if (s_updateDutyImmediate_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY) {
            uint32 h = s_updateDutyImmediate_historyCount;
            for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateDutyImmediate_history[h][i] = requestDuty[i];
            }
            s_updateDutyImmediate_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_DeadTime *requestDeadTime) {
    (void)pwm;
    (void)requestDeadTime;
    s_updateChannelsDeadTime_count++;
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity) {
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_setChannelPolarity_lastSubModule = (uint32)subModule;
    s_setChannelPolarity_lastChannel   = (uint32)channel;
    s_setChannelPolarity_lastPolarity  = (uint32)polarity;
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
            uint32 h = s_updateDuty_historyCount;
            for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateDuty_history[h][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

/* Mock controls: call counts */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_getChannelState(void) { return s_getChannelState_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void) { return s_updateChannelsDeadTime_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }

/* Mock controls: return value setters */
void IfxEgtm_Pwm_Mock_SetReturn_getChannelState(IfxEgtm_Pwm_ChannelState value) { s_getChannelState_ret = value; }

/* Last-argument getters */
uint32 IfxEgtm_Pwm_Mock_GetLastArg_getChannelState_channel(void) { return s_getChannelState_lastChannel; }

/* Array capture getters (immediate) */
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index) {
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDutyImmediate_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < IFXEGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateDutyImmediate_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void) {
    return s_updateDutyImmediate_historyCount;
}

/* Array capture getters (shadow) */
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

/* Scalar capture getters for setChannelPolarity */
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_setChannelPolarity_lastSubModule; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_setChannelPolarity_lastChannel; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setChannelPolarity_lastPolarity; }

/* Optional Pattern D getters (remain defaults) */
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxEgtm_Pwm_Mock_Reset(void) {
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_startChannelOutputs_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_getChannelState_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_initChannelConfig_count = 0u;
    s_updateChannelsDeadTime_count = 0u;
    s_setChannelPolarity_count = 0u;
    s_updateChannelsDuty_count = 0u;

    s_getChannelState_ret = (IfxEgtm_Pwm_ChannelState)0;

    s_getChannelState_lastChannel = 0u;
    s_setChannelPolarity_lastSubModule = 0u;
    s_setChannelPolarity_lastChannel = 0u;
    s_setChannelPolarity_lastPolarity = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    {
        uint32 i, j;
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_lastArray[i] = 0.0f;
            s_updateDuty_lastArray[i] = 0.0f;
        }
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_HISTORY; i++) {
            for (j = 0u; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; j++) {
                s_updateDutyImmediate_history[i][j] = 0.0f;
                s_updateDuty_history[i][j] = 0.0f;
            }
        }
    }
    s_updateDutyImmediate_historyCount = 0u;
    s_updateDuty_historyCount = 0u;
}
