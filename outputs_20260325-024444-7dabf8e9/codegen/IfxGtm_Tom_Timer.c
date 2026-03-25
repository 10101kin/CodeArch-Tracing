#include "IfxGtm_Tom_Timer.h"

static uint32 s_applyUpdate_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_disableUpdate_count = 0u;

static boolean s_init_ret = 0u;

/* Captured config fields for init() */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_applyUpdate_count++;
}

void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm) {
    (void)config;
    (void)gtm;
    s_initConfig_count++;
}

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) {
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

void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_disableUpdate_count++;
}

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void) { return s_applyUpdate_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void) { return s_disableUpdate_count; }

void IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value) { s_init_ret = value; }

float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxGtm_Tom_Timer_Mock_Reset(void) {
    s_applyUpdate_count = 0u;
    s_initConfig_count = 0u;
    s_init_count = 0u;
    s_disableUpdate_count = 0u;
    s_init_ret = 0u;
    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
