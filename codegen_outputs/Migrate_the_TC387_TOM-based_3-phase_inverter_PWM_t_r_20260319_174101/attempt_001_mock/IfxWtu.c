#include "IfxWtu.h"

static uint32 s_getCpuPwdInline_count = 0u;
static uint32 s_getSecPwdInline_count = 0u;
static uint32 s_getSysPwdInline_count = 0u;
static uint32 s_getCpuPwd_count = 0u;
static uint32 s_getSecPwd_count = 0u;
static uint32 s_getSysPwd_count = 0u;
static uint32 s_disableCpuWdt_count = 0u;
static uint32 s_disableSecWdt_count = 0u;
static uint32 s_disableSysWdt_count = 0u;

static uint16 s_getCpuPwdInline_ret = 0u;
static uint16 s_getSecPwdInline_ret = 0u;
static uint16 s_getSysPwdInline_ret = 0u;
static uint16 s_getCpuPwd_ret = 0u;
static uint16 s_getSecPwd_ret = 0u;
static uint16 s_getSysPwd_ret = 0u;

static uint16 s_disableCpuWdt_lastPassword = 0u;
static uint16 s_disableSecWdt_lastPassword = 0u;
static uint16 s_disableSysWdt_lastPassword = 0u;

uint16 IfxWtu_getCpuWatchdogPasswordInline(Ifx_WTU_WDTCPU *watchdog)
{
    (void)watchdog;
    s_getCpuPwdInline_count++;
    return s_getCpuPwdInline_ret;
}

uint16 IfxWtu_getSecurityWatchdogPasswordInline(void)
{
    s_getSecPwdInline_count++;
    return s_getSecPwdInline_ret;
}

uint16 IfxWtu_getSystemWatchdogPasswordInline(void)
{
    s_getSysPwdInline_count++;
    return s_getSysPwdInline_ret;
}

uint16 IfxWtu_getCpuWatchdogPassword(void)
{
    s_getCpuPwd_count++;
    return s_getCpuPwd_ret;
}

uint16 IfxWtu_getSecurityWatchdogPassword(void)
{
    s_getSecPwd_count++;
    return s_getSecPwd_ret;
}

uint16 IfxWtu_getSystemWatchdogPassword(void)
{
    s_getSysPwd_count++;
    return s_getSysPwd_ret;
}

void IfxWtu_disableCpuWatchdog(uint16 password)
{
    s_disableCpuWdt_count++;
    s_disableCpuWdt_lastPassword = password;
}

void IfxWtu_disableSecurityWatchdog(uint16 password)
{
    s_disableSecWdt_count++;
    s_disableSecWdt_lastPassword = password;
}

void IfxWtu_disableSystemWatchdog(uint16 password)
{
    s_disableSysWdt_count++;
    s_disableSysWdt_lastPassword = password;
}

uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPasswordInline(void) { return s_getCpuPwdInline_count; }
uint32 IfxWtu_Mock_GetCallCount_getSecurityWatchdogPasswordInline(void) { return s_getSecPwdInline_count; }
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPasswordInline(void) { return s_getSysPwdInline_count; }
uint32 IfxWtu_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuPwd_count; }
uint32 IfxWtu_Mock_GetCallCount_getSecurityWatchdogPassword(void) { return s_getSecPwd_count; }
uint32 IfxWtu_Mock_GetCallCount_getSystemWatchdogPassword(void) { return s_getSysPwd_count; }
uint32 IfxWtu_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpuWdt_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSecurityWatchdog(void) { return s_disableSecWdt_count; }
uint32 IfxWtu_Mock_GetCallCount_disableSystemWatchdog(void) { return s_disableSysWdt_count; }

void   IfxWtu_Mock_SetReturn_getCpuWatchdogPasswordInline(uint16 ret) { s_getCpuPwdInline_ret = ret; }
void   IfxWtu_Mock_SetReturn_getSecurityWatchdogPasswordInline(uint16 ret) { s_getSecPwdInline_ret = ret; }
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPasswordInline(uint16 ret) { s_getSysPwdInline_ret = ret; }
void   IfxWtu_Mock_SetReturn_getCpuWatchdogPassword(uint16 ret) { s_getCpuPwd_ret = ret; }
void   IfxWtu_Mock_SetReturn_getSecurityWatchdogPassword(uint16 ret) { s_getSecPwd_ret = ret; }
void   IfxWtu_Mock_SetReturn_getSystemWatchdogPassword(uint16 ret) { s_getSysPwd_ret = ret; }

uint16 IfxWtu_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpuWdt_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSecurityWatchdog_password(void) { return s_disableSecWdt_lastPassword; }
uint16 IfxWtu_Mock_GetLastArg_disableSystemWatchdog_password(void) { return s_disableSysWdt_lastPassword; }

void IfxWtu_Mock_Reset(void)
{
    s_getCpuPwdInline_count = 0u;
    s_getSecPwdInline_count = 0u;
    s_getSysPwdInline_count = 0u;
    s_getCpuPwd_count = 0u;
    s_getSecPwd_count = 0u;
    s_getSysPwd_count = 0u;
    s_disableCpuWdt_count = 0u;
    s_disableSecWdt_count = 0u;
    s_disableSysWdt_count = 0u;

    s_getCpuPwdInline_ret = 0u;
    s_getSecPwdInline_ret = 0u;
    s_getSysPwdInline_ret = 0u;
    s_getCpuPwd_ret = 0u;
    s_getSecPwd_ret = 0u;
    s_getSysPwd_ret = 0u;

    s_disableCpuWdt_lastPassword = 0u;
    s_disableSecWdt_lastPassword = 0u;
    s_disableSysWdt_lastPassword = 0u;
}
