#include "IfxGtm_Tom_Timer.h"

static uint32 s_init_count = 0;
static boolean s_init_ret = (boolean)0;

static uint32 s_updateInputFrequency_count = 0;
static uint32 s_run_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_applyUpdate_count = 0;

static uint32 s_getPeriod_count = 0;
static Ifx_TimerValue s_getPeriod_ret = 0u;

static uint32 s_disableUpdate_count = 0;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) {
    (void)driver;
    (void)config; /* do not dereference */
    s_init_count++;
    return s_init_ret;
}

void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver) {
    (void)driver;
    s_updateInputFrequency_count++;
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

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void) { return s_init_count; }
void   IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value) { s_init_ret = value; }

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_updateInputFrequency(void) { return s_updateInputFrequency_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_run(void) { return s_run_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void) { return s_applyUpdate_count; }

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_getPeriod(void) { return s_getPeriod_count; }
void   IfxGtm_Tom_Timer_Mock_SetReturn_getPeriod(Ifx_TimerValue value) { s_getPeriod_ret = value; }

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void) { return s_disableUpdate_count; }

void IfxGtm_Tom_Timer_Mock_Reset(void) {
    s_init_count = 0; s_init_ret = (boolean)0;
    s_updateInputFrequency_count = 0;
    s_run_count = 0;
    s_initConfig_count = 0;
    s_applyUpdate_count = 0;
    s_getPeriod_count = 0; s_getPeriod_ret = 0u;
    s_disableUpdate_count = 0;
}
