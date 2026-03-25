#include "IfxPort.h"

/* Call counters */
static uint32 s_getPinState_count = 0;
static uint32 s_setPinHigh_count = 0;
static uint32 s_setPinLow_count = 0;
static uint32 s_setPinModeInput_count = 0;
static uint32 s_setPinModeOutput_count = 0;
static uint32 s_setPinState_count = 0;
static uint32 s_togglePin_count = 0;
static uint32 s_setPinMode_count = 0;
static uint32 s_setPinModeLVDS_count = 0;

/* Return values */
static boolean s_getPinState_ret = 0u;

/* Captured args */
static uint8  s_last_pinIndex = 0u;
static uint32 s_last_setPinModeInput_mode = 0u;
static uint32 s_last_setPinModeOutput_mode = 0u;
static uint32 s_last_setPinModeOutput_index = 0u;
static uint32 s_last_setPinState_action = 0u;
static uint32 s_last_setPinMode_mode = 0u;
static uint32 s_last_setPinModeLVDS_mode = 0u;

boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_getPinState_count++;
    s_last_pinIndex = pinIndex;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_setPinHigh_count++;
    s_last_pinIndex = pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_setPinLow_count++;
    s_last_pinIndex = pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) {
    (void)port;
    s_setPinModeInput_count++;
    s_last_pinIndex = pinIndex;
    s_last_setPinModeInput_mode = (uint32)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) {
    (void)port;
    s_setPinModeOutput_count++;
    s_last_pinIndex = pinIndex;
    s_last_setPinModeOutput_mode = (uint32)mode;
    s_last_setPinModeOutput_index = (uint32)index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action) {
    (void)port;
    s_setPinState_count++;
    s_last_pinIndex = pinIndex;
    s_last_setPinState_action = (uint32)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_togglePin_count++;
    s_last_pinIndex = pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) {
    (void)port;
    s_setPinMode_count++;
    s_last_pinIndex = pinIndex;
    s_last_setPinMode_mode = (uint32)mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds) {
    (void)port;
    (void)lvds;
    s_setPinModeLVDS_count++;
    s_last_pinIndex = pinIndex;
    s_last_setPinModeLVDS_mode = (uint32)pinMode;
}

/* Mock control functions */
uint32 IfxPort_Mock_GetCallCount_getPinState(void) { return s_getPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void) { return s_setPinHigh_count; }
uint32 IfxPort_Mock_GetCallCount_setPinLow(void) { return s_setPinLow_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void) { return s_setPinModeInput_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void) { return s_setPinModeOutput_count; }
uint32 IfxPort_Mock_GetCallCount_setPinState(void) { return s_setPinState_count; }
uint32 IfxPort_Mock_GetCallCount_togglePin(void) { return s_togglePin_count; }
uint32 IfxPort_Mock_GetCallCount_setPinMode(void) { return s_setPinMode_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void) { return s_setPinModeLVDS_count; }

void IfxPort_Mock_SetReturn_getPinState(boolean value) { s_getPinState_ret = value; }

uint32 IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_last_setPinModeInput_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_last_setPinModeOutput_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_index(void) { return s_last_setPinModeOutput_index; }
uint32 IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_last_setPinState_action; }
uint32 IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_last_setPinMode_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_mode(void) { return s_last_setPinModeLVDS_mode; }
uint8  IfxPort_Mock_GetLastArg_pinIndex(void) { return s_last_pinIndex; }

void IfxPort_Mock_Reset(void) {
    s_getPinState_count = 0u;
    s_setPinHigh_count = 0u;
    s_setPinLow_count = 0u;
    s_setPinModeInput_count = 0u;
    s_setPinModeOutput_count = 0u;
    s_setPinState_count = 0u;
    s_togglePin_count = 0u;
    s_setPinMode_count = 0u;
    s_setPinModeLVDS_count = 0u;
    s_getPinState_ret = 0u;
    s_last_pinIndex = 0u;
    s_last_setPinModeInput_mode = 0u;
    s_last_setPinModeOutput_mode = 0u;
    s_last_setPinModeOutput_index = 0u;
    s_last_setPinState_action = 0u;
    s_last_setPinMode_mode = 0u;
    s_last_setPinModeLVDS_mode = 0u;
}
