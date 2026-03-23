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

/* Return values */
static boolean s_getPinState_ret = 0;

/* Last-arg captures */
static Ifx_P *s_getPinState_lastPort = NULL_PTR; static uint8 s_getPinState_lastPin = 0u;
static Ifx_P *s_setPinHigh_lastPort = NULL_PTR; static uint8 s_setPinHigh_lastPin = 0u;
static Ifx_P *s_setPinLow_lastPort  = NULL_PTR; static uint8 s_setPinLow_lastPin  = 0u;
static Ifx_P *s_setPinModeInput_lastPort = NULL_PTR; static uint8 s_setPinModeInput_lastPin = 0u; static uint32 s_setPinModeInput_lastMode = 0u;
static Ifx_P *s_setPinModeOutput_lastPort = NULL_PTR; static uint8 s_setPinModeOutput_lastPin = 0u; static uint32 s_setPinModeOutput_lastMode = 0u; static uint32 s_setPinModeOutput_lastIndex = 0u;
static Ifx_P *s_setPinState_lastPort = NULL_PTR; static uint8 s_setPinState_lastPin = 0u; static uint32 s_setPinState_lastAction = 0u;
static Ifx_P *s_togglePin_lastPort = NULL_PTR; static uint8 s_togglePin_lastPin = 0u;
static Ifx_P *s_setPinMode_lastPort = NULL_PTR; static uint8 s_setPinMode_lastPin = 0u; static uint32 s_setPinMode_lastMode = 0u;
static Ifx_P *s_setPinModeLVDS_lastPort = NULL_PTR; static uint8 s_setPinModeLVDS_lastPin = 0u; static uint32 s_setPinModeLVDS_lastPinMode = 0u; static IfxPort_LvdsConfig *s_setPinModeLVDS_lastLvds = NULL_PTR;
static Ifx_P *s_setPinModex_lastPort = NULL_PTR; static uint8 s_setPinModex_lastPin = 0u; static uint32 s_setPinModex_lastModex = 0u;

/* iLLD stubs */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex) {
    s_getPinState_count++;
    s_getPinState_lastPort = port;
    s_getPinState_lastPin = pinIndex;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) {
    (void)port; (void)pinIndex;
    s_setPinHigh_count++;
    s_setPinHigh_lastPort = port;
    s_setPinHigh_lastPin = pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) {
    (void)port; (void)pinIndex;
    s_setPinLow_count++;
    s_setPinLow_lastPort = port;
    s_setPinLow_lastPin = pinIndex;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode) {
    (void)port; (void)pinIndex; (void)mode;
    s_setPinModeInput_count++;
    s_setPinModeInput_lastPort = port;
    s_setPinModeInput_lastPin = pinIndex;
    s_setPinModeInput_lastMode = (uint32)mode;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) {
    (void)port; (void)pinIndex; (void)mode; (void)index;
    s_setPinModeOutput_count++;
    s_setPinModeOutput_lastPort = port;
    s_setPinModeOutput_lastPin = pinIndex;
    s_setPinModeOutput_lastMode = (uint32)mode;
    s_setPinModeOutput_lastIndex = (uint32)index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action) {
    (void)port; (void)pinIndex; (void)action;
    s_setPinState_count++;
    s_setPinState_lastPort = port;
    s_setPinState_lastPin = pinIndex;
    s_setPinState_lastAction = (uint32)action;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) {
    (void)port; (void)pinIndex;
    s_togglePin_count++;
    s_togglePin_lastPort = port;
    s_togglePin_lastPin = pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode) {
    (void)port; (void)pinIndex; (void)mode;
    s_setPinMode_count++;
    s_setPinMode_lastPort = port;
    s_setPinMode_lastPin = pinIndex;
    s_setPinMode_lastMode = (uint32)mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds) {
    (void)port; (void)pinIndex; (void)pinMode; (void)lvds;
    s_setPinModeLVDS_count++;
    s_setPinModeLVDS_lastPort = port;
    s_setPinModeLVDS_lastPin = pinIndex;
    s_setPinModeLVDS_lastPinMode = (uint32)pinMode;
    s_setPinModeLVDS_lastLvds = lvds;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex) {
    (void)port; (void)pinIndex; (void)modex;
    s_setPinModex_count++;
    s_setPinModex_lastPort = port;
    s_setPinModex_lastPin = pinIndex;
    s_setPinModex_lastModex = (uint32)modex;
}

/* Mock control implementations */
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

Ifx_P* IfxPort_Mock_GetLastArg_getPinState_port(void) { return s_getPinState_lastPort; }
uint8  IfxPort_Mock_GetLastArg_getPinState_pinIndex(void) { return s_getPinState_lastPin; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinHigh_port(void) { return s_setPinHigh_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void) { return s_setPinHigh_lastPin; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinLow_port(void) { return s_setPinLow_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void) { return s_setPinLow_lastPin; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinModeInput_port(void) { return s_setPinModeInput_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void) { return s_setPinModeInput_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_setPinModeInput_lastMode; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinModeOutput_port(void) { return s_setPinModeOutput_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void) { return s_setPinModeOutput_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_setPinModeOutput_lastMode; }
uint32 IfxPort_Mock_GetLastArg_setPinModeOutput_index(void) { return s_setPinModeOutput_lastIndex; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinState_port(void) { return s_setPinState_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinState_pinIndex(void) { return s_setPinState_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_setPinState_lastAction; }
Ifx_P* IfxPort_Mock_GetLastArg_togglePin_port(void) { return s_togglePin_lastPort; }
uint8  IfxPort_Mock_GetLastArg_togglePin_pinIndex(void) { return s_togglePin_lastPin; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinMode_port(void) { return s_setPinMode_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void) { return s_setPinMode_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_setPinMode_lastMode; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinModeLVDS_port(void) { return s_setPinModeLVDS_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void) { return s_setPinModeLVDS_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void) { return s_setPinModeLVDS_lastPinMode; }
IfxPort_LvdsConfig* IfxPort_Mock_GetLastArg_setPinModeLVDS_lvds(void) { return s_setPinModeLVDS_lastLvds; }
Ifx_P* IfxPort_Mock_GetLastArg_setPinModex_port(void) { return s_setPinModex_lastPort; }
uint8  IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void) { return s_setPinModex_lastPin; }
uint32 IfxPort_Mock_GetLastArg_setPinModex_modex(void) { return s_setPinModex_lastModex; }

void IfxPort_Mock_Reset(void) {
    s_getPinState_count = 0; s_setPinHigh_count = 0; s_setPinLow_count = 0;
    s_setPinModeInput_count = 0; s_setPinModeOutput_count = 0; s_setPinState_count = 0;
    s_togglePin_count = 0; s_setPinMode_count = 0; s_setPinModeLVDS_count = 0; s_setPinModex_count = 0;
    s_getPinState_ret = 0;
    s_getPinState_lastPort = NULL_PTR; s_getPinState_lastPin = 0u;
    s_setPinHigh_lastPort = NULL_PTR; s_setPinHigh_lastPin = 0u;
    s_setPinLow_lastPort = NULL_PTR; s_setPinLow_lastPin = 0u;
    s_setPinModeInput_lastPort = NULL_PTR; s_setPinModeInput_lastPin = 0u; s_setPinModeInput_lastMode = 0u;
    s_setPinModeOutput_lastPort = NULL_PTR; s_setPinModeOutput_lastPin = 0u; s_setPinModeOutput_lastMode = 0u; s_setPinModeOutput_lastIndex = 0u;
    s_setPinState_lastPort = NULL_PTR; s_setPinState_lastPin = 0u; s_setPinState_lastAction = 0u;
    s_togglePin_lastPort = NULL_PTR; s_togglePin_lastPin = 0u;
    s_setPinMode_lastPort = NULL_PTR; s_setPinMode_lastPin = 0u; s_setPinMode_lastMode = 0u;
    s_setPinModeLVDS_lastPort = NULL_PTR; s_setPinModeLVDS_lastPin = 0u; s_setPinModeLVDS_lastPinMode = 0u; s_setPinModeLVDS_lastLvds = NULL_PTR;
    s_setPinModex_lastPort = NULL_PTR; s_setPinModex_lastPin = 0u; s_setPinModex_lastModex = 0u;
}
