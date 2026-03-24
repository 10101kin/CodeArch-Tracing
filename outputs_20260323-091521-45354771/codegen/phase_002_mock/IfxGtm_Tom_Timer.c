#include "IfxGtm_Tom_Timer.h"

static uint32 s_init_count = 0;
static boolean s_init_ret = 0; /* default FALSE */

/* Pattern D: capture key config fields */
static float32 s_init_lastFrequency = 0.0f;
static uint32  s_init_lastMode = 0;
static uint32  s_init_lastSyncStart = 0;
static uint32  s_init_lastIsrPriority = 0;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) {
    (void)driver;
    s_init_count++;
    if (config != NULL_PTR) {
        s_init_lastFrequency = config->frequency;
        s_init_lastMode = (uint32)config->mode;
        s_init_lastSyncStart = (uint32)config->syncStart;
        s_init_lastIsrPriority = (uint32)config->isrPriority;
    }
    return s_init_ret;
}

uint32  IfxGtm_Tom_Timer_Mock_GetCallCount_init(void) { return s_init_count; }
void    IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value) { s_init_ret = value; }
float32 IfxGtm_Tom_Timer_Mock_GetLastArg_init_frequency(void) { return s_init_lastFrequency; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_mode(void) { return s_init_lastMode; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_syncStart(void) { return s_init_lastSyncStart; }
uint32  IfxGtm_Tom_Timer_Mock_GetLastArg_init_isrPriority(void) { return s_init_lastIsrPriority; }

void IfxGtm_Tom_Timer_Mock_Reset(void) {
    s_init_count = 0;
    s_init_ret = 0;
    s_init_lastFrequency = 0.0f;
    s_init_lastMode = 0;
    s_init_lastSyncStart = 0;
    s_init_lastIsrPriority = 0;
}
