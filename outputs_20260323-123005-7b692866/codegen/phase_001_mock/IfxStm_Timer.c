#include "IfxStm_Timer.h"

/* Call counters */
static uint32 s_cnt_IfxCcu6_Timer_initConfig = 0u;
static uint32 s_cnt_IfxCcu6_PwmHl_init = 0u;
static uint32 s_cnt_IfxCcu6_Timer_init = 0u;
static uint32 s_cnt_IfxCcu6_PwmHl_initConfig = 0u;

void IfxCcu6_Timer_initConfig(IfxCcu6_Timer_Config *config) {
    (void)config; /* suppress unused-parameter warning */
    s_cnt_IfxCcu6_Timer_initConfig++;
}

void IfxCcu6_PwmHl_init(void) {
    s_cnt_IfxCcu6_PwmHl_init++;
}

void IfxCcu6_Timer_init(void) {
    s_cnt_IfxCcu6_Timer_init++;
}

void IfxCcu6_PwmHl_initConfig(IfxCcu6_PwmHl_Config *config) {
    (void)config; /* suppress unused-parameter warning */
    s_cnt_IfxCcu6_PwmHl_initConfig++;
}

/* Mock controls */
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_initConfig(void) { return s_cnt_IfxCcu6_Timer_initConfig; }
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_init(void) { return s_cnt_IfxCcu6_PwmHl_init; }
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_init(void) { return s_cnt_IfxCcu6_Timer_init; }
uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_initConfig(void) { return s_cnt_IfxCcu6_PwmHl_initConfig; }

void IfxStm_Timer_Mock_Reset(void) {
    s_cnt_IfxCcu6_Timer_initConfig = 0u;
    s_cnt_IfxCcu6_PwmHl_init = 0u;
    s_cnt_IfxCcu6_Timer_init = 0u;
    s_cnt_IfxCcu6_PwmHl_initConfig = 0u;
}
