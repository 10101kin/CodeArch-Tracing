#include "IfxPort.h"

/* Call counters */
static uint32 s_setPinModeOutput_count = 0u;
static uint32 s_setPinState_count      = 0u;
static uint32 s_setPinModeInput_count  = 0u;
static uint32 s_getPinState_count      = 0u;
static uint32 s_setPinHigh_count       = 0u;
static uint32 s_setPinLow_count        = 0u;
static uint32 s_togglePin_count        = 0u;
static uint32 s_setPinMode_count       = 0u;
static uint32 s_setPinModeLVDS_count   = 0u;
static uint32 s_setPinModex_count      = 0u;

/* Return values */
static boolean s_getPinState_ret = 0u;

/* Captured last arguments */
static uint8              s_last_setPinModeOutput_pinIndex = 0u;
static IfxPort_OutputMode s_last_setPinModeOutput_mode     = 0u;
static IfxPort_OutputIdx  s_last_setPinModeOutput_index    = 0u;

static uint8         s_last_setPinState_pinIndex = 0u;
static IfxPort_State s_last_setPinState_action   = 0u;

static uint8             s_last_setPinModeInput_pinIndex = 0u;
static IfxPort_InputMode s_last_setPinModeInput_mode     = 0u;

static uint8 s_last_getPinState_pinIndex = 0u;

static uint8 s_last_setPinHigh_pinIndex  = 0u;
static uint8 s_last_setPinLow_pinIndex   = 0u;
static uint8 s_last_togglePin_pinIndex   = 0u;

static uint8       s_last_setPinMode_pinIndex = 0u;
static IfxPort_Mode s_last_setPinMode_mode    = 0u;

static uint8        s_last_setPinModeLVDS_pinIndex = 0u;
static IfxPort_Mode s_last_setPinModeLVDS_pinMode  = 0u;

static uint8        s_last_setPinModex_pinIndex = 0u;
static IfxPort_Modex s_last_setPinModex_modex   = 0u;

/* API implementations */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port;
    s_setPinModeOutput_count++;
    s_last_setPinModeOutput_pinIndex = pinIndex;
    s_last_setPinModeOutput_mode = mode;
    s_last_setPinModeOutput_index = index;
}

void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action)
{
    (void)port;
    s_setPinState_count++;
    s_last_setPinState_pinIndex = pinIndex;
    s_last_setPinState_action = action;
}

void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode)
{
    (void)port;
    s_setPinModeInput_count++;
    s_last_setPinModeInput_pinIndex = pinIndex;
    s_last_setPinModeInput_mode = mode;
}

boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_getPinState_count++;
    s_last_getPinState_pinIndex = pinIndex;
    return s_getPinState_ret;
}

void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_setPinHigh_count++;
    s_last_setPinHigh_pinIndex = pinIndex;
}

void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_setPinLow_count++;
    s_last_setPinLow_pinIndex = pinIndex;
}

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    s_togglePin_count++;
    s_last_togglePin_pinIndex = pinIndex;
}

void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode)
{
    (void)port;
    s_setPinMode_count++;
    s_last_setPinMode_pinIndex = pinIndex;
    s_last_setPinMode_mode = mode;
}

void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds)
{
    (void)port;
    (void)lvds;
    s_setPinModeLVDS_count++;
    s_last_setPinModeLVDS_pinIndex = pinIndex;
    s_last_setPinModeLVDS_pinMode = pinMode;
}

void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex)
{
    (void)port;
    s_setPinModex_count++;
    s_last_setPinModex_pinIndex = pinIndex;
    s_last_setPinModex_modex = modex;
}

/* Mock control: GetCallCount */
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void) { return s_setPinModeOutput_count; }
uint32 IfxPort_Mock_GetCallCount_setPinState(void)      { return s_setPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void)  { return s_setPinModeInput_count; }
uint32 IfxPort_Mock_GetCallCount_getPinState(void)      { return s_getPinState_count; }
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void)       { return s_setPinHigh_count; }
uint32 IfxPort_Mock_GetCallCount_setPinLow(void)        { return s_setPinLow_count; }
uint32 IfxPort_Mock_GetCallCount_togglePin(void)        { return s_togglePin_count; }
uint32 IfxPort_Mock_GetCallCount_setPinMode(void)       { return s_setPinMode_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void)   { return s_setPinModeLVDS_count; }
uint32 IfxPort_Mock_GetCallCount_setPinModex(void)      { return s_setPinModex_count; }

/* Mock control: SetReturn for non-void */
void IfxPort_Mock_SetReturn_getPinState(boolean value) { s_getPinState_ret = value; }

/* Mock control: last-arg getters */
uint8 IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void) { return s_last_setPinModeOutput_pinIndex; }
IfxPort_OutputMode IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void) { return s_last_setPinModeOutput_mode; }
IfxPort_OutputIdx  IfxPort_Mock_GetLastArg_setPinModeOutput_index(void){ return s_last_setPinModeOutput_index; }

uint8 IfxPort_Mock_GetLastArg_setPinState_pinIndex(void) { return s_last_setPinState_pinIndex; }
IfxPort_State IfxPort_Mock_GetLastArg_setPinState_action(void) { return s_last_setPinState_action; }

uint8 IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void) { return s_last_setPinModeInput_pinIndex; }
IfxPort_InputMode IfxPort_Mock_GetLastArg_setPinModeInput_mode(void) { return s_last_setPinModeInput_mode; }

uint8 IfxPort_Mock_GetLastArg_getPinState_pinIndex(void) { return s_last_getPinState_pinIndex; }

uint8 IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void) { return s_last_setPinHigh_pinIndex; }
uint8 IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void)  { return s_last_setPinLow_pinIndex; }
uint8 IfxPort_Mock_GetLastArg_togglePin_pinIndex(void)  { return s_last_togglePin_pinIndex; }

uint8 IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void) { return s_last_setPinMode_pinIndex; }
IfxPort_Mode IfxPort_Mock_GetLastArg_setPinMode_mode(void) { return s_last_setPinMode_mode; }

uint8 IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void) { return s_last_setPinModeLVDS_pinIndex; }
IfxPort_Mode IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void) { return s_last_setPinModeLVDS_pinMode; }

uint8 IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void) { return s_last_setPinModex_pinIndex; }
IfxPort_Modex IfxPort_Mock_GetLastArg_setPinModex_modex(void) { return s_last_setPinModex_modex; }

/* Reset all counters and captured state */
void IfxPort_Mock_Reset(void)
{
    s_setPinModeOutput_count = 0u;
    s_setPinState_count      = 0u;
    s_setPinModeInput_count  = 0u;
    s_getPinState_count      = 0u;
    s_setPinHigh_count       = 0u;
    s_setPinLow_count        = 0u;
    s_togglePin_count        = 0u;
    s_setPinMode_count       = 0u;
    s_setPinModeLVDS_count   = 0u;
    s_setPinModex_count      = 0u;

    s_getPinState_ret = 0u;

    s_last_setPinModeOutput_pinIndex = 0u;
    s_last_setPinModeOutput_mode     = 0u;
    s_last_setPinModeOutput_index    = 0u;

    s_last_setPinState_pinIndex = 0u;
    s_last_setPinState_action   = 0u;

    s_last_setPinModeInput_pinIndex = 0u;
    s_last_setPinModeInput_mode     = 0u;

    s_last_getPinState_pinIndex = 0u;

    s_last_setPinHigh_pinIndex = 0u;
    s_last_setPinLow_pinIndex  = 0u;
    s_last_togglePin_pinIndex  = 0u;

    s_last_setPinMode_pinIndex = 0u;
    s_last_setPinMode_mode     = 0u;

    s_last_setPinModeLVDS_pinIndex = 0u;
    s_last_setPinModeLVDS_pinMode  = 0u;

    s_last_setPinModex_pinIndex = 0u;
    s_last_setPinModex_modex    = 0u;
}
