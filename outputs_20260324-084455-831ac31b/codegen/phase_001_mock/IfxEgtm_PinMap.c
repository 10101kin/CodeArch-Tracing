#include "IfxEgtm_PinMap.h"

/* Call counters */
static uint32 s_setAtomTout_count = 0;

/* Last-arg captures */
static uint32 s_setAtomTout_lastOutputMode = 0;
static uint32 s_setAtomTout_lastPadDriver = 0;

void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config;
    s_setAtomTout_count++;
    s_setAtomTout_lastOutputMode = (uint32)outputMode;
    s_setAtomTout_lastPadDriver = (uint32)padDriver;
}

uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void) { return s_setAtomTout_count; }
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void) { return s_setAtomTout_lastOutputMode; }
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void) { return s_setAtomTout_lastPadDriver; }

void IfxEgtm_PinMap_Mock_Reset(void)
{
    s_setAtomTout_count = 0;
    s_setAtomTout_lastOutputMode = 0;
    s_setAtomTout_lastPadDriver = 0;
}
