#include "IfxEgtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_initConfig_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_startChannelOutputs_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_getChannelState_count = 0u;
static uint32 s_updateDutyImmediate_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_updateDeadTime_count = 0u;
static uint32 s_setChannelPolarity_count = 0u;
static uint32 s_updateDuty_count = 0u;

/* Return values */
static IfxEgtm_Pwm_ChannelState s_getChannelState_ret = IfxEgtm_Pwm_ChannelState_stopped;

/* Captured config fields for init() */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Captured scalars/enums */
static uint32 s_setChannelPolarity_lastSubModule = 0u;
static uint32 s_setChannelPolarity_lastChannel   = 0u;
static uint32 s_setChannelPolarity_lastPolarity  = 0u;

/* Array capture for duty updates */
static float32 s_updateDutyImmediate_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDutyImmediate_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDutyImmediate_historyCount = 0u;

static float32 s_updateDuty_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment = (uint32)config->alignment;
        s_init_lastSyncStart = (uint32)config->syncStart;
    }
}

void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startChannelOutputs_count++;
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_getChannelState(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_SubModule_Ch channel)
{
    (void)pwm;
    (void)channel;
    s_getChannelState_count++;
    return s_getChannelState_ret;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDutyImmediate_count++;
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_lastArray[i] = requestDuty[i];
        }
        if (s_updateDutyImmediate_historyCount < MOCK_MAX_HISTORY) {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_updateDutyImmediate_history[s_updateDutyImmediate_historyCount][i] = requestDuty[i];
            }
            s_updateDutyImmediate_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_DeadTime *requestDeadTime)
{
    (void)pwm;
    (void)requestDeadTime;
    s_updateDeadTime_count++;
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity)
{
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_setChannelPolarity_lastSubModule = (uint32)subModule;
    s_setChannelPolarity_lastChannel = (uint32)channel;
    s_setChannelPolarity_lastPolarity = (uint32)polarity;
}

void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDuty_count++;
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

/* Mock controls */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_getChannelState(void) { return s_getChannelState_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void) { return s_updateDeadTime_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateDuty_count; }

void IfxEgtm_Pwm_Mock_SetReturn_getChannelState(IfxEgtm_Pwm_ChannelState value) { s_getChannelState_ret = value; }
IfxEgtm_Pwm_ChannelState IfxEgtm_Pwm_Mock_GetReturn_getChannelState(void) { return s_getChannelState_ret; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_setChannelPolarity_lastSubModule; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_setChannelPolarity_lastChannel; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setChannelPolarity_lastPolarity; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDutyImmediate_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateDutyImmediate_historyCount && elemIdx < MOCK_MAX_ELEMENTS)
        return s_updateDutyImmediate_history[callIdx][elemIdx];
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDutyImmediate_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateDuty_historyCount && elemIdx < MOCK_MAX_ELEMENTS)
        return s_updateDuty_history[callIdx][elemIdx];
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void)
{
    return s_updateDuty_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    s_initConfig_count = 0u; s_init_count = 0u; s_startChannelOutputs_count = 0u; s_startSyncedChannels_count = 0u;
    s_getChannelState_count = 0u; s_updateDutyImmediate_count = 0u; s_initChannelConfig_count = 0u; s_updateDeadTime_count = 0u;
    s_setChannelPolarity_count = 0u; s_updateDuty_count = 0u;

    s_getChannelState_ret = IfxEgtm_Pwm_ChannelState_stopped;

    s_init_lastFrequency = 0.0f; s_init_lastNumChannels = 0u; s_init_lastAlignment = 0u; s_init_lastSyncStart = 0u;

    s_setChannelPolarity_lastSubModule = 0u; s_setChannelPolarity_lastChannel = 0u; s_setChannelPolarity_lastPolarity = 0u;

    for (uint32 i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
        s_updateDutyImmediate_lastArray[i] = 0.0f;
        s_updateDuty_lastArray[i] = 0.0f;
    }
    for (uint32 c = 0u; c < MOCK_MAX_HISTORY; c++) {
        for (uint32 i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_updateDutyImmediate_history[c][i] = 0.0f;
            s_updateDuty_history[c][i] = 0.0f;
        }
    }
    s_updateDutyImmediate_historyCount = 0u;
    s_updateDuty_historyCount = 0u;
}
