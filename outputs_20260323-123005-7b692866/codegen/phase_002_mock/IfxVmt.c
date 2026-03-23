#include "IfxVmt.h"

/* Call counters */
static uint32 s_IfxCcu6_enableModule_count = 0u;

void IfxCcu6_enableModule(void)
{
    s_IfxCcu6_enableModule_count++;
}

uint32 IfxVmt_Mock_GetCallCount_IfxCcu6_enableModule(void)
{
    return s_IfxCcu6_enableModule_count;
}

void IfxVmt_Mock_Reset(void)
{
    s_IfxCcu6_enableModule_count = 0u;
}
