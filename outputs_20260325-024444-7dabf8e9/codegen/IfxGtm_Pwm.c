#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_initConfig_count = 0;
static uint32 s_updateDeadTime_count = 0;
static uint32 s_init_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_startSyncedChannels_count = 0;
static uint32 s_updateDuty_count = 0;
static uint32 s_setChannelPolarity_count = 0;

/* Captured values */
static float32 s_updateDuty_lastDuty = 0.0f;
static float32 s_updateDeadTime_lastRising = 0.0f;
static float32 s_updateDeadTime_lastFalling = 0.0f;
static uint32  s_setChannelPolarity_lastPolarity = 0u;

/* Pattern D captured fields */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* ==================== Stub implementations ==================== */
void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR)
{
    (void)config; (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, IfxGtm_Pwm_DeadTime requestDeadTime)
{
    (void)pwm; (void)configIndex;
    s_updateDeadTime_count++;
    s_updateDeadTime_lastRising = requestDeadTime.risingEdge;
    s_updateDeadTime_lastFalling = requestDeadTime.fallingEdge;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment = (uint32)config->alignment;
        s_init_lastSyncStart = (uint32)config->syncStart;
    }
}

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

void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, float32 requestDuty)
{
    (void)pwm; (void)configIndex;
    s_updateDuty_count++;
    s_updateDuty_lastDuty = requestDuty;
}

void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm_ClusterSFR *clusterSFR, IfxGtm_Pwm_SubModule subModule, IfxGtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity)
{
    (void)clusterSFR; (void)subModule; (void)channel;
    s_setChannelPolarity_count++;
    s_setChannelPolarity_lastPolarity = (uint32)polarity;
}

/* ==================== Mock controls ==================== */
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate(void) { return s_updateDeadTime_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate(void) { return s_updateDuty_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }

float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty(void) { return s_updateDuty_lastDuty; }
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_risingEdge(void) { return s_updateDeadTime_lastRising; }
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_fallingEdge(void) { return s_updateDeadTime_lastFalling; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_setChannelPolarity_lastPolarity; }

float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void)
{
    s_initConfig_count = 0u;
    s_updateDeadTime_count = 0u;
    s_init_count = 0u;
    s_initChannelConfig_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateDuty_count = 0u;
    s_setChannelPolarity_count = 0u;

    s_updateDuty_lastDuty = 0.0f;
    s_updateDeadTime_lastRising = 0.0f;
    s_updateDeadTime_lastFalling = 0.0f;
    s_setChannelPolarity_lastPolarity = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
