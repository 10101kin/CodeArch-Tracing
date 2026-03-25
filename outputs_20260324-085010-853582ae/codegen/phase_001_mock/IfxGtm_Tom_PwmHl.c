#include "IfxGtm_Tom_PwmHl.h"

static uint32  s_initConfig_count = 0;
static uint32  s_init_count = 0;
static boolean s_init_ret = (boolean)0;

/* Captured config fields */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0;
static uint32  s_init_lastAlignment = 0;
static uint32  s_init_lastSyncStart = 0;

void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config) {
    (void)config;
    s_initConfig_count++;
}

boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config) {
    (void)driver;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency   = config->frequency;
        s_init_lastNumChannels = config->numChannels;
        s_init_lastAlignment   = config->alignment;
        s_init_lastSyncStart   = config->syncStart;
    }
    return s_init_ret;
}

uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void) { return s_init_count; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value) { s_init_ret = value; }

float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Tom_PwmHl_Mock_Reset(void) {
    s_initConfig_count = 0;
    s_init_count = 0;
    s_init_ret = (boolean)0;
    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0;
    s_init_lastAlignment = 0;
    s_init_lastSyncStart = 0;
}
