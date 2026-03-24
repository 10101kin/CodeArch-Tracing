#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_initConfig_count                = 0;
static uint32 s_init_count                      = 0;
static uint32 s_startSyncedChannels_count       = 0;
static uint32 s_initChannelConfig_count         = 0;
static uint32 s_updateChannelsDutyImmediate_cnt = 0;

/* Array capture (Pattern B) */
#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u
static float32 s_updateDuty_lastArray[MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0;

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)config;
    (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    (void)config; /* No dereference per rules (Pattern D skipped due to unknown fields) */
    s_init_count++;
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDutyImmediate_cnt++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0; i < MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < MOCK_MAX_HISTORY)
        {
            uint32 idx = s_updateDuty_historyCount;
            for (i = 0; i < MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[idx][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void)          { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void)                { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void)   { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_cnt; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateDuty_historyCount && callIdx < MOCK_MAX_HISTORY && elemIdx < MOCK_MAX_ELEMENTS)
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDuty_historyCount;
}

void IfxGtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_initConfig_count                = 0;
    s_init_count                      = 0;
    s_startSyncedChannels_count       = 0;
    s_initChannelConfig_count         = 0;
    s_updateChannelsDutyImmediate_cnt = 0;

    for (i = 0; i < MOCK_MAX_ELEMENTS; i++)
    {
        s_updateDuty_lastArray[i] = 0.0f;
    }
    for (i = 0; i < MOCK_MAX_HISTORY; i++)
    {
        for (j = 0; j < MOCK_MAX_ELEMENTS; j++)
        {
            s_updateDuty_history[i][j] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0;
}
