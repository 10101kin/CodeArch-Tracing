#include "IfxEgtm_Dtm.h"

/* Call counters */
static uint32 s_setRelfall_count = 0;
static uint32 s_setOutput1Select_count = 0;
static uint32 s_setOutput1Polarity_count = 0;
static uint32 s_setRelrise_count = 0;
static uint32 s_setClockSource_count = 0;
static uint32 s_setOutput1Function_count = 0;

/* Last-arg captures */
static uint32 s_setRelfall_lastChannel = 0;
static uint32 s_setRelfall_lastRelfall = 0;
static uint32 s_setOutput1Select_lastChannel = 0;
static uint32 s_setOutput1Select_lastOutput1Select = 0;
static uint32 s_setOutput1Polarity_lastChannel = 0;
static uint32 s_setOutput1Polarity_lastPolarity = 0;
static uint32 s_setRelrise_lastChannel = 0;
static uint32 s_setRelrise_lastRelrise = 0;
static uint32 s_setClockSource_lastClockSource = 0;
static uint32 s_setOutput1Function_lastChannel = 0;
static uint32 s_setOutput1Function_lastOutput1Function = 0;

void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall)
{
    (void)dtm;
    s_setRelfall_count++;
    s_setRelfall_lastChannel = (uint32)channel;
    s_setRelfall_lastRelfall = (uint32)relfall;
}

void IfxEgtm_Dtm_setOutput1Select(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Select output1Select)
{
    (void)dtm;
    s_setOutput1Select_count++;
    s_setOutput1Select_lastChannel = (uint32)channel;
    s_setOutput1Select_lastOutput1Select = (uint32)output1Select;
}

void IfxEgtm_Dtm_setOutput1Polarity(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_OutputPolarity polarity)
{
    (void)dtm;
    s_setOutput1Polarity_count++;
    s_setOutput1Polarity_lastChannel = (uint32)channel;
    s_setOutput1Polarity_lastPolarity = (uint32)polarity;
}

void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise)
{
    (void)dtm;
    s_setRelrise_count++;
    s_setRelrise_lastChannel = (uint32)channel;
    s_setRelrise_lastRelrise = (uint32)relrise;
}

void IfxEgtm_Dtm_setClockSource(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_ClockSource clockSource)
{
    (void)dtm;
    s_setClockSource_count++;
    s_setClockSource_lastClockSource = (uint32)clockSource;
}

void IfxEgtm_Dtm_setOutput1Function(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Function output1Function)
{
    (void)dtm;
    s_setOutput1Function_count++;
    s_setOutput1Function_lastChannel = (uint32)channel;
    s_setOutput1Function_lastOutput1Function = (uint32)output1Function;
}

/* Mock control: call counts */
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void) { return s_setRelfall_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Select(void) { return s_setOutput1Select_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Polarity(void) { return s_setOutput1Polarity_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void) { return s_setRelrise_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setClockSource(void) { return s_setClockSource_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Function(void) { return s_setOutput1Function_count; }

/* Mock control: last-arg capture getters */
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void) { return s_setRelfall_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void) { return s_setRelfall_lastRelfall; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_channel(void) { return s_setOutput1Select_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_output1Select(void) { return s_setOutput1Select_lastOutput1Select; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_channel(void) { return s_setOutput1Polarity_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_polarity(void) { return s_setOutput1Polarity_lastPolarity; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void) { return s_setRelrise_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void) { return s_setRelrise_lastRelrise; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setClockSource_clockSource(void) { return s_setClockSource_lastClockSource; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_channel(void) { return s_setOutput1Function_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_output1Function(void) { return s_setOutput1Function_lastOutput1Function; }

void IfxEgtm_Dtm_Mock_Reset(void)
{
    s_setRelfall_count = 0;
    s_setOutput1Select_count = 0;
    s_setOutput1Polarity_count = 0;
    s_setRelrise_count = 0;
    s_setClockSource_count = 0;
    s_setOutput1Function_count = 0;

    s_setRelfall_lastChannel = 0;
    s_setRelfall_lastRelfall = 0;
    s_setOutput1Select_lastChannel = 0;
    s_setOutput1Select_lastOutput1Select = 0;
    s_setOutput1Polarity_lastChannel = 0;
    s_setOutput1Polarity_lastPolarity = 0;
    s_setRelrise_lastChannel = 0;
    s_setRelrise_lastRelrise = 0;
    s_setClockSource_lastClockSource = 0;
    s_setOutput1Function_lastChannel = 0;
    s_setOutput1Function_lastOutput1Function = 0;
}
