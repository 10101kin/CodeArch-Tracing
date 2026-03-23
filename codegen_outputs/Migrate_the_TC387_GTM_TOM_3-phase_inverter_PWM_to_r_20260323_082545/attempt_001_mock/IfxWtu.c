#include "IfxWtu.h"

/* Call counters */
static uint32 s_disableCpuWdg_count = 0;
static uint32 s_disableSystemWdg_count = 0;
static uint32 s_getCpuWdgPass_count = 0;
static uint32 s_getSystemWdgPass_count = 0;

/* Argument capture */
static uint16 s_disableCpuWdg_lastPassword = 0u;
static uint16 s_disableSystemWdg_lastPassword = 0u;

/* Return values */
static uint16 s_getCpuWdgPass_ret = 0u;
static uint16 s_getSystemWdgPass_ret = 0u;

/* iLLD stubs */
void IfxWtu_disableCpuWatchdog(uint16 password) {
    (void)password;
    s_disableCpuWdg_count++;
    s_disableCpuWdg_lastPassword = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password) {
    (void)password;
    s_disableSystemWdg_count++;
    s_disableSystemWdg_lastPassword = password;
}

uint16 IfxWtu_getCpuWatchdogPassword(void) {
    s_getCpuWdgPass_count++;
    return s_getCpuWdgPass_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void) {
    s_getSystemWdgPass_count++;
    return s_getSystemWdgPass_ret;
}

/* Mock controls */
uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpuWdg_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void) { return s_disableSystemWdg_count; }
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuWdgPass_count; }
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void) { return s_getSystemWdgPass_count; }

uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpuWdg_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void) { return s_disableSystemWdg_lastPassword; }

void IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value) { s_getCpuWdgPass_ret = value; }
void IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value) { s_getSystemWdgPass_ret = value; }

void IfxWtu_Mock_Reset(void) {
    s_disableCpuWdg_count = 0u;
    s_disableSystemWdg_count = 0u;
    s_getCpuWdgPass_count = 0u;
    s_getSystemWdgPass_count = 0u;
    s_disableCpuWdg_lastPassword = 0u;
    s_disableSystemWdg_lastPassword = 0u;
    s_getCpuWdgPass_ret = 0u;
    s_getSystemWdgPass_ret = 0u;
}
