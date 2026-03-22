#include "IfxEgtm_Pwm.h"

static uint32 s_init_count = 0u;
static uint32 s_initConfig_count = 0u;

/* Pattern D captured values (remain 0 if config is opaque/unavailable) */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastNumChannels = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastSyncStart = 0u;

void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config) {
    (void)pwm;
    (void)channels;
    s_init_count++;
    /* EXCEPTION: If config struct definition is available in the build, tests may extend
       this mock to capture real fields. Here we avoid dereferencing opaque pointers. */
    (void)config;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR) {
    (void)config;
    (void)egtmSFR;
    s_initConfig_count++;
}

uint32 IfxEgtm_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }

float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void) { return s_init_lastNumChannels; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }

void IfxEgtm_Pwm_Mock_Reset(void) {
    s_init_count = 0u;
    s_initConfig_count = 0u;
    s_init_lastFrequency = 0.0f;
    s_init_lastNumChannels = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastSyncStart = 0u;
}
