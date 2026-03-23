#include "IfxPort.h"

static uint32 s_setPinState_count = 0;
static uint32 s_setPinModeOutput_count = 0;
static uint32 s_getPinState_count = 0;
static uint32 s_setPinHigh_count = 0;
static uint32 s_setPinLow_count = 0;
static uint32 s_setPinModeInput_count = 0;
static uint32 s_togglePin_count = 0;
static uint32 s_setPinMode_count = 0;
static uint32 s_setPinModeLVDS_count = 0;
static uint32 s_setPinModex_count = 0;

static boolean s_getPinState_ret = 0u;

static uint32 s_last_setPinState_pinIndex = 0;
static uint32 s_last_setPinState_action = 0;
static uint32 s_last_setPinModeOutput_pinIndex = 0;
static uint32 s_last_setPinModeOutput_mode = 0;
static uint32 s_last_setPinModeOutput_index = 0;
static uint32 s_last_setPinHigh_pinIndex = 0;
static uint32 s_last_setPinLow_pinIndex = 0;
static uint32 s_last_setPinModeInput_pinIndex = 0;
static uint32 s_last_setPinModeInput_mode = 0;
static uint32 s_last_togglePin_pinIndex = 0;
static uint32 s_last_setPinMode_pinIndex = 0;
static uint32 s_last_setPinMode_mode = 0;
static uint32 s_last_setPinModeLVDS_pinIndex = 0;
static uint32 s_last_setPinModeLVDS_pinMode = 0;
static uint32 s_last_setPinModex_pinIndex = 0;
static uint32 s_last_setPinModex_modex = 0;

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action) {
    (void)port;
    s_setPinState_count++;
    s_last_setPinState_pinIndex = (uint32)pinIndex;
    s_last_setPinState_action = (uint32)action;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) {
    (void)port;
    s_setPinModeOutput_count++;
    s_last_setPinModeOutput_pinIndex = (uint32)pinIndex;
    s_last_setPinModeOutput_mode = (uint32)mode;
    s_last_setPinModeOutput_index = (uint32)index;
}

boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    (void)pinIndex;
    s_getPinState_count++;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_setPinHigh_count++;
    s_last_setPinHigh_pinIndex = (uint32)pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_setPinLow_count++;
    s_last_setPinLow_pinIndex = (uint32)pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) {
    (void)port;
    s_setPinModeInput_count++;
    s_last_setPinModeInput_pinIndex = (uint32)pinIndex;
    s_last_setPinModeInput_mode = (uint32)mode;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) {
    (void)port;
    s_togglePin_count++;
    s_last_togglePin_pinIndex = (uint32)pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) {
    (void)port;
    s_setPinMode_count++;
    s_last_setPinMode_pinIndex = (uint32)pinIndex;
    s_last_setPinMode_mode = (uint32)mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds) {
    (void)port;
    (void)lvds;
    s_setPinModeLVDS_count++;
    s_last_setPinModeLVDS_pinIndex = (uint32)pinIndex;
    s_last_setPinModeLVDS_pinMode = (uint32)pinMode;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex) {
    (void)port;
    s_setPinModex_count++;
    s_last_setPinModex_pinIndex = (uint32)pinIndex;
    s_last_setPinModex_modex = (uint32)modex;
}

uint32 IfxPort_Mock_GetCallCount_setPinState(void) { return s_setPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void) { return s_setPinModeOutput_count; }
uint32 IfxPort_Mock_GetCallCount_getPinState(void) { return s_getPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void) { return s_setPinHigh_count; }
uint32 IfxPort_Mock_GetCallCount_setPinLow(void) { return s_setPinLow_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void) { return s_setPinModeInput_count; }
uint32 IfxPort_Mock_GetCallCount_togglePin(void) { return s_togglePin_count; }
uint32 IfxPort_Mock_GetCallCount_setPinMode(void) { return s_setPinMode_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void) { return s_setPinModeLVDS_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModex(void) { return s_setPinModex_count; }

void IfxPort_Mock_SetReturn_getPinState(boolean value) { s_getPinState_ret = value; }
boolean IfxPort_Mock_GetReturn_getPinState(void) { return s_getPinState_ret; }

uint32 IfxPort_Mock_GetLastArg_setPinState_pinIndex(void) { return s_last_setPinState_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_last_setPinState_action; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void) { return s_last_setPinModeOutput_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_last_setPinModeOutput_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_index(void) { return s_last_setPinModeOutput_index; }
uint32 IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void) { return s_last_setPinHigh_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void) { return s_last_setPinLow_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void) { return s_last_setPinModeInput_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_last_setPinModeInput_mode; }
uint32 IfxPort_Mock_GetLastArg_togglePin_pinIndex(void) { return s_last_togglePin_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void) { return s_last_setPinMode_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_last_setPinMode_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void) { return s_last_setPinModeLVDS_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void) { return s_last_setPinModeLVDS_pinMode; }
uint32 IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void) { return s_last_setPinModex_pinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModex_modex(void) { return s_last_setPinModex_modex; }

void IfxPort_Mock_Reset(void) {
    s_setPinState_count = 0;
    s_setPinModeOutput_count = 0;
    s_getPinState_count = 0;
    s_setPinHigh_count = 0;
    s_setPinLow_count = 0;
    s_setPinModeInput_count = 0;
    s_togglePin_count = 0;
    s_setPinMode_count = 0;
    s_setPinModeLVDS_count = 0;
    s_setPinModex_count = 0;

    s_getPinState_ret = 0u;

    s_last_setPinState_pinIndex = 0;
    s_last_setPinState_action = 0;
    s_last_setPinModeOutput_pinIndex = 0;
    s_last_setPinModeOutput_mode = 0;
    s_last_setPinModeOutput_index = 0;
    s_last_setPinHigh_pinIndex = 0;
    s_last_setPinLow_pinIndex = 0;
    s_last_setPinModeInput_pinIndex = 0;
    s_last_setPinModeInput_mode = 0;
    s_last_togglePin_pinIndex = 0;
    s_last_setPinMode_pinIndex = 0;
    s_last_setPinMode_mode = 0;
    s_last_setPinModeLVDS_pinIndex = 0;
    s_last_setPinModeLVDS_pinMode = 0;
    s_last_setPinModex_pinIndex = 0;
    s_last_setPinModex_modex = 0;
}
