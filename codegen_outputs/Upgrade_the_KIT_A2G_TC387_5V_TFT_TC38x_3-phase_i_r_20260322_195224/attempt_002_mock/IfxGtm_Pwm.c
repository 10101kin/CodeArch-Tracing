#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_init_count = 0u;
static uint32 s_updateDuty_count = 0u;

/* Array capture for requestDuty (Pattern B) */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u

static float32 s_updateDuty_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_updateDuty_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_updateDuty_historyCount = 0u;

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    (void)config; /* Do not dereference in mock (no local typedef of config fields) */
    s_init_count++;
}

void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    s_updateDuty_count++;
    if (requestDuty != NULL_PTR)
    {
        uint32 i;
        for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
        {
            s_updateDuty_lastArray[i] = requestDuty[i];
        }
        if (s_updateDuty_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY)
        {
            uint32 idx = s_updateDuty_historyCount;
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_updateDuty_history[idx][i] = requestDuty[i];
            }
            s_updateDuty_historyCount++;
        }
    }
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void)
{
    return s_init_count;
}

uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void)
{
    return s_updateDuty_count;
}

float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index)
{
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateDuty_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_updateDuty_historyCount) && (elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS))
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
    s_init_count = 0u;
    s_updateDuty_count = 0u;
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
