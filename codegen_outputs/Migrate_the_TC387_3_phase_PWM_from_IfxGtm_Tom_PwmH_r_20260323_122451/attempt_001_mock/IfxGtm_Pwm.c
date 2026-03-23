#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_updateChannelsDuty_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_initConfig_count = 0u;

/* Pattern B: array capture for updateChannelsDuty */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_updateChannelsDuty_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateChannelsDuty_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateChannelsDuty_historyCount = 0u;

/* Pattern D: config field capture for init/initConfig */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

static float32 s_initConfig_lastFrequency = 0.0f;
static uint32  s_initConfig_lastNumChannels = 0u;
static uint32  s_initConfig_lastAlignment = 0u;
static uint32  s_initConfig_lastSyncStart = 0u;

/* iLLD API implementations (stubs) */
void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

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
            uint32 idx = s_updateChannelsDuty_historyCount;
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateChannelsDuty_history[idx][i] = requestDuty[i];
            }
            s_updateChannelsDuty_historyCount++;
        }
    }
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR)
    {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    s_initConfig_count++;
    if (config != NULL_PTR)
    {
        s_initConfig_lastFrequency   = config->frequency;
        s_initConfig_lastNumChannels = config->numChannels;
        s_initConfig_lastAlignment   = (uint32)config->alignment;
        s_initConfig_lastSyncStart   = (uint32)config->syncStart;
    }
}

/* Mock control: call counts */
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void){ return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void)               { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void)         { return s_initConfig_count; }

/* Mock control: Pattern B getters */
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

/* Mock control: Pattern D getters for init */
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void)   { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void)   { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void)   { return s_init_lastSyncStart; }

/* Mock control: Pattern D getters for initConfig */
float32 IfxGtm_Pwm_Mock_GetLastArg_initConfig_frequency(void)   { return s_initConfig_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void) { return s_initConfig_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_alignment(void)   { return s_initConfig_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void)   { return s_initConfig_lastSyncStart; }

/* Mock control: Reset */
void IfxGtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_initChannelConfig_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateChannelsDuty_count = 0u;
    s_init_count = 0u;
    s_initConfig_count = 0u;

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

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    s_initConfig_lastFrequency = 0.0f;
    s_initConfig_lastNumChannels = 0u;
    s_initConfig_lastAlignment = 0u;
    s_initConfig_lastSyncStart = 0u;
}
