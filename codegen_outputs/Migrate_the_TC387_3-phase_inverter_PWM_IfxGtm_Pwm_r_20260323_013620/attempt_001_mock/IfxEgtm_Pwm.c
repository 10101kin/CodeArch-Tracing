#include "IfxEgtm_Pwm.h"

/* Counters */
static uint32 s_updateDuty_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_startSynced_count = 0;
static uint32 s_stopSynced_count = 0;
static uint32 s_initChCfg_count = 0;
static uint32 s_init_count = 0;

/* Pattern B: capture array parameters and history */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u

static float32 s_updateDuty_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDuty_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[s_updateDuty_historyCount][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSynced_count++;
}

void IfxEgtm_Pwm_stopSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_stopSynced_count++;
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChCfg_count++;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    (void)config; /* Pattern D: config field capture skipped (unknown struct fields) */
    s_init_count++;
}

uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSynced_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_stopSyncedChannels(void) { return s_stopSynced_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChCfg_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateDuty_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDuty_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_updateDuty_count = 0u;
    s_initConfig_count = 0u;
    s_startSynced_count = 0u;
    s_stopSynced_count = 0u;
    s_initChCfg_count = 0u;
    s_init_count = 0u;
    for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
    {
        s_updateDuty_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_HISTORY; i++)
    {
        for (j = 0u; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; j++)
        {
            s_updateDuty_history[i][j] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0u;
}
