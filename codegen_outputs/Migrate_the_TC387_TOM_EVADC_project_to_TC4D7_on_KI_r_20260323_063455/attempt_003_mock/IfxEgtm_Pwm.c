#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count        = 0;
static uint32 s_init_count              = 0;
static uint32 s_updateChannelsDuty_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;

/* Array capture buffers */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS  (8u)
#define IFXEGTM_PWM_MOCK_MAX_HISTORY   (32u)

static float32 s_updateDuty_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0;

static float32 s_updateDutyImm_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDutyImm_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDutyImm_historyCount = 0;

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
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
    (void)config; /* Pattern D field capture skipped due to opaque config in mock */
    s_init_count++;
}

void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    uint32 i;
    (void)pwm;
    s_updateChannelsDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++i)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++i)
            {
                s_updateDuty_history[s_updateDuty_historyCount][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxEgtm_Pwm_startSyncedChannels(IfxEgtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    uint32 i;
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR)
    {
        for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++i)
        {
            s_updateDutyImm_lastArray[i] = requestDuty[i];
        }
        if (s_updateDutyImm_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++i)
            {
                s_updateDutyImm_history[s_updateDutyImm_historyCount][i] = requestDuty[i];
            }
            s_updateDutyImm_historyCount++;
        }
    }
}

/* Call count getters */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void)      { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void)             { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void)                   { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void)     { return s_updateChannelsDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void)    { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }

/* Array capture getters */
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateDuty_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void)
{
    return s_updateDuty_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDutyImm_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateDutyImm_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateDutyImm_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDutyImm_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_initChannelConfig_count = 0;
    s_initConfig_count = 0;
    s_init_count = 0;
    s_updateChannelsDuty_count = 0;
    s_startSyncedChannels_count = 0;
    s_updateChannelsDutyImmediate_count = 0;

    for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++i)
    {
        s_updateDuty_lastArray[i] = 0.0f;
        s_updateDutyImm_lastArray[i] = 0.0f;
    }
    for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_HISTORY; ++i)
    {
        for (j = 0; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; ++j)
        {
            s_updateDuty_history[i][j] = 0.0f;
            s_updateDutyImm_history[i][j] = 0.0f;
        }
    }
    s_updateDuty_historyCount = 0;
    s_updateDutyImm_historyCount = 0;
}
