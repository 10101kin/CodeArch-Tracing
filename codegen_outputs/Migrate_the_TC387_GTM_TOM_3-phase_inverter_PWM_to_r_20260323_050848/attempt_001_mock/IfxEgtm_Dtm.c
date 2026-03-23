#include "IfxEgtm_Dtm.h"

static uint32 s_setRelrise_count = 0;
static uint32 s_setClockSource_count = 0;
static uint32 s_setRelfall_count = 0;

static uint32 s_last_setRelrise_channel = 0;
static uint32 s_last_setRelrise_relrise = 0;
static uint32 s_last_setClockSource_clockSource = 0;
static uint32 s_last_setRelfall_channel = 0;
static uint32 s_last_setRelfall_relfall = 0;

void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise) {
    (void)dtm;
    s_setRelrise_count++;
    s_last_setRelrise_channel = (uint32)channel;
    s_last_setRelrise_relrise = (uint32)relrise;
}

void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource) {
    (void)dtm;
    s_setClockSource_count++;
    s_last_setClockSource_clockSource = (uint32)clockSource;
}

void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall) {
    (void)dtm;
    s_setRelfall_count++;
    s_last_setRelfall_channel = (uint32)channel;
    s_last_setRelfall_relfall = (uint32)relfall;
}

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void) { return s_setRelrise_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void) { return s_setClockSource_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void) { return s_setRelfall_count; }

uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void) { return s_last_setRelrise_channel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void) { return s_last_setRelrise_relrise; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void) { return s_last_setClockSource_clockSource; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void) { return s_last_setRelfall_channel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void) { return s_last_setRelfall_relfall; }

void IfxEgtm_Dtm_Mock_Reset(void) {
    s_setRelrise_count = 0;
    s_setClockSource_count = 0;
    s_setRelfall_count = 0;
    s_last_setRelrise_channel = 0;
    s_last_setRelrise_relrise = 0;
    s_last_setClockSource_clockSource = 0;
    s_last_setRelfall_channel = 0;
    s_last_setRelfall_relfall = 0;
}
