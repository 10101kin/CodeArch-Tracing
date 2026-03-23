#include "IfxStm_Timer.h"

/* Call counters */
static uint32 s_IfxCcu6_Timer_initConfig_count = 0u;
static uint32 s_IfxCcu6_PwmHl_init_count = 0u;
static uint32 s_IfxCcu6_Timer_init_count = 0u;

void IfxCcu6_Timer_initConfig(IfxCcu6_Timer_Config *config)
{
    (void)config; /* do not dereference in mock */
    s_IfxCcu6_Timer_initConfig_count++;
}

void IfxCcu6_PwmHl_init(void)
{
    s_IfxCcu6_PwmHl_init_count++;
}

void IfxCcu6_Timer_init(void)
{
    s_IfxCcu6_Timer_init_count++;
}

uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_initConfig(void)
{
    return s_IfxCcu6_Timer_initConfig_count;
}

uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_PwmHl_init(void)
{
    return s_IfxCcu6_PwmHl_init_count;
}

uint32 IfxStm_Timer_Mock_GetCallCount_IfxCcu6_Timer_init(void)
{
    return s_IfxCcu6_Timer_init_count;
}

void IfxStm_Timer_Mock_Reset(void)
{
    s_IfxCcu6_Timer_initConfig_count = 0u;
    s_IfxCcu6_PwmHl_init_count = 0u;
    s_IfxCcu6_Timer_init_count = 0u;
}
