#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_updateChannelsDuty_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;

/* Pattern D captured fields for init */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0;
static uint32  s_init_lastAlignment = 0;
static uint32  s_init_lastSyncStart = 0;

/* Array capture for duty updates */
static float32 s_update_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0;

static float32 s_updateImm_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateImm_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateImm_historyCount = 0;

/* Implementations */
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
    s_init_count++;
    if (config != NULL_PTR)
    {
        /* Pattern D: capture commonly-verified configuration fields */
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
}

void IfxEgtm_Pwm_updateChannelsDuty(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDuty_count++;
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
            s_updateImm_lastArray[i] = requestDuty[i];
        }
        if (s_updateImm_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateImm_history[s_updateImm_historyCount][i] = requestDuty[i];
            }
            s_updateImm_historyCount++;
        }
    }
}

/* Mock control: call counts */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void)       { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void)              { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void)                    { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void)      { return s_updateChannelsDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void)     { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }

/* Mock control: Pattern D getters */
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

/* Mock control: array capture getters */
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_update_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDuty(void)
{
    return s_update_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateImm_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateImm_historyCount) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateImm_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateImm_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    s_initChannelConfig_count = 0;
    s_initConfig_count = 0;
    s_init_count = 0;
    s_updateChannelsDuty_count = 0;
    s_startSyncedChannels_count = 0;
    s_updateChannelsDutyImmediate_count = 0;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0;
    s_init_lastAlignment = 0;
    s_init_lastSyncStart = 0;

    for (uint32 i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
    {
        s_update_lastArray[i] = 0.0f;
        s_updateImm_lastArray[i] = 0.0f;
    }
    for (uint32 c = 0; c < IFXEGTM_PWM_MOCK_MAX_HISTORY; c++)
    {
        for (uint32 i = 0; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_update_history[c][i] = 0.0f;
            s_updateImm_history[c][i] = 0.0f;
        }
    }
    s_update_historyCount = 0;
    s_updateImm_historyCount = 0;
}
