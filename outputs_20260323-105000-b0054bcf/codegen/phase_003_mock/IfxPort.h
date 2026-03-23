#ifndef IFXPORT_H
#define IFXPORT_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint32, boolean types */

/* ============= Type Definitions ============= */
/* IfxPort_OutputMode - from illd_types/Ifx_Types.h */
/* IfxPort_OutputIdx - from illd_types/Ifx_Types.h */
/* IfxPort_State - from illd_types/Ifx_Types.h */
/* IfxPort_InputMode - from illd_types/Ifx_Types.h */
/* IfxPort_Mode - from illd_types/Ifx_Types.h */
typedef struct IfxPort_LvdsConfig IfxPort_LvdsConfig; /* opaque */
typedef uint32 IfxPort_Modex;

/* Forward/placeholder typedefs for parameter types used by the real iLLD APIs.
   These are intentionally minimal to avoid pulling full driver definitions. */
/* Stubbed iLLD API declarations (exact signatures as provided) */
/* Mock control: call counters */
/* Mock control: set return values for non-void APIs */
/* Mock control: capture of last scalar/enum arguments */
/* Reset all counters and captured values */

/* ============= Function Declarations ============= */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action);
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode);
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode);
void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds);
void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex);
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void);
uint32 IfxPort_Mock_GetCallCount_setPinState(void);
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void);
uint32 IfxPort_Mock_GetCallCount_getPinState(void);
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void);
uint32 IfxPort_Mock_GetCallCount_setPinLow(void);
uint32 IfxPort_Mock_GetCallCount_togglePin(void);
uint32 IfxPort_Mock_GetCallCount_setPinMode(void);
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void);
uint32 IfxPort_Mock_GetCallCount_setPinModex(void);
void IfxPort_Mock_SetReturn_getPinState(boolean value);
uint8               IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void);
IfxPort_OutputMode  IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void);
IfxPort_OutputIdx   IfxPort_Mock_GetLastArg_setPinModeOutput_index(void);
uint8               IfxPort_Mock_GetLastArg_setPinState_pinIndex(void);
IfxPort_State       IfxPort_Mock_GetLastArg_setPinState_action(void);
uint8               IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void);
IfxPort_InputMode   IfxPort_Mock_GetLastArg_setPinModeInput_mode(void);
uint8               IfxPort_Mock_GetLastArg_getPinState_pinIndex(void);
uint8               IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void);
uint8               IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void);
uint8               IfxPort_Mock_GetLastArg_togglePin_pinIndex(void);
uint8               IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void);
IfxPort_Mode        IfxPort_Mock_GetLastArg_setPinMode_mode(void);
uint8               IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void);
IfxPort_Mode        IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void);
uint8               IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void);
IfxPort_Modex       IfxPort_Mock_GetLastArg_setPinModex_modex(void);
void IfxPort_Mock_Reset(void);

#endif