#include "IfxGtm_Tom_Timer.h"

static uint32  s_init_count = 0u;
static boolean s_init_ret = (boolean)0;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{
    (void)driver;
    (void)config; /* No dereference per mock rules */
    s_init_count++;
    return s_init_ret;
}

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void)
{
    return s_init_count;
}

void IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value)
{
    s_init_ret = value;
}

void IfxGtm_Tom_Timer_Mock_Reset(void)
{
    s_init_count = 0u;
    s_init_ret = (boolean)0;
}
