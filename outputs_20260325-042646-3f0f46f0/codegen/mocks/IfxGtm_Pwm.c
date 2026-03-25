#include "IfxGtm_Pwm.h"

/* Counters */
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;

/* Pattern D: captured config fields (default if not present) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0;
static uint32  s_init_lastAlignment = 0;
static uint32  s_init_lastSyncStart = 0;

/* Pattern B: array capture for requestDuty */
#define IFXGTM_PWM_MOCK_MAX_ELEMENTS 8u
#define IFXGTM_PWM_MOCK_MAX_HISTORY  32u
static float32 s_update_lastArray[IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {0.0f};
static float32 s_update_history[IFXGTM_PWM_MOCK_MAX_HISTORY][IFXGTM_PWM_MOCK_MAX_ELEMENTS] = {{0.0f}};
static uint32  s_update_historyCount = 0u;

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
    s_init_count++;
    /* Pattern D: capture selected fields if available in the real config type.
       These conditionals allow building regardless of actual field presence. */
#ifdef IFXGTM_PWM_CONFIG_HAS_FREQUENCY
 if(config != NULL_PTR){ (void)NULL_PTR; }
#endif
#ifdef IFXGTM_PWM_CONFIG_HAS_NUMCHANNELS
 if(config != NULL_PTR){ (void)NULL_PTR; }
#endif
#ifdef IFXGTM_PWM_CONFIG_HAS_ALIGNMENT
 if(config != NULL_PTR){ (void)NULL_PTR; }
#endif
#ifdef IFXGTM_PWM_CONFIG_HAS_SYNCSTART
 if(config != NULL_PTR){ (void)NULL_PTR; }
#endif
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm)
{
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
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
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < IFXGTM_PWM_MOCK_MAX_HISTORY)
        {
            uint32 idx = s_update_historyCount;
            for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
            {
                s_update_history[idx][i] = requestDuty[i];
            }
            s_update_historyCount++;
        }
    }
}

/* Call count getters */
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }

/* Pattern D getters */
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

/* Pattern B getters */
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate_requestDuty(uint32 index)
{
    return (index < IFXGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate_requestDuty(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < IFXGTM_PWM_MOCK_MAX_HISTORY) && (elemIdx < IFXGTM_PWM_MOCK_MAX_ELEMENTS))
    {
        return s_update_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate_requestDuty(void)
{
    return s_update_historyCount;
}

void IfxGtm_Pwm_Mock_Reset(void)
{
    uint32 i, j;
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_initChannelConfig_count = 0u;
    s_updateChannelsDutyImmediate_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_ELEMENTS; i++)
    {
        s_update_lastArray[i] = 0.0f;
    }
    for (i = 0u; i < IFXGTM_PWM_MOCK_MAX_HISTORY; i++)
    {
        for (j = 0u; j < IFXGTM_PWM_MOCK_MAX_ELEMENTS; j++)
        {
            s_update_history[i][j] = 0.0f;
        }
    }
    s_update_historyCount = 0u;
}
