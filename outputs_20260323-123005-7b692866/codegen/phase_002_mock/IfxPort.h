#ifndef IFXPORT_H
#define IFXPORT_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
/* IfxPort_InputMode - from illd_types/Ifx_Types.h */
/* IfxPort_OutputMode - from illd_types/Ifx_Types.h */
/* IfxPort_OutputIdx - from illd_types/Ifx_Types.h */
/* IfxPort_State - from illd_types/Ifx_Types.h */
/* IfxPort_Mode - from illd_types/Ifx_Types.h */
typedef uint32 IfxPort_Modex;
typedef struct IfxPort_LvdsConfig { uint32 reserved; } IfxPort_LvdsConfig;

/* Minimal type aliases to avoid pulling in full iLLD enum definitions.
   IMPORTANT: We intentionally do NOT declare any enum values here to
   prevent redeclaration conflicts with real iLLD enumerators. */
/* Function declarations (as required) */
/* Mock control functions */
/* Return control for non-void functions */
/* Last-argument captures for value-carrying params */

/* ============= Function Declarations ============= */
boolean IfxPort_getPinState(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinModeInput(Ifx_P *port, uint8 pinIndex, IfxPort_InputMode mode);
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State action);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinMode(Ifx_P *port, uint8 pinIndex, IfxPort_Mode mode);
void IfxPort_setPinModeLVDS(Ifx_P *port, uint8 pinIndex, IfxPort_Mode pinMode, IfxPort_LvdsConfig *lvds);
void IfxPort_setPinModex(Ifx_P *port, uint8 pinIndex, IfxPort_Modex modex);
uint32 IfxPort_Mock_GetCallCount_getPinState(void);
uint32 IfxPort_Mock_GetCallCount_setPinHigh(void);
uint32 IfxPort_Mock_GetCallCount_setPinLow(void);
uint32 IfxPort_Mock_GetCallCount_setPinModeInput(void);
uint32 IfxPort_Mock_GetCallCount_setPinModeOutput(void);
uint32 IfxPort_Mock_GetCallCount_setPinState(void);
uint32 IfxPort_Mock_GetCallCount_togglePin(void);
uint32 IfxPort_Mock_GetCallCount_setPinMode(void);
uint32 IfxPort_Mock_GetCallCount_setPinModeLVDS(void);
uint32 IfxPort_Mock_GetCallCount_setPinModex(void);
void    IfxPort_Mock_SetReturn_getPinState(boolean value);
uint32  IfxPort_Mock_GetLastArg_getPinState_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_togglePin_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeInput_mode(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeOutput_index(void);
uint32  IfxPort_Mock_GetLastArg_setPinState_action(void);
uint32  IfxPort_Mock_GetLastArg_setPinMode_mode(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void);
uint32  IfxPort_Mock_GetLastArg_setPinModex_modex(void);
void IfxPort_Mock_Reset(void);

#endif /* IFXPORT_H */