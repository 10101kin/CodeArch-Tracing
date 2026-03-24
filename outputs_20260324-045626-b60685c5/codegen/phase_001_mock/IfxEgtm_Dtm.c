#include "IfxEgtm_Dtm.h"

static uint32 s_setClockSource_count = 0u;
static uint32 s_setClockSource_last  = 0u;

void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource)
{
    (void)dtm;
    s_setClockSource_count++;
    s_setClockSource_last = (uint32)clockSource;
}

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void)
{
    return s_setClockSource_count;
}

uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource(void)
{
    return s_setClockSource_last;
}

void IfxEgtm_Dtm_Mock_Reset(void)
{
    s_setClockSource_count = 0u;
    s_setClockSource_last  = 0u;
}
