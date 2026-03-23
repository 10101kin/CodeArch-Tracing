#include "IfxEgtm_PinMap.h"

static uint32 s_setAtomTout_count = 0;
static uint32 s_last_setAtomTout_outputMode = 0;
static uint32 s_last_setAtomTout_padDriver = 0;

void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver) {
    (void)config;
    s_setAtomTout_count++;
    s_last_setAtomTout_outputMode = (uint32)outputMode;
    s_last_setAtomTout_padDriver = (uint32)padDriver;
}

uint32 IfxEgtm_PinMap_Mock_GetCallCount_setAtomTout(void) { return s_setAtomTout_count; }
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_outputMode(void) { return s_last_setAtomTout_outputMode; }
uint32 IfxEgtm_PinMap_Mock_GetLastArg_setAtomTout_padDriver(void) { return s_last_setAtomTout_padDriver; }

void IfxEgtm_PinMap_Mock_Reset(void) {
    s_setAtomTout_count = 0;
    s_last_setAtomTout_outputMode = 0;
    s_last_setAtomTout_padDriver = 0;
}
