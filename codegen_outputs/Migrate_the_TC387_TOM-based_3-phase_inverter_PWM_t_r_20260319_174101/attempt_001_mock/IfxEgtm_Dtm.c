#include "IfxEgtm_Dtm.h"

static uint32 s_setRelrise_count = 0u;
static uint32 s_setOutput0DeadTimePath_count = 0u;
static uint32 s_setRelfall_count = 0u;
static uint32 s_setOutput1DeadTimePath_count = 0u;
static uint32 s_setOutput1Polarity_count = 0u;
static uint32 s_setOutput1Select_count = 0u;
static uint32 s_setOutput1Function_count = 0u;

static uint32 s_setRelrise_lastChannel = 0u; static uint16 s_setRelrise_lastRelrise = 0u;
static uint32 s_setOutput0DeadTimePath_lastChannel = 0u; static uint32 s_setOutput0DeadTimePath_lastPath = 0u;
static uint32 s_setRelfall_lastChannel = 0u; static uint16 s_setRelfall_lastRelfall = 0u;
static uint32 s_setOutput1DeadTimePath_lastChannel = 0u; static uint32 s_setOutput1DeadTimePath_lastPath = 0u;
static uint32 s_setOutput1Polarity_lastChannel = 0u; static uint32 s_setOutput1Polarity_lastPolarity = 0u;
static uint32 s_setOutput1Select_lastChannel = 0u; static uint32 s_setOutput1Select_lastSel = 0u;
static uint32 s_setOutput1Function_lastChannel = 0u; static uint32 s_setOutput1Function_lastFunc = 0u;

void IfxEgtm_Dtm_setRelrise(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relrise)
{
    (void)dtm;
    s_setRelrise_count++;
    s_setRelrise_lastChannel = (uint32)channel;
    s_setRelrise_lastRelrise = relrise;
}

void IfxEgtm_Dtm_setOutput0DeadTimePath(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_DeadTimePath deadTimePath)
{
    (void)dtm;
    s_setOutput0DeadTimePath_count++;
    s_setOutput0DeadTimePath_lastChannel = (uint32)channel;
    s_setOutput0DeadTimePath_lastPath = (uint32)deadTimePath;
}

void IfxEgtm_Dtm_setRelfall(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, uint16 relfall)
{
    (void)dtm;
    s_setRelfall_count++;
    s_setRelfall_lastChannel = (uint32)channel;
    s_setRelfall_lastRelfall = relfall;
}

void IfxEgtm_Dtm_setOutput1DeadTimePath(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_DeadTimePath deadTimePath)
{
    (void)dtm;
    s_setOutput1DeadTimePath_count++;
    s_setOutput1DeadTimePath_lastChannel = (uint32)channel;
    s_setOutput1DeadTimePath_lastPath = (uint32)deadTimePath;
}

void IfxEgtm_Dtm_setOutput1Polarity(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_OutputPolarity polarity)
{
    (void)dtm;
    s_setOutput1Polarity_count++;
    s_setOutput1Polarity_lastChannel = (uint32)channel;
    s_setOutput1Polarity_lastPolarity = (uint32)polarity;
}

void IfxEgtm_Dtm_setOutput1Select(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Select output1Select)
{
    (void)dtm;
    s_setOutput1Select_count++;
    s_setOutput1Select_lastChannel = (uint32)channel;
    s_setOutput1Select_lastSel = (uint32)output1Select;
}

void IfxEgtm_Dtm_setOutput1Function(Ifx_EGTM_CLS_CDTM_DTM *dtm, IfxEgtm_Dtm_Ch channel, IfxEgtm_Dtm_Output1Function output1Function)
{
    (void)dtm;
    s_setOutput1Function_count++;
    s_setOutput1Function_lastChannel = (uint32)channel;
    s_setOutput1Function_lastFunc = (uint32)output1Function;
}

uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelrise(void) { return s_setRelrise_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput0DeadTimePath(void) { return s_setOutput0DeadTimePath_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setRelfall(void) { return s_setRelfall_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1DeadTimePath(void) { return s_setOutput1DeadTimePath_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Polarity(void) { return s_setOutput1Polarity_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Select(void) { return s_setOutput1Select_count; }
uint32 IfxEgtm_Dtm_Mock_GetCallCount_setOutput1Function(void) { return s_setOutput1Function_count; }

uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_channel(void) { return s_setRelrise_lastChannel; }
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelrise_relrise(void) { return s_setRelrise_lastRelrise; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput0DeadTimePath_channel(void) { return s_setOutput0DeadTimePath_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput0DeadTimePath_deadTimePath(void) { return s_setOutput0DeadTimePath_lastPath; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_channel(void) { return s_setRelfall_lastChannel; }
uint16 IfxEgtm_Dtm_Mock_GetLastArg_setRelfall_relfall(void) { return s_setRelfall_lastRelfall; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1DeadTimePath_channel(void) { return s_setOutput1DeadTimePath_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1DeadTimePath_deadTimePath(void) { return s_setOutput1DeadTimePath_lastPath; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_channel(void) { return s_setOutput1Polarity_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Polarity_polarity(void) { return s_setOutput1Polarity_lastPolarity; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_channel(void) { return s_setOutput1Select_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Select_output1Select(void) { return s_setOutput1Select_lastSel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_channel(void) { return s_setOutput1Function_lastChannel; }
uint32 IfxEgtm_Dtm_Mock_GetLastArg_setOutput1Function_output1Function(void) { return s_setOutput1Function_lastFunc; }

void IfxEgtm_Dtm_Mock_Reset(void)
{
    s_setRelrise_count = 0u;
    s_setOutput0DeadTimePath_count = 0u;
    s_setRelfall_count = 0u;
    s_setOutput1DeadTimePath_count = 0u;
    s_setOutput1Polarity_count = 0u;
    s_setOutput1Select_count = 0u;
    s_setOutput1Function_count = 0u;

    s_setRelrise_lastChannel = 0u; s_setRelrise_lastRelrise = 0u;
    s_setOutput0DeadTimePath_lastChannel = 0u; s_setOutput0DeadTimePath_lastPath = 0u;
    s_setRelfall_lastChannel = 0u; s_setRelfall_lastRelfall = 0u;
    s_setOutput1DeadTimePath_lastChannel = 0u; s_setOutput1DeadTimePath_lastPath = 0u;
    s_setOutput1Polarity_lastChannel = 0u; s_setOutput1Polarity_lastPolarity = 0u;
    s_setOutput1Select_lastChannel = 0u; s_setOutput1Select_lastSel = 0u;
    s_setOutput1Function_lastChannel = 0u; s_setOutput1Function_lastFunc = 0u;
}
