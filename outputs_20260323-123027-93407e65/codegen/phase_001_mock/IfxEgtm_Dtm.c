#include "IfxEgtm_Dtm.h"

static uint32 s_setRelfall_count = 0u;
static uint32 s_setClockSource_count = 0u;
static uint32 s_setRelrise_count = 0u;

static uint16 s_setRelfall_lastRel = 0u;
static uint32 s_setClockSource_lastSrc = 0u;
static uint16 s_setRelrise_lastRel = 0u;

void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall)
{
    (void)dtm;
    (void)channel;
    s_setRelfall_count++;
    s_setRelfall_lastRel = relfall;
}

void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource)
{
    (void)dtm;
    s_setClockSource_count++;
    s_setClockSource_lastSrc = (uint32)clockSource;
}

void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise)
{
    (void)dtm;
    (void)channel;
    s_setRelrise_count++;
    s_setRelrise_lastRel = relrise;
}

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void)    { return s_setRelfall_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void){ return s_setClockSource_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void)    { return s_setRelrise_count; }

uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void)      { return s_setRelfall_lastRel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void){ return s_setClockSource_lastSrc; }
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void)      { return s_setRelrise_lastRel; }

void IfxEgtm_Dtm_Mock_Reset(void)
{
    s_setRelfall_count = 0u;
    s_setClockSource_count = 0u;
    s_setRelrise_count = 0u;

    s_setRelfall_lastRel = 0u;
    s_setClockSource_lastSrc = 0u;
    s_setRelrise_lastRel = 0u;
}
