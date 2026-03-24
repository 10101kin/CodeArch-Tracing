#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_initChannelConfig_count = 0;

/* Array capture (Pattern B) */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_update_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0;

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_update_history[s_update_historyCount][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

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
    (void)config; /* Not dereferencing config in mock */
    s_init_count++;
}

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

/* Mock control: call counts */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }

/* Mock control: array capture getters */
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < IFXEGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS)
    {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_update_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_startSyncedChannels_count = 0;
    s_updateChannelsDutyImmediate_count = 0;
    s_initConfig_count = 0;
    s_init_count = 0;
    s_initChannelConfig_count = 0;

    for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
    {
        s_update_lastArray[i] = 0.0f;
    }
    for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_HISTORY; i++)
    {
        for (j = 0; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; j++)
        {
            s_update_history[i][j] = 0.0f;
        }
    }
    s_update_historyCount = 0;
}
