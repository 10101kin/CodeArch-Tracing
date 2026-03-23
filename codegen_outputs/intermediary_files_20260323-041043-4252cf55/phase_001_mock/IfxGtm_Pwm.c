#include "IfxGtm_Pwm.h"

/* Limits for array capture */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_updateChannelsDuty_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;

/* Array capture for updateChannelsDuty */
static float32 s_updateChannelsDuty_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateChannelsDuty_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateChannelsDuty_historyCount = 0u;

void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateChannelsDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateChannelsDuty_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateChannelsDuty_history[s_updateChannelsDuty_historyCount][i] = requestDuty[i];
            }
            s_updateChannelsDuty_historyCount++;
        }
    }
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

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
    (void)config;
    s_init_count++;
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

/* Mock controls */
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index)
{
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateChannelsDuty_lastArray[index] : 0.0f;
}
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateChannelsDuty_historyCount) && (elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateChannelsDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}
uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void)
{
    return s_updateChannelsDuty_historyCount;
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }

void IfxGtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_updateChannelsDuty_count = 0u;
    s_initChannelConfig_count = 0u;
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;

    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
    {
        s_updateChannelsDuty_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_HISTORY; i++)
    {
        for (j = 0u; j < IFXGTM_PWM_MOCK_MAX_ELEMENTS; j++)
        {
            s_updateChannelsDuty_history[i][j] = 0.0f;
        }
    }
    s_updateChannelsDuty_historyCount = 0u;
}
