#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_init_count                     = 0u;
static uint32 s_startSyncedChannels_count      = 0u;
static uint32 s_updateChannelsDutyImmediate_count = 0u;
static uint32 s_initConfig_count               = 0u;

/* Array capture configuration */
#define IFXEGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXEGTM_PWM_MOCK_MAX_HISTORY  32u

static float32 s_updateDuty_lastArray[IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateDuty_history[IFXEGTM_PWM_MOCK_MAX_HISTORY][IFXEGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateDuty_historyCount = 0u;

/* Implementations */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    (void)config; /* Exception Pattern D not applied due to opaque config in mock */
    s_init_count++;
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
        for (i = 0u; i < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXEGTM_PWM_MOCK_MAX_HISTORY)
        {
            uint32 j;
            for (j = 0u; j < IFXEGTM_PWM_MOCK_MAX_ELEMENTS; j++)
            {
                s_updateDuty_history[s_updateDuty_historyCount][j] = requestDuty[j];
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

/* Mock control implementations */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateDuty(uint32 index)
{
    if (index < IFXEGTM_PWM_MOCK_MAX_ELEMENTS)
    {
        return s_updateDuty_lastArray[index];
    }
    return 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < IFXEGTM_PWM_MOCK_MAX_HISTORY) && (elemIdx < IFXEGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_updateDuty_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateDuty(void)
{
    return s_updateDuty_historyCount;
}

void IfxEgtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;
    s_initConfig_count = 0u;

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
