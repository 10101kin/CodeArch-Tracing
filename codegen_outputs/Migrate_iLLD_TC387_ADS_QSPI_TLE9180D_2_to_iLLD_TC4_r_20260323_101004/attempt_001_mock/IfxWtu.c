#include "IfxWtu.h"

/* Call counters */
static uint32 s_disableCpuWatchdog_count = 0u;
static uint32 s_disableSystemWatchdog_count = 0u;
static uint32 s_getCpuWatchdogPassword_count = 0u;
static uint32 s_getSystemWatchdogPassword_count = 0u;

/* Return values */
static uint16 s_getCpuWatchdogPassword_ret = 0u;
static uint16 s_getSystemWatchdogPassword_ret = 0u;

/* Captured arguments */
static uint16 s_disableCpuWatchdog_lastPassword = 0u;
static uint16 s_disableSystemWatchdog_lastPassword = 0u;

void IfxWtu_disableCpuWatchdog(uint16 password)
{
    s_disableCpuWatchdog_count++;
    s_disableCpuWatchdog_lastPassword = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password)
{
    s_disableSystemWatchdog_count++;
    s_disableSystemWatchdog_lastPassword = password;
}

uint16 IfxWtu_getCpuWatchdogPassword(void)
{
    s_getCpuWatchdogPassword_count++;
    return s_getCpuWatchdogPassword_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void)
{
    s_getSystemWatchdogPassword_count++;
    return s_getSystemWatchdogPassword_ret;
}

uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void)     { return s_disableCpuWatchdog_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void)  { return s_disableSystemWatchdog_count; }
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuWatchdogPassword_count; }
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void) { return s_getSystemWatchdogPassword_count; }

void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value)      { s_getCpuWatchdogPassword_ret = value; }
uint16 IfxWtu_Mock_GetReturn_getCpuWatchdogPassword(void)              { return s_getCpuWatchdogPassword_ret; }
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value)   { s_getSystemWatchdogPassword_ret = value; }
uint16 IfxWtu_Mock_GetReturn_getSystemWatchdogPassword(void)           { return s_getSystemWatchdogPassword_ret; }

uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void)        { return s_disableCpuWatchdog_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void)     { return s_disableSystemWatchdog_lastPassword; }

void IfxWtu_Mock_Reset(void)
{
    s_disableCpuWatchdog_count = 0u;
    s_disableSystemWatchdog_count = 0u;
    s_getCpuWatchdogPassword_count = 0u;
    s_getSystemWatchdogPassword_count = 0u;

    s_getCpuWatchdogPassword_ret = 0u;
    s_getSystemWatchdogPassword_ret = 0u;

    s_disableCpuWatchdog_lastPassword = 0u;
    s_disableSystemWatchdog_lastPassword = 0u;
}
