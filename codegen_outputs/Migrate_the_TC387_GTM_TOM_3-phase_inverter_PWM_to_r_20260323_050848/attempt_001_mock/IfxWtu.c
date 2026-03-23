#include "IfxWtu.h"

static uint32 s_disableCpu_count = 0;
static uint32 s_disableSystem_count = 0;

static uint16 s_getCpuPassword_ret = 0u;
static uint16 s_getSystemPassword_ret = 0u;

static uint16 s_last_disableCpu_password = 0u;
static uint16 s_last_disableSystem_password = 0u;

void IfxWtu_disableCpuWatchdog(uint16 password) {
    s_disableCpu_count++;
    s_last_disableCpu_password = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password) {
    s_disableSystem_count++;
    s_last_disableSystem_password = password;
}

uint16 IfxWtu_getCpuWatchdogPassword(void) {
    return s_getCpuPassword_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void) {
    return s_getSystemPassword_ret;
}

uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpu_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void) { return s_disableSystem_count; }

void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 value) { s_getCpuPassword_ret = value; }
uint16 IfxWtu_Mock_GetReturn_getCpuWatchdogPassword(void) { return s_getCpuPassword_ret; }
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 value) { s_getSystemPassword_ret = value; }
uint16 IfxWtu_Mock_GetReturn_getSystemWatchdogPassword(void) { return s_getSystemPassword_ret; }

uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_last_disableCpu_password; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void) { return s_last_disableSystem_password; }

void IfxWtu_Mock_Reset(void) {
    s_disableCpu_count = 0u;
    s_disableSystem_count = 0u;
    s_getCpuPassword_ret = 0u;
    s_getSystemPassword_ret = 0u;
    s_last_disableCpu_password = 0u;
    s_last_disableSystem_password = 0u;
}
