#include "IfxEgtm_Pwm.h"

/* Call counters */
static uint32 s_initChannelConfig_count             = 0;
static uint32 s_initConfig_count                    = 0;
static uint32 s_init_count                          = 0;
static uint32 s_updateChannelsDuty_count            = 0;
static uint32 s_startSyncedChannels_count           = 0;
static uint32 s_updateChannelsDutyImmediate_count   = 0;

/* Array capture storage */
#define EGTM_PWM_MOCK_MAX_ELEMENTS  (8u)
#define EGTM_PWM_MOCK_MAX_HISTORY   (32u)

static float32 s_update_lastArray[EGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_update_history[EGTM_PWM_MOCK_MAX_HISTORY][EGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_update_historyCount = 0;

static float32 s_updateImm_lastArray[EGTM_PWM_MOCK_MAX_ELEMENTS] = {0};
static float32 s_updateImm_history[EGTM_PWM_MOCK_MAX_HISTORY][EGTM_PWM_MOCK_MAX_ELEMENTS] = {{0}};
static uint32  s_updateImm_historyCount = 0;

/* Init config captured fields (if provided in struct) */
static float32 s_init_lastFrequency   = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment   = 0u;
static uint32  s_init_lastSyncStart   = 0u;

void IfxEgtm_Pwm_initChannelConfig(IfxEgtm_Pwm_ChannelConfig *channelConfig)
{
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    s_initConfig_count++;
    if (config != NULL_PTR) {
        /* Pattern D: capture if fields exist in our placeholder type */
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
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
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_lastArray[i] = requestDuty[i];
        }
        if (s_update_historyCount < EGTM_PWM_MOCK_MAX_HISTORY) {
            uint32 h = s_update_historyCount;
            for (i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_update_history[h][i] = requestDuty[i];
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
    if (requestDuty != NULL_PTR) {
        uint32 i;
        for (i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_updateImm_lastArray[i] = requestDuty[i];
        }
        if (s_updateImm_historyCount < EGTM_PWM_MOCK_MAX_HISTORY) {
            uint32 h = s_updateImm_historyCount;
            for (i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
                s_updateImm_history[h][i] = requestDuty[i];
            }
            s_updateImm_historyCount++;
        }
    }
}

/* Mock control implementations */
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initChannelConfig(void)           { return s_initChannelConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void)                  { return s_initConfig_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void)                        { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDuty(void)          { return s_updateChannelsDuty_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_startSyncedChannels(void)         { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDuty(uint32 index)
{
    return (index < EGTM_PWM_MOCK_MAX_ELEMENTS) ? s_update_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDuty(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_update_historyCount && elemIdx < EGTM_PWM_MOCK_MAX_ELEMENTS) {
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
    return (index < EGTM_PWM_MOCK_MAX_ELEMENTS) ? s_updateImm_lastArray[index] : 0.0f;
}

float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx)
{
    if (callIdx < s_updateImm_historyCount && elemIdx < EGTM_PWM_MOCK_MAX_ELEMENTS) {
        return s_updateImm_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void)
{
    return s_updateImm_historyCount;
}

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void)   { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void)   { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void)   { return s_init_lastSyncStart; }

void IfxEgtm_Pwm_Mock_Reset(void)
{
    s_initChannelConfig_count           = 0u;
    s_initConfig_count                  = 0u;
    s_init_count                        = 0u;
    s_updateChannelsDuty_count          = 0u;
    s_startSyncedChannels_count         = 0u;
    s_updateChannelsDutyImmediate_count = 0u;

    for (uint32 i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
        s_update_lastArray[i]    = 0.0f;
        s_updateImm_lastArray[i] = 0.0f;
    }
    for (uint32 h = 0; h < EGTM_PWM_MOCK_MAX_HISTORY; h++) {
        for (uint32 i = 0; i < EGTM_PWM_MOCK_MAX_ELEMENTS; i++) {
            s_update_history[h][i]    = 0.0f;
            s_updateImm_history[h][i] = 0.0f;
        }
    }
    s_update_historyCount    = 0u;
    s_updateImm_historyCount = 0u;

    s_init_lastFrequency   = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment   = 0u;
    s_init_lastSyncStart   = 0u;
}
