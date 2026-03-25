#include "IfxGtm_Pwm.h"

/* Call counters */
static uint32 s_initConfig_count = 0u;
static uint32 s_updateDeadTime_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_updateDuty_count = 0u;
static uint32 s_setChannelPolarity_count = 0u;

/* Captured config fields for init() */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

/* Captured value params */
static uint32  s_last_updateDuty_index = 0u;
static float32 s_last_updateDuty_value = 0.0f;

static uint32  s_last_updateDead_index = 0u;
static float32 s_last_updateDead_rising = 0.0f;
static float32 s_last_updateDead_falling = 0.0f;

static uint32 s_last_setPolarity_subModule = 0u;
static uint32 s_last_setPolarity_channel = 0u;
static uint32 s_last_setPolarity_polarity = 0u;

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) {
    (void)config;
    (void)gtmSFR;
    s_initConfig_count++;
}

void IfxGtm_Pwm_updateChannelDeadTimeImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, IfxGtm_Pwm_DeadTime requestDeadTime) {
    (void)pwm;
    s_updateDeadTime_count++;
    s_last_updateDead_index  = (uint32)configIndex;
    s_last_updateDead_rising = requestDeadTime.rising;
    s_last_updateDead_falling = requestDeadTime.falling;
}

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
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

void IfxGtm_Pwm_initChannelConfig(IfxGtm_Pwm_ChannelConfig *channelConfig) {
    (void)channelConfig;
    s_initChannelConfig_count++;
}

void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm) {
    (void)pwm;
    s_startSyncedChannels_count++;
}

void IfxGtm_Pwm_updateChannelDutyImmediate(IfxGtm_Pwm *pwm, IfxGtm_Pwm_SyncChannelIndex configIndex, float32 requestDuty) {
    (void)pwm;
    s_updateDuty_count++;
    s_last_updateDuty_index = (uint32)configIndex;
    s_last_updateDuty_value = requestDuty;
}

void IfxGtm_Pwm_setChannelPolarity(IfxGtm_Pwm_ClusterSFR *clusterSFR, IfxGtm_Pwm_SubModule subModule, IfxGtm_Pwm_SubModule_Ch channel, Ifx_ActiveState polarity) {
    (void)clusterSFR;
    s_setChannelPolarity_count++;
    s_last_setPolarity_subModule = (uint32)subModule;
    s_last_setPolarity_channel   = (uint32)channel;
    s_last_setPolarity_polarity  = (uint32)polarity;
}

/* Mock control */
uint32 IfxGtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDeadTimeImmediate(void) { return s_updateDeadTime_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_updateChannelDutyImmediate(void) { return s_updateDuty_count; }
uint32 IfxGtm_Pwm_Mock_GetCallCount_setChannelPolarity(void) { return s_setChannelPolarity_count; }

float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

uint32  IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_index(void) { return s_last_updateDuty_index; }
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDutyImmediate_duty(void) { return s_last_updateDuty_value; }

uint32  IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_index(void) { return s_last_updateDead_index; }
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_rising(void) { return s_last_updateDead_rising; }
float32 IfxGtm_Pwm_Mock_GetLastArg_updateChannelDeadTimeImmediate_falling(void) { return s_last_updateDead_falling; }

uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_subModule(void) { return s_last_setPolarity_subModule; }
uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_channel(void) { return s_last_setPolarity_channel; }
uint32 IfxGtm_Pwm_Mock_GetLastArg_setChannelPolarity_polarity(void) { return s_last_setPolarity_polarity; }

void IfxGtm_Pwm_Mock_Reset(void) {
    s_initConfig_count = 0u;
    s_updateDeadTime_count = 0u;
    s_init_count = 0u;
    s_initChannelConfig_count = 0u;
    s_startSyncedChannels_count = 0u;
    s_updateDuty_count = 0u;
    s_setChannelPolarity_count = 0u;

    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;

    s_last_updateDuty_index = 0u;
    s_last_updateDuty_value = 0.0f;

    s_last_updateDead_index = 0u;
    s_last_updateDead_rising = 0.0f;
    s_last_updateDead_falling = 0.0f;

    s_last_setPolarity_subModule = 0u;
    s_last_setPolarity_channel = 0u;
    s_last_setPolarity_polarity = 0u;
}
