#include "IfxGtm_Tom_PwmHl.h"

#define MOCK_MAX_ELEMENTS 8u
#define MOCK_MAX_HISTORY  32u

/* Call counters */
static uint32 s_setMode_count = 0u;
static uint32 s_setOnTime_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_init_count = 0u;

/* Return controls */
static boolean s_setMode_ret = FALSE;
static boolean s_init_ret = FALSE;

/* Captured values */
static uint32        s_setMode_lastMode = 0u;
static Ifx_TimerValue s_setOnTime_lastArray[MOCK_MAX_ELEMENTS] = {0};
static Ifx_TimerValue s_setOnTime_history[MOCK_MAX_HISTORY][MOCK_MAX_ELEMENTS] = {{0}};
static uint32         s_setOnTime_historyCount = 0u;

boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode) {
    (void)driver;
    s_setMode_count++;
    s_setMode_lastMode = (uint32)mode;
    return s_setMode_ret;
}

void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn) {
    (void)driver;
    s_setOnTime_count++;
    if (tOn != NULL_PTR) {
        uint32 i;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
            s_setOnTime_lastArray[i] = tOn[i];
        }
        if (s_setOnTime_historyCount < MOCK_MAX_HISTORY) {
            uint32 idx = s_setOnTime_historyCount;
            for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) {
                s_setOnTime_history[idx][i] = tOn[i];
            }
            s_setOnTime_historyCount++;
        }
    }
}

void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config) {
    (void)config;
    s_initConfig_count++;
}

boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config) {
    (void)driver;
    (void)config;
    s_init_count++;
    return s_init_ret;
}

/* Mock controls */
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode(void) { return s_setMode_count; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMode(boolean value) { s_setMode_ret = value; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMode_mode(void) { return s_setMode_lastMode; }

uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime(void) { return s_setOnTime_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetArgHistoryCount_setOnTime(void) { return s_setOnTime_historyCount; }
Ifx_TimerValue IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(uint32 index) {
    return (index < MOCK_MAX_ELEMENTS) ? s_setOnTime_lastArray[index] : (Ifx_TimerValue)0u;
}
Ifx_TimerValue IfxGtm_Tom_PwmHl_Mock_GetArgHistory_setOnTime(uint32 callIdx, uint32 elemIdx) {
    if (callIdx < s_setOnTime_historyCount && elemIdx < MOCK_MAX_ELEMENTS) {
        return s_setOnTime_history[callIdx][elemIdx];
    }
    return (Ifx_TimerValue)0u;
}

uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }

uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void) { return s_init_count; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value) { s_init_ret = value; }

void IfxGtm_Tom_PwmHl_Mock_Reset(void) {
    s_setMode_count = 0u; s_setOnTime_count = 0u; s_initConfig_count = 0u; s_init_count = 0u;
    s_setMode_ret = FALSE; s_init_ret = FALSE;
    s_setMode_lastMode = 0u;
    {
        uint32 i, j;
        for (i = 0u; i < MOCK_MAX_ELEMENTS; i++) { s_setOnTime_lastArray[i] = (Ifx_TimerValue)0u; }
        for (i = 0u; i < MOCK_MAX_HISTORY; i++) { for (j = 0u; j < MOCK_MAX_ELEMENTS; j++) { s_setOnTime_history[i][j] = (Ifx_TimerValue)0u; } }
        s_setOnTime_historyCount = 0u;
    }
}
