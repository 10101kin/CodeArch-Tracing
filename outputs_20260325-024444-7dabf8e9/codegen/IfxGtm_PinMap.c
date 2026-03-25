#include "IfxGtm_PinMap.h"

static uint32 s_setTomTout_count = 0u;
static uint32 s_last_setTomTout_outputMode = 0u;
static uint32 s_last_setTomTout_padDriver = 0u;

void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver) {
    (void)config;
    s_setTomTout_count++;
    s_last_setTomTout_outputMode = (uint32)outputMode;
    s_last_setTomTout_padDriver = (uint32)padDriver;
}

uint32 IfxGtm_PinMap_Mock_GetCallCount_setTomTout(void) { return s_setTomTout_count; }
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_outputMode(void) { return s_last_setTomTout_outputMode; }
uint32 IfxGtm_PinMap_Mock_GetLastArg_setTomTout_padDriver(void) { return s_last_setTomTout_padDriver; }

void IfxGtm_PinMap_Mock_Reset(void) {
    s_setTomTout_count = 0u;
    s_last_setTomTout_outputMode = 0u;
    s_last_setTomTout_padDriver = 0u;
}
