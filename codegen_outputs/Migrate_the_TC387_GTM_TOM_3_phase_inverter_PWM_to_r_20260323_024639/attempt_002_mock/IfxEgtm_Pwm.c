#include "IfxEgtm_Pwm.h"

/* Counters */
static uint32 s_initConfig_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_setChannelPolarity_count = 0;
static uint32 s_interruptHandler_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_init_count = 0;
static uint32 s_startSyncedChannels_count = 0;

/* Capture storage */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

static float32 s_initConfig_lastFrequency = 0.0f;
static uint32  s_initConfig_lastNumChannels = 0u;
static uint32  s_initConfig_lastAlignment = 0u;
static uint32  s_initConfig_lastSyncStart = 0u;

static float32 s_initChannelConfig_lastDuty = 0.0f;
static uint32  s_initChannelConfig_lastPolarity = 0u;
static uint32  s_initChannelConfig_lastSubModule = 0u;
static uint32  s_initChannelConfig_lastChannel = 0u;

static uint32  s_setPolarity_lastSubModule = 0u;
static uint32  s_setPolarity_lastChannel = 0u;
static uint32  s_setPolarity_lastPolarity = 0u;

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u
static float32 s_updateDuty_lastArray[MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDuty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    s_initConfig_count++;
    if (config != NULL_PTR) {
        s_initConfig_lastFrequency = config->frequency;
        s_initConfig_lastNumChannels = config->numChannels;
        s_initConfig_lastAlignment = (uint32)config->alignment;
        s_initConfig_lastSyncStart = (uint32)config->syncStart;
    }
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    s_initChannelConfig_count++;
    if (channelConfig != NULL_PTR) {
        s_initChannelConfig_lastDuty = channelConfig->duty;
        s_initChannelConfig_lastPolarity = (uint32)channelConfig->polarity;
        s_initChannelConfig_lastSubModule = (uint32)channelConfig->subModule;
        s_initChannelConfig_lastChannel = (uint32)channelConfig->channel;
    }
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity)
{
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_setPolarity_lastSubModule = (uint32)subModule;
    s_setPolarity_lastChannel   = (uint32)channel;
    s_setPolarity_lastPolarity  = (uint32)polarity;
}

void IfxEgtm_Pwm_interruptHandler(IfxEgtm_Pwm_Channel *channel, void *data)
{
    (void)channel;
    (void)data;
    s_interruptHandler_count++;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
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

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

/* Getters for counters */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }

/* Capture getters */
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_initConfig_frequency(void) { return s_initConfig_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void) { return s_initConfig_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_alignment(void) { return s_initConfig_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void) { return s_initConfig_lastSyncStart; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_duty(void) { return s_initChannelConfig_lastDuty; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_polarity(void) { return s_initChannelConfig_lastPolarity; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_subModule(void) { return s_initChannelConfig_lastSubModule; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_initChannelConfig_channel(void) { return s_initChannelConfig_lastChannel; }

uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_setPolarity_lastSubModule; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_setPolarity_lastChannel; }
uint32 IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setPolarity_lastPolarity; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateDuty_historyCount) && (elemIdx < MOCK_MAX_ELEMENTS)) {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty(void)
{
    return s_updateDuty_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_initConfig_count = 0u;
    s_initChannelConfig_count = 0u;
    s_setChannelPolarity_count = 0u;
    s_interruptHandler_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    s_initConfig_lastFrequency = 0.0f;
    s_initConfig_lastNumChannels = 0u;
    s_initConfig_lastAlignment = 0u;
    s_initConfig_lastSyncStart = 0u;

    s_initChannelConfig_lastDuty = 0.0f;
    s_initChannelConfig_lastPolarity = 0u;
    s_initChannelConfig_lastSubModule = 0u;
    s_initChannelConfig_lastChannel = 0u;

    s_setPolarity_lastSubModule = 0u;
    s_setPolarity_lastChannel = 0u;
    s_setPolarity_lastPolarity = 0u;

    for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
        s_updateDuty_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < MOCK_MAX_HISTORY; i++) {
        for (j = 0u; j < MOCK_MAX_ELEMENTS; j++) {
            s_updateDuty_history[i][j] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0u;
}
