#include "IfxScuWdt.h"

static uint32 s_disableCpu_count = 0;
static uint16 s_disableCpu_lastPassword = 0u;

static uint32 s_disableSafety_count = 0;
static uint16 s_disableSafety_lastPassword = 0u;

static uint32 s_getCpuPwdInline_count = 0;
static uint32 s_getGSafeEndinitPwdInline_count = 0;
static uint32 s_getSafePwdInline_count = 0;

static uint32 s_getCpuPwd_count = 0;
static uint32 s_getGEndinitPwd_count = 0;
static uint32 s_getGSafeEndinitPwd_count = 0;
static uint32 s_getSafePwd_count = 0;

static uint16 s_ret_getCpuPwdInline = 0u;
static uint16 s_ret_getGSafeEndinitPwdInline = 0u;
static uint16 s_ret_getSafePwdInline = 0u;

static uint16 s_ret_getCpuPwd = 0u;
static uint16 s_ret_getGEndinitPwd = 0u;
static uint16 s_ret_getGSafeEndinitPwd = 0u;
static uint16 s_ret_getSafePwd = 0u;

uint16 IfxScuWdt_getGlobalSafetyEndinitPasswordInline(void) {
    s_getGSafeEndinitPwdInline_count++;
    return s_ret_getGSafeEndinitPwdInline;
}

uint16 IfxScuWdt_getCpuWatchdogPasswordInline(Ifx_SCU_WDTCPU *watchdog) {
    (void)watchdog;
    s_getCpuPwdInline_count++;
    return s_ret_getCpuPwdInline;
}

uint16 IfxScuWdt_getSafetyWatchdogPasswordInline(void) {
    s_getSafePwdInline_count++;
    return s_ret_getSafePwdInline;
}

void IfxScuWdt_disableCpuWatchdog(uint16 password) {
    s_disableCpu_count++;
    s_disableCpu_lastPassword = password;
}

void IfxScuWdt_disableSafetyWatchdog(uint16 password) {
    s_disableSafety_count++;
    s_disableSafety_lastPassword = password;
}

uint16 IfxScuWdt_getCpuWatchdogPassword(void) {
    s_getCpuPwd_count++;
    return s_ret_getCpuPwd;
}

uint16 IfxScuWdt_getGlobalEndinitPassword(void) {
    s_getGEndinitPwd_count++;
    return s_ret_getGEndinitPwd;
}

uint16 IfxScuWdt_getGlobalSafetyEndinitPassword(void) {
    s_getGSafeEndinitPwd_count++;
    return s_ret_getGSafeEndinitPwd;
}

uint16 IfxScuWdt_getSafetyWatchdogPassword(void) {
    s_getSafePwd_count++;
    return s_ret_getSafePwd;
}

/* Mock controls */
uint32 IfxScuWdt_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpu_count; }
uint16 IfxScuWdt_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpu_lastPassword; }

uint32 IfxScuWdt_Mock_GetCallCount_disableSafetyWatchdog(void) { return s_disableSafety_count; }
uint16 IfxScuWdt_Mock_GetLastArg_disableSafetyWatchdog_password(void) { return s_disableSafety_lastPassword; }

void IfxScuWdt_Mock_SetReturn_getCpuWatchdogPassword(uint16 ret) { s_ret_getCpuPwd = ret; }
void IfxScuWdt_Mock_SetReturn_getGlobalEndinitPassword(uint16 ret) { s_ret_getGEndinitPwd = ret; }
void IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPassword(uint16 ret) { s_ret_getGSafeEndinitPwd = ret; }
void IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPassword(uint16 ret) { s_ret_getSafePwd = ret; }
void IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPasswordInline(uint16 ret) { s_ret_getGSafeEndinitPwdInline = ret; }
void IfxScuWdt_Mock_SetReturn_getCpuWatchdogPasswordInline(uint16 ret) { s_ret_getCpuPwdInline = ret; }
void IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPasswordInline(uint16 ret) { s_ret_getSafePwdInline = ret; }

uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPasswordInline(void) { return s_getCpuPwdInline_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPasswordInline(void) { return s_getGSafeEndinitPwdInline_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPasswordInline(void) { return s_getSafePwdInline_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuPwd_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalEndinitPassword(void) { return s_getGEndinitPwd_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPassword(void) { return s_getGSafeEndinitPwd_count; }
uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPassword(void) { return s_getSafePwd_count; }

void IfxScuWdt_Mock_Reset(void) {
    s_disableCpu_count = 0; s_disableCpu_lastPassword = 0u;
    s_disableSafety_count = 0; s_disableSafety_lastPassword = 0u;
    s_getCpuPwdInline_count = 0; s_getGSafeEndinitPwdInline_count = 0; s_getSafePwdInline_count = 0;
    s_getCpuPwd_count = 0; s_getGEndinitPwd_count = 0; s_getGSafeEndinitPwd_count = 0; s_getSafePwd_count = 0;
    s_ret_getCpuPwdInline = 0u; s_ret_getGSafeEndinitPwdInline = 0u; s_ret_getSafePwdInline = 0u;
    s_ret_getCpuPwd = 0u; s_ret_getGEndinitPwd = 0u; s_ret_getGSafeEndinitPwd = 0u; s_ret_getSafePwd = 0u;
}
