#include "IfxScuWdt.h"

static uint32 s_getGlobalSafetyEndinitPasswordInline_count = 0;
static uint16 s_getGlobalSafetyEndinitPasswordInline_ret = 0u;

static uint32 s_getCpuWatchdogPasswordInline_count = 0;
static uint16 s_getCpuWatchdogPasswordInline_ret = 0u;

static uint32 s_getSafetyWatchdogPasswordInline_count = 0;
static uint16 s_getSafetyWatchdogPasswordInline_ret = 0u;

static uint32 s_disableCpuWatchdog_count = 0;
static uint16 s_disableCpuWatchdog_lastPassword = 0u;

static uint32 s_disableSafetyWatchdog_count = 0;
static uint16 s_disableSafetyWatchdog_lastPassword = 0u;

static uint32 s_getCpuWatchdogPassword_count = 0;
static uint16 s_getCpuWatchdogPassword_ret = 0u;

static uint32 s_getGlobalEndinitPassword_count = 0;
static uint16 s_getGlobalEndinitPassword_ret = 0u;

static uint32 s_getGlobalSafetyEndinitPassword_count = 0;
static uint16 s_getGlobalSafetyEndinitPassword_ret = 0u;

static uint32 s_getSafetyWatchdogPassword_count = 0;
static uint16 s_getSafetyWatchdogPassword_ret = 0u;

uint16 IfxScuWdt_getGlobalSafetyEndinitPasswordInline(void) {
    s_getGlobalSafetyEndinitPasswordInline_count++;
    return s_getGlobalSafetyEndinitPasswordInline_ret;
}

uint16 IfxScuWdt_getCpuWatchdogPasswordInline(Ifx_SCU_WDTCPU *watchdog) {
    (void)watchdog;
    s_getCpuWatchdogPasswordInline_count++;
    return s_getCpuWatchdogPasswordInline_ret;
}

uint16 IfxScuWdt_getSafetyWatchdogPasswordInline(void) {
    s_getSafetyWatchdogPasswordInline_count++;
    return s_getSafetyWatchdogPasswordInline_ret;
}

void IfxScuWdt_disableCpuWatchdog(uint16 password) {
    s_disableCpuWatchdog_count++;
    s_disableCpuWatchdog_lastPassword = password;
}

void IfxScuWdt_disableSafetyWatchdog(uint16 password) {
    s_disableSafetyWatchdog_count++;
    s_disableSafetyWatchdog_lastPassword = password;
}

uint16 IfxScuWdt_getCpuWatchdogPassword(void) {
    s_getCpuWatchdogPassword_count++;
    return s_getCpuWatchdogPassword_ret;
}

uint16 IfxScuWdt_getGlobalEndinitPassword(void) {
    s_getGlobalEndinitPassword_count++;
    return s_getGlobalEndinitPassword_ret;
}

uint16 IfxScuWdt_getGlobalSafetyEndinitPassword(void) {
    s_getGlobalSafetyEndinitPassword_count++;
    return s_getGlobalSafetyEndinitPassword_ret;
}

uint16 IfxScuWdt_getSafetyWatchdogPassword(void) {
    s_getSafetyWatchdogPassword_count++;
    return s_getSafetyWatchdogPassword_ret;
}

uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPasswordInline(void) { return s_getGlobalSafetyEndinitPasswordInline_count; }
void   IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPasswordInline(uint16 value) { s_getGlobalSafetyEndinitPasswordInline_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPasswordInline(void) { return s_getCpuWatchdogPasswordInline_count; }
void   IfxScuWdt_Mock_SetReturn_getCpuWatchdogPasswordInline(uint16 value) { s_getCpuWatchdogPasswordInline_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPasswordInline(void) { return s_getSafetyWatchdogPasswordInline_count; }
void   IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPasswordInline(uint16 value) { s_getSafetyWatchdogPasswordInline_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_disableCpuWatchdog(void) { return s_disableCpuWatchdog_count; }
uint16 IfxScuWdt_Mock_GetLastArg_disableCpuWatchdog_password(void) { return s_disableCpuWatchdog_lastPassword; }

uint32 IfxScuWdt_Mock_GetCallCount_disableSafetyWatchdog(void) { return s_disableSafetyWatchdog_count; }
uint16 IfxScuWdt_Mock_GetLastArg_disableSafetyWatchdog_password(void) { return s_disableSafetyWatchdog_lastPassword; }

uint32 IfxScuWdt_Mock_GetCallCount_getCpuWatchdogPassword(void) { return s_getCpuWatchdogPassword_count; }
void   IfxScuWdt_Mock_SetReturn_getCpuWatchdogPassword(uint16 value) { s_getCpuWatchdogPassword_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_getGlobalEndinitPassword(void) { return s_getGlobalEndinitPassword_count; }
void   IfxScuWdt_Mock_SetReturn_getGlobalEndinitPassword(uint16 value) { s_getGlobalEndinitPassword_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_getGlobalSafetyEndinitPassword(void) { return s_getGlobalSafetyEndinitPassword_count; }
void   IfxScuWdt_Mock_SetReturn_getGlobalSafetyEndinitPassword(uint16 value) { s_getGlobalSafetyEndinitPassword_ret = value; }

uint32 IfxScuWdt_Mock_GetCallCount_getSafetyWatchdogPassword(void) { return s_getSafetyWatchdogPassword_count; }
void   IfxScuWdt_Mock_SetReturn_getSafetyWatchdogPassword(uint16 value) { s_getSafetyWatchdogPassword_ret = value; }

void IfxScuWdt_Mock_Reset(void) {
    s_getGlobalSafetyEndinitPasswordInline_count = 0; s_getGlobalSafetyEndinitPasswordInline_ret = 0u;
    s_getCpuWatchdogPasswordInline_count = 0; s_getCpuWatchdogPasswordInline_ret = 0u;
    s_getSafetyWatchdogPasswordInline_count = 0; s_getSafetyWatchdogPasswordInline_ret = 0u;
    s_disableCpuWatchdog_count = 0; s_disableCpuWatchdog_lastPassword = 0u;
    s_disableSafetyWatchdog_count = 0; s_disableSafetyWatchdog_lastPassword = 0u;
    s_getCpuWatchdogPassword_count = 0; s_getCpuWatchdogPassword_ret = 0u;
    s_getGlobalEndinitPassword_count = 0; s_getGlobalEndinitPassword_ret = 0u;
    s_getGlobalSafetyEndinitPassword_count = 0; s_getGlobalSafetyEndinitPassword_ret = 0u;
    s_getSafetyWatchdogPassword_count = 0; s_getSafetyWatchdogPassword_ret = 0u;
}
