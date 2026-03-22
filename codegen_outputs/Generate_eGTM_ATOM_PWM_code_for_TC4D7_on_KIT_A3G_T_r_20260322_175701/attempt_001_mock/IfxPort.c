#include "IfxPort.h"

static uint32 s_getPinState_count = 0;
static uint32 s_setPinHigh_count = 0;
static uint32 s_setPinLow_count = 0;
static uint32 s_setPinModeInput_count = 0;
static uint32 s_setPinModeOutput_count = 0;
static uint32 s_setPinState_count = 0;
static uint32 s_togglePin_count = 0;
static uint32 s_setPinMode_count = 0;
static uint32 s_setPinModeLVDS_count = 0;
static uint32 s_setPinModex_count = 0;

static boolean s_getPinState_ret = 0u;

static uint8  s_getPinState_lastPinIndex = 0u;
static uint8  s_setPinHigh_lastPinIndex = 0u;
static uint8  s_setPinLow_lastPinIndex = 0u;
static uint8  s_setPinModeInput_lastPinIndex = 0u;
static uint32 s_setPinModeInput_lastMode = 0u;
static uint8  s_setPinModeOutput_lastPinIndex = 0u;
static uint32 s_setPinModeOutput_lastMode = 0u;
static uint32 s_setPinModeOutput_lastIndex = 0u;
static uint8  s_setPinState_lastPinIndex = 0u;
static uint32 s_setPinState_lastAction = 0u;
static uint8  s_togglePin_lastPinIndex = 0u;
static uint8  s_setPinMode_lastPinIndex = 0u;
static uint32 s_setPinMode_lastMode = 0u;
static uint8  s_setPinModeLVDS_lastPinIndex = 0u;
static uint32 s_setPinModeLVDS_lastPinMode = 0u;
static uint8  s_setPinModex_lastPinIndex = 0u;
static uint32 s_setPinModex_lastModex = 0u;

boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_getPinState_count++;
    s_getPinState_lastPinIndex = pinIndex;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_setPinHigh_count++;
    s_setPinHigh_lastPinIndex = pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_setPinLow_count++;
    s_setPinLow_lastPinIndex = pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    (void)port;
    s_setPinModeInput_count++;
    s_setPinModeInput_lastPinIndex = pinIndex;
    s_setPinModeInput_lastMode = (uint32)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port;
    s_setPinModeOutput_count++;
    s_setPinModeOutput_lastPinIndex = pinIndex;
    s_setPinModeOutput_lastMode = (uint32)mode;
    s_setPinModeOutput_lastIndex = (uint32)index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    (void)port;
    s_setPinState_count++;
    s_setPinState_lastPinIndex = pinIndex;
    s_setPinState_lastAction = (uint32)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_togglePin_count++;
    s_togglePin_lastPinIndex = pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port;
    s_setPinMode_count++;
    s_setPinMode_lastPinIndex = pinIndex;
    s_setPinMode_lastMode = (uint32)mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds)
{
    (void)port;
    (void)lvds;
    s_setPinModeLVDS_count++;
    s_setPinModeLVDS_lastPinIndex = pinIndex;
    s_setPinModeLVDS_lastPinMode = (uint32)pinMode;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex)
{
    (void)port;
    s_setPinModex_count++;
    s_setPinModex_lastPinIndex = pinIndex;
    s_setPinModex_lastModex = (uint32)modex;
}

uint32 IfxPort_Mock_GetCallCount_getPinState(void) { return s_getPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void) { return s_setPinHigh_count; }
uint32 IfxPort_Mock_GetCallCount_setPinLow(void) { return s_setPinLow_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void) { return s_setPinModeInput_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void) { return s_setPinModeOutput_count; }
uint32 IfxPort_Mock_GetCallCount_setPinState(void) { return s_setPinState_count; }
uint32 IfxPort_Mock_GetCallCount_togglePin(void) { return s_togglePin_count; }
uint32 IfxPort_Mock_GetCallCount_setPinMode(void) { return s_setPinMode_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void) { return s_setPinModeLVDS_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModex(void) { return s_setPinModex_count; }

void IfxPort_Mock_SetReturn_getPinState(boolean ret) { s_getPinState_ret = ret; }

uint8  IfxPort_Mock_GetLastArg_getPinState_pinIndex(void) { return s_getPinState_lastPinIndex; }
uint8  IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void) { return s_setPinHigh_lastPinIndex; }
uint8  IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void) { return s_setPinLow_lastPinIndex; }
uint8  IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void) { return s_setPinModeInput_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_setPinModeInput_lastMode; }
uint8  IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void) { return s_setPinModeOutput_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_setPinModeOutput_lastMode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_index(void) { return s_setPinModeOutput_lastIndex; }
uint8  IfxPort_Mock_GetLastArg_setPinState_pinIndex(void) { return s_setPinState_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_setPinState_lastAction; }
uint8  IfxPort_Mock_GetLastArg_togglePin_pinIndex(void) { return s_togglePin_lastPinIndex; }
uint8  IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void) { return s_setPinMode_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_setPinMode_lastMode; }
uint8  IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void) { return s_setPinModeLVDS_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void) { return s_setPinModeLVDS_lastPinMode; }
uint8  IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void) { return s_setPinModex_lastPinIndex; }
uint32 IfxPort_Mock_GetLastArg_setPinModex_modex(void) { return s_setPinModex_lastModex; }

void IfxPort_Mock_Reset(void)
{
    s_getPinState_count = 0;
    s_setPinHigh_count = 0;
    s_setPinLow_count = 0;
    s_setPinModeInput_count = 0;
    s_setPinModeOutput_count = 0;
    s_setPinState_count = 0;
    s_togglePin_count = 0;
    s_setPinMode_count = 0;
    s_setPinModeLVDS_count = 0;
    s_setPinModex_count = 0;

    s_getPinState_ret = 0u;

    s_getPinState_lastPinIndex = 0u;
    s_setPinHigh_lastPinIndex = 0u;
    s_setPinLow_lastPinIndex = 0u;
    s_setPinModeInput_lastPinIndex = 0u;
    s_setPinModeInput_lastMode = 0u;
    s_setPinModeOutput_lastPinIndex = 0u;
    s_setPinModeOutput_lastMode = 0u;
    s_setPinModeOutput_lastIndex = 0u;
    s_setPinState_lastPinIndex = 0u;
    s_setPinState_lastAction = 0u;
    s_togglePin_lastPinIndex = 0u;
    s_setPinMode_lastPinIndex = 0u;
    s_setPinMode_lastMode = 0u;
    s_setPinModeLVDS_lastPinIndex = 0u;
    s_setPinModeLVDS_lastPinMode = 0u;
    s_setPinModex_lastPinIndex = 0u;
    s_setPinModex_lastModex = 0u;
}
