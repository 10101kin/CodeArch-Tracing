#include "IfxEgtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

static uint32 s_initChannelConfig_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_updateDuty_count = 0u;
static uint32 s_updateFreq_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_interruptHandler_count = 0u;
static uint32 s_setChannelPolarity_count = 0u;
static uint32 s_updatePhase_count = 0u;
static uint32 s_startChannelOutputs_count = 0u;

static float32 s_updateFreq_lastFrequency = 0.0f;
static uint32  s_setChannelPolarity_lastSubModule = 0u;
static uint32  s_setChannelPolarity_lastChannel = 0u;
static uint32  s_setChannelPolarity_lastPolarity = 0u;

static float32 s_updateDuty_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0u;

static float32 s_updatePhase_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updatePhase_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updatePhase_historyCount = 0u;

/* Optional init() field captures (remain zero unless real type known) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < MOCK_MAX_HISTORY)
        {
            uint32 idx = s_updateDuty_historyCount;
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[idx][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_updateFrequencyImmediate(IfxEgtm_Pwm *pwm, float32 requestFrequency)
{
    (void)pwm;
    s_updateFreq_count++;
    s_updateFreq_lastFrequency = requestFrequency;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    /* Pattern D: would capture fields if known; keep defaults if unknown */
    (void)config;
}

void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm_Channel *channel, void *data)
{
    (void)channel;
    (void)data;
    s_interruptHandler_count++;
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity)
{
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_setChannelPolarity_lastSubModule = (uint32)subModule;
    s_setChannelPolarity_lastChannel = (uint32)channel;
    s_setChannelPolarity_lastPolarity = (uint32)polarity;
}

void IfxEgtm_Pwm_updateChannelsPhaseImmediate(IfxEgtm_Pwm *pwm, float32 *requestPhase)
{
    (void)pwm;
    s_updatePhase_count++;
    if (requestPhase != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
        {
            s_updatePhase_lastArray[i] = requestPhase[i];
        }
        if (s_updatePhase_historyCount < MOCK_MAX_HISTORY)
        {
            uint32 idx = s_updatePhase_historyCount;
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
            {
                s_updatePhase_history[idx][i] = requestPhase[i];
            }
            s_updatePhase_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_startChannelOutputs(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startChannelOutputs_count++;
}

/* Mock controls */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateFrequencyImmediate(void) { return s_updateFreq_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsPhaseImmediate(void) { return s_updatePhase_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_updateFrequencyImmediate_frequency(void) { return s_updateFreq_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_setChannelPolarity_lastSubModule; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_setChannelPolarity_lastChannel; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setChannelPolarity_lastPolarity; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateDuty_historyCount && elemIdx < MOCK_MAX_ELEMENTS)
        return s_updateDuty_history[callIdx][elemIdx];
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDuty_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsPhaseImmediate(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updatePhase_lastArray[index] : 0.0f;
}
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsPhaseImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updatePhase_historyCount && elemIdx < MOCK_MAX_ELEMENTS)
        return s_updatePhase_history[callIdx][elemIdx];
    return 0.0f;
}
uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsPhaseImmediate(void)
{
    return s_updatePhase_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxEgtm_Pwm_Mock_Reset(void)
{
    s_initChannelConfig_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_initConfig_count = 0u;
    s_updateDuty_count = 0u;
    s_updateFreq_count = 0u;
    s_init_count = 0u;
    s_interruptHandler_count = 0u;
    s_setChannelPolarity_count = 0u;
    s_updatePhase_count = 0u;
    s_startChannelOutputs_count = 0u;

    s_updateFreq_lastFrequency = 0.0f;
    s_setChannelPolarity_lastSubModule = 0u;
    s_setChannelPolarity_lastChannel = 0u;
    s_setChannelPolarity_lastPolarity = 0u;

    for (uint32 i = 0u; i < MOCK_MAX_ELEMENTS; i++) { s_updateDuty_lastArray[i] = 0.0f; s_updatePhase_lastArray[i] = 0.0f; }
    for (uint32 c = 0u; c < MOCK_MAX_HISTORY; c++) { for (uint32 i = 0u; i < MOCK_MAX_ELEMENTS; i++) { s_updateDuty_history[c][i] = 0.0f; s_updatePhase_history[c][i] = 0.0f; } }
    s_updateDuty_historyCount = 0u;
    s_updatePhase_historyCount = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
