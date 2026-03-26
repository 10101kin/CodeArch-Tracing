#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_startChannelOutputs_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_init_count = 0;
static uint32 s_updateChannelsDeadTimeImmediate_count = 0;
static uint32 s_updateFrequencyImmediate_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_initConfig_count = 0;

/* Duty array capture with history */
static float32 s_updateDuty_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0;

/* Scalar captures */
static float32 s_updateFrequency_lastValue = 0.0f;

/* Pattern D: init() config captures */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Pattern D: initConfig() config captures */
static float32 s_initConfig_lastFrequency = 0.0f;
static uint32  s_initConfig_lastNumChannels = 0u;
static uint32  s_initConfig_lastAlignment = 0u;
static uint32  s_initConfig_lastSyncStart = 0u;

void IfxGtm_Pwm_startChannelOutputs(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startChannelOutputs_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateChannelsDutyImmediate_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY)
        {
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[s_updateDuty_historyCount][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR)
    {
        /* Pattern D: capture selected config fields */
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

void IfxGtm_Pwm_updateChannelsDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_DeadTime *requestDeadTime)
{
    (void)pwm;
    (void)requestDeadTime;
    s_updateChannelsDeadTimeImmediate_count++;
}

void IfxGtm_Pwm_updateFrequencyImmediate(IfxGtm_Pwm *pwm, float32 requestFrequency)
{
    (void)pwm;
    s_updateFrequencyImmediate_count++;
    s_updateFrequency_lastValue = requestFrequency;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)gtmSFR;
    s_initConfig_count++;
    if (config != NULL_PTR)
    {
        /* Pattern D: capture selected config fields */
        s_initConfig_lastFrequency   = config->frequency;
        s_initConfig_lastNumChannels = config->numChannels;
        s_initConfig_lastAlignment   = (uint32)config->alignment;
        s_initConfig_lastSyncStart   = (uint32)config->syncStart;
    }
}

/* Call count getters */
uint32 IfxGtm_Pwm_Mock_GetCallCount_startChannelOutputs(void) { return s_startChannelOutputs_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDeadTimeImmediate(void) { return s_updateChannelsDeadTimeImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateFrequencyImmediate(void) { return s_updateFrequencyImmediate_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }

/* Value capture getters */
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < IFXGTM_PWM_MOCK_MAX_HISTORY && elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS)
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateDuty_historyCount;
}

float32 IfxGtm_Pwm_Mock_GetLastArg_updateFrequencyImmediate(void)
{
    return s_updateFrequency_lastValue;
}

/* Pattern D getters */
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

float32 IfxGtm_Pwm_Mock_GetLastArg_initConfig_frequency(void) { return s_initConfig_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_numChannels(void) { return s_initConfig_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_alignment(void) { return s_initConfig_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_initConfig_syncStart(void) { return s_initConfig_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void)
{
    s_startChannelOutputs_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_init_count = 0u;
    s_updateChannelsDeadTimeImmediate_count = 0u;
    s_updateFrequencyImmediate_count = 0u;
    s_initChannelConfig_count = 0u;
    s_initConfig_count = 0u;

    {
        uint32 i, j;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = 0.0f;
        }
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_HISTORY; i++)
        {
            for (j = 0u; j < IFXGTM_PWM_MOCK_MAX_ELEMENTS; j++)
            {
                s_updateDuty_history[i][j] = 0.0f;
            }
        }
        s_updateDuty_historyCount = 0u;
    }

    s_updateFrequency_lastValue = 0.0f;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    s_initConfig_lastFrequency = 0.0f;
    s_initConfig_lastNumChannels = 0u;
    s_initConfig_lastAlignment = 0u;
    s_initConfig_lastSyncStart = 0u;
}
