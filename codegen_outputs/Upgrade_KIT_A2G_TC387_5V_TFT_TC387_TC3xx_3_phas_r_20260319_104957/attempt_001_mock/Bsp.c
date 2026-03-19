#include "Bsp.h"

static uint32       s_waitTime_count = 0;
static Ifx_TickTime s_waitTime_lastTicks = 0u;

void waitTime(Ifx_TickTime ticks) {
    s_waitTime_count++;
    s_waitTime_lastTicks = ticks;
}

uint32 Bsp_Mock_GetCallCount_waitTime(void) { return s_waitTime_count; }
Ifx_TickTime Bsp_Mock_GetLastArg_waitTime_ticks(void) { return s_waitTime_lastTicks; }
void Bsp_Mock_Reset(void) { s_waitTime_count = 0; s_waitTime_lastTicks = 0u; }
