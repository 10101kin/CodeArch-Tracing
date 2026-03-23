#ifndef IFXPORT_H
#define IFXPORT_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD function declarations */
/* Mock controls: call counts */
/* Mock controls: return value setters/getters */
/* Mock controls: argument capture getters */
/* Mock controls: reset */

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
Ifx_P*  IfxPort_Mock_GetLastArg_getPinState_port(void);
uint8   IfxPort_Mock_GetLastArg_getPinState_pinIndex(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinHigh_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinHigh_pinIndex(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinLow_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinLow_pinIndex(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinModeInput_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeInput_mode(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinModeOutput_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeOutput_mode(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeOutput_index(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinState_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinState_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinState_action(void);
Ifx_P*  IfxPort_Mock_GetLastArg_togglePin_port(void);
uint8   IfxPort_Mock_GetLastArg_togglePin_pinIndex(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinMode_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinMode_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinMode_mode(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinModeLVDS_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinModeLVDS_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinModeLVDS_pinMode(void);
IfxPort_LvdsConfig* IfxPort_Mock_GetLastArg_setPinModeLVDS_lvds(void);
Ifx_P*  IfxPort_Mock_GetLastArg_setPinModex_port(void);
uint8   IfxPort_Mock_GetLastArg_setPinModex_pinIndex(void);
uint32  IfxPort_Mock_GetLastArg_setPinModex_modex(void);
void IfxPort_Mock_Reset(void);

#endif