#include "IfxEgtm_Pwm.h"

static uint32 s_initConfig_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_setChannelPolarity_count = 0u;
static uint32 s_interruptHandler_count = 0u;
static uint32 s_updateChannelsDutyImmediate_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;

static uint32 s_setChannelPolarity_lastSubModule = 0u;
static uint32 s_setChannelPolarity_lastChannel = 0u;
static uint32 s_setChannelPolarity_lastPolarity = 0u;

#define PWM_MOCK_MAX_ELEMENTS 8u
#define PWM_MOCK_MAX_HISTORY  32u
static float32 s_updateDuty_lastArray[PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[PWM_MOCK_MAX_HISTORY][PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_setChannelPolarity(Ifx_EGTM_CLS *clusterSFR, IfxEgtm_Pwm_SubModule subModule, IfxEgtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity)
{
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_setChannelPolarity_lastSubModule = (uint32)subModule;
    s_setChannelPolarity_lastChannel = (uint32)channel;
    s_setChannelPolarity_lastPolarity = (uint32)polarity;
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
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < PWM_MOCK_MAX_HISTORY)
        {
            uint32 idx = s_updateDuty_historyCount;
            for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[idx][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    (void)config; /* Pattern D capture is skipped due to opaque config in mocks */
    s_init_count++;
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }

uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_setChannelPolarity_lastSubModule; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_setChannelPolarity_lastChannel; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setChannelPolarity_lastPolarity; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate_requestDuty(uint32 index)
{
    return (index < PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate_requestDuty(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateDuty_historyCount && elemIdx < PWM_MOCK_MAX_ELEMENTS)
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate_requestDuty(void)
{
    return s_updateDuty_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    s_initConfig_count = 0u;
    s_initChannelConfig_count = 0u;
    s_setChannelPolarity_count = 0u;
    s_interruptHandler_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;

    s_setChannelPolarity_lastSubModule = 0u;
    s_setChannelPolarity_lastChannel = 0u;
    s_setChannelPolarity_lastPolarity = 0u;

    {
        uint32 i, j;
        for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) { s_updateDuty_lastArray[i] = 0.0f; }
        for (j = 0u; j < PWM_MOCK_MAX_HISTORY; j++)
        {
            for (i = 0u; i < PWM_MOCK_MAX_ELEMENTS; i++) { s_updateDuty_history[j][i] = 0.0f; }
        }
        s_updateDuty_historyCount = 0u;
    }
}
