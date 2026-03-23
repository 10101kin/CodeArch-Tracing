#include "IfxGtm_Tom_Timer.h"

static uint32 s_init_count = 0;
static boolean s_init_ret = 0;

static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastPeriod = 0u;
static uint32  s_init_lastAlignment = 0u;
static uint32  s_init_lastStart = 0u;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) {
    (void)driver;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency = config->frequency;
        s_init_lastPeriod    = config->period;
        s_init_lastAlignment = (uint32)config->alignment;
        s_init_lastStart     = (uint32)config->start;
    }
    return s_init_ret;
}

uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void) { return s_init_count; }
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean ret) { s_init_ret = ret; }
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_period(void) { return s_init_lastPeriod; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_alignment(void) { return s_init_lastAlignment; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_start(void) { return s_init_lastStart; }

void IfxGtm_Tom_Timer_Mock_Reset(void) {
    s_init_count = 0u;
    s_init_ret = 0;
    s_init_lastFrequency = 0.0f;
    s_init_lastPeriod = 0u;
    s_init_lastAlignment = 0u;
    s_init_lastStart = 0u;
}
