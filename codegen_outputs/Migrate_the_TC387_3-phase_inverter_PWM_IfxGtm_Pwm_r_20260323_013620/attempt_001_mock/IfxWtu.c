#include "IfxWtu.h"

static uint32 s_disableCpuWdg_count = 0;
static uint32 s_disableSysWdg_count = 0;
static uint32 s_getCpuPwd_count = 0;
static uint32 s_getSysPwd_count = 0;

static uint16 s_getCpuPwd_ret = 0u;
static uint16 s_getSysPwd_ret = 0u;

static uint16 s_disableCpuWdg_lastPassword = 0u;
static uint16 s_disableSysWdg_lastPassword = 0u;

void IfxWtu_disableCpuWatchdog(uint16 password)
{
    s_disableCpuWdg_count++;
    s_disableCpuWdg_lastPassword = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password)
{
    s_disableSysWdg_count++;
    s_disableSysWdg_lastPassword = password;
}

uint16 IfxWtu_getCpuWatchdogPassword(void)
{
    s_getCpuPwd_count++;
    return s_getCpuPwd_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void)
{
    s_getSysPwd_count++;
    return s_getSysPwd_ret;
}

uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpuWdg_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void) { return s_disableSysWdg_count; }
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuPwd_count; }
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void) { return s_getSysPwd_count; }

void IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value) { s_getCpuPwd_ret = value; }
void IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value) { s_getSysPwd_ret = value; }

uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpuWdg_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void) { return s_disableSysWdg_lastPassword; }

void IfxWtu_Mock_Reset(void)
{
    s_disableCpuWdg_count = 0u;
    s_disableSysWdg_count = 0u;
    s_getCpuPwd_count = 0u;
    s_getSysPwd_count = 0u;
    s_getCpuPwd_ret = 0u;
    s_getSysPwd_ret = 0u;
    s_disableCpuWdg_lastPassword = 0u;
    s_disableSysWdg_lastPassword = 0u;
}
