#include "IfxGtm_Tom_PwmHl.h"

/* Call counters */
static uint32 s_setMode_count = 0;
static uint32 s_setMinPulse_count = 0;
static uint32 s_setDeadtime_count = 0;
static uint32 s_setOnTime_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_init_count = 0;

/* Returns */
static boolean s_setMode_ret = 0;
static boolean s_setMinPulse_ret = 0;
static boolean s_setDeadtime_ret = 0;
static boolean s_init_ret = 0;

/* Last-arg captures */
static uint32  s_setMode_lastMode = 0;
static float32 s_setMinPulse_last = 0.0f;
static float32 s_setDeadtime_last = 0.0f;

/* Array capture for tOn */
#define PWMHL_MOCK_MAX_ELEMENTS 8u
#define PWMHL_MOCK_MAX_HISTORY  32u
static Ifx_TimerValue s_setOnTime_lastArray[PWMHL_MOCK_MAX_ELEMENTS] = {0};
static Ifx_TimerValue s_setOnTime_history[PWMHL_MOCK_MAX_HISTORY][PWMHL_MOCK_MAX_ELEMENTS] = {{0}};
static uint32         s_setOnTime_historyCount = 0;

boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode)
{
    (void)driver;
    s_setMode_count++;
    s_setMode_lastMode = (uint32)mode;
    return s_setMode_ret;
}

boolean IfxGtm_Tom_PwmHl_setMinPulse(IfxGtm_Tom_PwmHl *driver, float32 minPulse)
{
    (void)driver;
    s_setMinPulse_count++;
    s_setMinPulse_last = minPulse;
    return s_setMinPulse_ret;
}

boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime)
{
    (void)driver;
    s_setDeadtime_count++;
    s_setDeadtime_last = deadtime;
    return s_setDeadtime_ret;
}

void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    (void)driver;
    s_setOnTime_count++;
    if (tOn != NULL_PTR)
    {
        uint32 i;
        for (i = 0; i < PWMHL_MOCK_MAX_ELEMENTS; i++)
        {
            s_setOnTime_lastArray[i] = tOn[i];
        }
        if (s_setOnTime_historyCount < PWMHL_MOCK_MAX_HISTORY)
        {
            for (i = 0; i < PWMHL_MOCK_MAX_ELEMENTS; i++)
            {
                s_setOnTime_history[s_setOnTime_historyCount][i] = tOn[i];
            }
            s_setOnTime_historyCount++;
        }
    }
}

void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config)
{
    (void)config;
    s_initConfig_count++;
}

boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config)
{
    (void)driver;
    (void)config;
    s_init_count++;
    return s_init_ret;
}

/* Mock controls */
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMode(void)      { return s_setMode_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setMinPulse(void)  { return s_setMinPulse_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setDeadtime(void)  { return s_setDeadtime_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_setOnTime(void)    { return s_setOnTime_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_initConfig(void)   { return s_initConfig_count; }
uint32 IfxGtm_Tom_PwmHl_Mock_GetCallCount_init(void)         { return s_init_count; }

void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMode(boolean value)     { s_setMode_ret = value; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setMinPulse(boolean value) { s_setMinPulse_ret = value; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_setDeadtime(boolean value) { s_setDeadtime_ret = value; }
void   IfxGtm_Tom_PwmHl_Mock_SetReturn_init(boolean value)        { s_init_ret = value; }

uint32  IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMode_mode(void)       { return s_setMode_lastMode; }
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setMinPulse_value(void)  { return s_setMinPulse_last; }
float32 IfxGtm_Tom_PwmHl_Mock_GetLastArg_setDeadtime_value(void)  { return s_setDeadtime_last; }

float32 IfxGtm_Tom_PwmHl_Mock_GetLastArrayArg_setOnTime(uint32 index)
{
    if (index < PWMHL_MOCK_MAX_ELEMENTS)
    {
        return (float32)s_setOnTime_lastArray[index];
    }
    return 0.0f;
}

float32 IfxGtm_Tom_PwmHl_Mock_GetArgHistory_setOnTime(uint32 callIdx, uint32 elemIdx)
{
    if ((callIdx < s_setOnTime_historyCount) && (elemIdx < PWMHL_MOCK_MAX_ELEMENTS))
    {
        return (float32)s_setOnTime_history[callIdx][elemIdx];
    }
    return 0.0f;
}

uint32 IfxGtm_Tom_PwmHl_Mock_GetArgHistoryCount_setOnTime(void)
{
    return s_setOnTime_historyCount;
}

void IfxGtm_Tom_PwmHl_Mock_Reset(void)
{
    s_setMode_count = 0; s_setMinPulse_count = 0; s_setDeadtime_count = 0; s_setOnTime_count = 0;
    s_initConfig_count = 0; s_init_count = 0;
    s_setMode_ret = 0; s_setMinPulse_ret = 0; s_setDeadtime_ret = 0; s_init_ret = 0;
    s_setMode_lastMode = 0; s_setMinPulse_last = 0.0f; s_setDeadtime_last = 0.0f;
    {
        uint32 i, j;
        for (i = 0; i < PWMHL_MOCK_MAX_ELEMENTS; i++) { s_setOnTime_lastArray[i] = 0; }
        for (j = 0; j < PWMHL_MOCK_MAX_HISTORY; j++)
        {
            for (i = 0; i < PWMHL_MOCK_MAX_ELEMENTS; i++) { s_setOnTime_history[j][i] = 0; }
        }
        s_setOnTime_historyCount = 0;
    }
}
