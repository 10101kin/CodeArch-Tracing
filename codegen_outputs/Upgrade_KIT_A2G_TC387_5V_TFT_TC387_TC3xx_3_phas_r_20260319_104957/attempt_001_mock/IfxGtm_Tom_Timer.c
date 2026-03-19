#include "IfxGtm_Tom_Timer.h"

static uint32 s_init_count = 0u;
static float32 s_init_lastFrequency = 0.0f;
static Ifx_TimerValue s_init_lastPeriod = 0u;
static boolean s_init_ret = 0u;

static uint32 s_updateInputFreq_count = 0u;
static uint32 s_run_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_applyUpdate_count = 0u;
static uint32 s_disableUpdate_count = 0u;

static uint32 s_getPeriod_count = 0u;
static Ifx_TimerValue s_getPeriod_ret = 0u;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) {
    (void)driver;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency = config->frequency;
        s_init_lastPeriod = config->period;
    }
    return s_init_ret;
}

void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_updateInputFreq_count++;
}

void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_run_count++;
}

void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm) {
    (void)config;
    (void)gtm;
    s_initConfig_count++;
}

void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_applyUpdate_count++;
}

Ifx_TimerValue IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_getPeriod_count++;
    return s_getPeriod_ret;
}

void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_disableUpdate_count++;
}

/* Mock controls */
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void) { return s_init_count; }
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
Ifx_TimerValue IfxGtm_Tom_Timer_Mock_GetLastArg_init_period(void) { return s_init_lastPeriod; }
void IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean ret) { s_init_ret = ret; }

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_updateInputFrequency(void) { return s_updateInputFreq_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_run(void) { return s_run_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void) { return s_applyUpdate_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void) { return s_disableUpdate_count; }

void   IfxGtm_Tom_Timer_Mock_SetReturn_getPeriod(Ifx_TimerValue ret) { s_getPeriod_ret = ret; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_getPeriod(void) { return s_getPeriod_count; }

void IfxGtm_Tom_Timer_Mock_Reset(void) {
    s_init_count = 0u; s_init_lastFrequency = 0.0f; s_init_lastPeriod = 0u; s_init_ret = 0u;
    s_updateInputFreq_count = 0u; s_run_count = 0u; s_initConfig_count = 0u; s_applyUpdate_count = 0u; s_disableUpdate_count = 0u;
    s_getPeriod_count = 0u; s_getPeriod_ret = 0u;
}
