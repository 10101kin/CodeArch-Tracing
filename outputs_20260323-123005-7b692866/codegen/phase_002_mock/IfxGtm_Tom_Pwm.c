#include "IfxGtm_Tom_Pwm.h"

/* Call counters */
static uint32 s_IfxCcu6_PwmHl_start_count = 0u;

void IfxCcu6_PwmHl_start(void)
{
    s_IfxCcu6_PwmHl_start_count++;
}

uint32 IfxGtm_Tom_Pwm_Mock_GetCallCount_IfxCcu6_PwmHl_start(void)
{
    return s_IfxCcu6_PwmHl_start_count;
}

void IfxGtm_Tom_Pwm_Mock_Reset(void)
{
    s_IfxCcu6_PwmHl_start_count = 0u;
}
