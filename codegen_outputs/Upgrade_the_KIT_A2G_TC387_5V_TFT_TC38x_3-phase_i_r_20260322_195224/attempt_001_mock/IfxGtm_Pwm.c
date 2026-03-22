#include "IfxGtm_Pwm.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_init_count = 0u;
static uint32 s_updateDuty_count = 0u;

/* PATTERN D: captured init config fields */
static float32 s_init_lastFrequency   = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment   = 0u;
static uint32  s_init_lastSyncStart   = 0u;

/* PATTERN B: array capture + history */
static float32 s_update_lastArray[MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_update_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_update_historyCount = 0u;

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    /* PATTERN D exception: capture known config fields if provided */
    if (config != NULL_PTR)
    {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
        {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < MOCK_MAX_HISTORY)
        {
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
            {
                s_update_history[s_update_historyCount][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

/* Mock controls */
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateDuty_count; }

float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void)   { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void)   { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void)   { return s_init_lastSyncStart; }

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_update_historyCount) && (elemIdx < MOCK_MAX_ELEMENTS))
    {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_update_historyCount;
}

void IfxGtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_init_count = 0u;
    s_updateDuty_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
    {
        s_update_lastArray[i] = 0.0f;
    }
    for (j = 0u; j < MOCK_MAX_HISTORY; j++)
    {
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++)
        {
            s_update_history[j][i] = 0.0f;
        }
    }
    s_update_historyCount = 0u;
}
