#include "IfxWtu.h"

static uint32 s_disableCpuWdt_count = 0u;
static uint32 s_disableSystemWdt_count = 0u;

static uint16 s_disableCpuWdt_lastPassword = 0u;
static uint16 s_disableSystemWdt_lastPassword = 0u;

static uint16 s_getCpuWdtPassword_ret = 0u;
static uint16 s_getSystemWdtPassword_ret = 0u;

void IfxWtu_disableCpuWatchdog(uint16 password)
{
    s_disableCpuWdt_count++;
    s_disableCpuWdt_lastPassword = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password)
{
    s_disableSystemWdt_count++;
    s_disableSystemWdt_lastPassword = password;
}

uint16 IfxWtu_getCpuWatchdogPassword(void)
{
    return s_getCpuWdtPassword_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void)
{
    return s_getSystemWdtPassword_ret;
}

uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpuWdt_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void) { return s_disableSystemWdt_count; }
uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpuWdt_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void) { return s_disableSystemWdt_lastPassword; }

void IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 pw) { s_getCpuWdtPassword_ret = pw; }
void IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 pw) { s_getSystemWdtPassword_ret = pw; }

void IfxWtu_Mock_Reset(void)
{
    s_disableCpuWdt_count = 0u;
    s_disableSystemWdt_count = 0u;
    s_disableCpuWdt_lastPassword = 0u;
    s_disableSystemWdt_lastPassword = 0u;
    s_getCpuWdtPassword_ret = 0u;
    s_getSystemWdtPassword_ret = 0u;
}
