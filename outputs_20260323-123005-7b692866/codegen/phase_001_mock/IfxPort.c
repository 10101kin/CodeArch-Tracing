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
static uint32 s_setPinModex_count = 0;

/* Return controls */
static boolean s_getPinState_ret = 0;

/* Last-argument captures */
static Ifx_P  *s_last_getPinState_port = NULL_PTR;
static uint8   s_last_getPinState_pinIndex = 0u;

static Ifx_P  *s_last_setPinHigh_port = NULL_PTR;
static uint8   s_last_setPinHigh_pinIndex = 0u;

static Ifx_P  *s_last_setPinLow_port = NULL_PTR;
static uint8   s_last_setPinLow_pinIndex = 0u;

static uint32  s_last_setPinModeInput_mode = 0u;
static uint8   s_last_setPinModeInput_pinIndex = 0u;

static uint32  s_last_setPinModeOutput_mode = 0u;
static uint32  s_last_setPinModeOutput_index = 0u;
static uint8   s_last_setPinModeOutput_pinIndex = 0u;

static uint32  s_last_setPinState_action = 0u;
static uint8   s_last_setPinState_pinIndex = 0u;

static uint32  s_last_setPinMode_mode = 0u;
static uint8   s_last_setPinMode_pinIndex = 0u;

static uint32  s_last_setPinModeLVDS_pinMode = 0u;
static uint8   s_last_setPinModeLVDS_pinIndex = 0u;

static uint32  s_last_setPinModex_modex = 0u;
static uint8   s_last_setPinModex_pinIndex = 0u;

/* API implementations */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) {
    s_getPinState_count++;
    s_last_getPinState_port = port;
    s_last_getPinState_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) {
    s_setPinHigh_count++;
    s_last_setPinHigh_port = port;
    s_last_setPinHigh_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) {
    s_setPinLow_count++;
    s_last_setPinLow_port = port;
    s_last_setPinLow_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) {
    s_setPinModeInput_count++;
    s_last_setPinModeInput_mode = (uint32)mode;
    s_last_setPinModeInput_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) {
    s_setPinModeOutput_count++;
    s_last_setPinModeOutput_mode = (uint32)mode;
    s_last_setPinModeOutput_index = (uint32)index;
    s_last_setPinModeOutput_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)mode;
    (void)index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action) {
    s_setPinState_count++;
    s_last_setPinState_action = (uint32)action;
    s_last_setPinState_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) {
    s_togglePin_count++;
    (void)port;
    (void)pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) {
    s_setPinMode_count++;
    s_last_setPinMode_mode = (uint32)mode;
    s_last_setPinMode_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds) {
    s_setPinModeLVDS_count++;
    s_last_setPinModeLVDS_pinMode = (uint32)pinMode;
    s_last_setPinModeLVDS_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)pinMode;
    (void)lvds;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex) {
    s_setPinModex_count++;
    s_last_setPinModex_modex = (uint32)modex;
    s_last_setPinModex_pinIndex = pinIndex;
    (void)port;
    (void)pinIndex;
    (void)modex;
}

/* Mock controls */
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

void IfxPort_Mock_SetReturn_getPinState(boolean value) { s_getPinState_ret = value; }

uint8  IfxPort_Mock_GetLastArg_getPinState_pinIndex(void) { return s_last_getPinState_pinIndex; }
Ifx_P* IfxPort_Mock_GetLastArg_getPinState_port(void) { return s_last_getPinState_port; }

uint8  IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void) { return s_last_setPinHigh_pinIndex; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinHigh_port(void) { return s_last_setPinHigh_port; }

uint8  IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void) { return s_last_setPinLow_pinIndex; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinLow_port(void) { return s_last_setPinLow_port; }

uint32 IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_last_setPinModeInput_mode; }
uint8  IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void) { return s_last_setPinModeInput_pinIndex; }

uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_last_setPinModeOutput_mode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_index(void) { return s_last_setPinModeOutput_index; }
uint8  IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void) { return s_last_setPinModeOutput_pinIndex; }

uint32 IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_last_setPinState_action; }
uint8  IfxPort_Mock_GetLastArg_setPinState_pinIndex(void) { return s_last_setPinState_pinIndex; }

uint32 IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_last_setPinMode_mode; }
uint8  IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void) { return s_last_setPinMode_pinIndex; }

uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void) { return s_last_setPinModeLVDS_pinMode; }
uint8  IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void) { return s_last_setPinModeLVDS_pinIndex; }

uint32 IfxPort_Mock_GetLastArg_setPinModex_modex(void) { return s_last_setPinModex_modex; }
uint8  IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void) { return s_last_setPinModex_pinIndex; }

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
    s_setPinModex_count = 0u;

    s_getPinState_ret = 0;

    s_last_getPinState_port = NULL_PTR;
    s_last_getPinState_pinIndex = 0u;

    s_last_setPinHigh_port = NULL_PTR;
    s_last_setPinHigh_pinIndex = 0u;

    s_last_setPinLow_port = NULL_PTR;
    s_last_setPinLow_pinIndex = 0u;

    s_last_setPinModeInput_mode = 0u;
    s_last_setPinModeInput_pinIndex = 0u;

    s_last_setPinModeOutput_mode = 0u;
    s_last_setPinModeOutput_index = 0u;
    s_last_setPinModeOutput_pinIndex = 0u;

    s_last_setPinState_action = 0u;
    s_last_setPinState_pinIndex = 0u;

    s_last_setPinMode_mode = 0u;
    s_last_setPinMode_pinIndex = 0u;

    s_last_setPinModeLVDS_pinMode = 0u;
    s_last_setPinModeLVDS_pinIndex = 0u;

    s_last_setPinModex_modex = 0u;
    s_last_setPinModex_pinIndex = 0u;
}
