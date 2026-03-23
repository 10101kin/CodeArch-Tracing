#include "IfxGtm_Tom_Pwm.h"

static uint32 s_cnt_IfxCcu6_PwmHl_start = 0u;

void IfxCcu6_PwmHl_start(void) {
    s_cnt_IfxCcu6_PwmHl_start++;
}

uint32 IfxGtm_Tom_Pwm_Mock_GetCallCount_IfxCcu6_PwmHl_start(void) {
    return s_cnt_IfxCcu6_PwmHl_start;
}

void IfxGtm_Tom_Pwm_Mock_Reset(void) {
    s_cnt_IfxCcu6_PwmHl_start = 0u;
}
