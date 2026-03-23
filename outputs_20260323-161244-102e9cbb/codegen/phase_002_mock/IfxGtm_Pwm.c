#include "IfxGtm_Pwm.h"

static uint32 s_init_count = 0;
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = (uint32)config->alignment;
        s_init_lastSyncStart   = (uint32)config->syncStart;
    }
}

uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Pwm_Mock_Reset(void) {
    s_init_count = 0u;
    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
