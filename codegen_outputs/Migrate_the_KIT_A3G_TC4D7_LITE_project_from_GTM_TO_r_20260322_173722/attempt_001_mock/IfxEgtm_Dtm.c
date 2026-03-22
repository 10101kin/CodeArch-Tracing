#include "IfxEgtm_Dtm.h"

static uint32 s_setRelrise_count = 0;
static uint32 s_setRelrise_lastChannel = 0;
static uint16 s_setRelrise_lastRelrise = 0u;

static uint32 s_setClockSource_count = 0;
static uint32 s_setClockSource_lastClock = 0;

static uint32 s_setRelfall_count = 0;
static uint32 s_setRelfall_lastChannel = 0;
static uint16 s_setRelfall_lastRelfall = 0u;

void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise) {
    (void)dtm;
    s_setRelrise_count++;
    s_setRelrise_lastChannel = (uint32)channel;
    s_setRelrise_lastRelrise = relrise;
}

void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource) {
    (void)dtm;
    s_setClockSource_count++;
    s_setClockSource_lastClock = (uint32)clockSource;
}

void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall) {
    (void)dtm;
    s_setRelfall_count++;
    s_setRelfall_lastChannel = (uint32)channel;
    s_setRelfall_lastRelfall = relfall;
}

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void) { return s_setRelrise_count; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void) { return s_setRelrise_lastChannel; }
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void) { return s_setRelrise_lastRelrise; }

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void) { return s_setClockSource_count; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void) { return s_setClockSource_lastClock; }

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void) { return s_setRelfall_count; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void) { return s_setRelfall_lastChannel; }
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void) { return s_setRelfall_lastRelfall; }

void IfxEgtm_Dtm_Mock_Reset(void) {
    s_setRelrise_count = 0u; s_setRelrise_lastChannel = 0u; s_setRelrise_lastRelrise = 0u;
    s_setClockSource_count = 0u; s_setClockSource_lastClock = 0u;
    s_setRelfall_count = 0u; s_setRelfall_lastChannel = 0u; s_setRelfall_lastRelfall = 0u;
}
