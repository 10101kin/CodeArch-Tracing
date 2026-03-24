#include "IfxEgtm_Atom_Pwm.h"

static uint32 s_startSyncedChannels_count = 0u;
static uint32 s_interruptHandler_count = 0u;
static uint32 s_init_count = 0u;
static uint32 s_initConfig_count = 0u;
static uint32 s_initChannelConfig_count = 0u;
static uint32 s_updateChannelsDeadTime_count = 0u;
static uint32 s_updateChannelsDuty_count = 0u;

void IfxEgtm_Atom_Pwm_startSyncedChannels(IfxEgtm_Atom_Driver *driver)
{
    (void)driver;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Atom_Pwm_interruptHandler(void)
{
    s_interruptHandler_count++;
}

void IfxEgtm_Atom_Pwm_init(void)
{
    s_init_count++;
}

void IfxEgtm_Atom_Pwm_initConfig(void)
{
    s_initConfig_count++;
}

void IfxEgtm_Atom_Pwm_initChannelConfig(void)
{
    s_initChannelConfig_count++;
}

void IfxEgtm_Atom_Pwm_updateChannelsDeadTime(void)
{
    s_updateChannelsDeadTime_count++;
}

void IfxEgtm_Atom_Pwm_updateChannelsDuty(void)
{
    s_updateChannelsDuty_count++;
}

uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void) { return s_updateChannelsDeadTime_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty(void) { return s_updateChannelsDuty_count; }

void IfxEgtm_Atom_Pwm_Mock_Reset(void)
{
    s_startSyncedChannels_count = 0u;
    s_interruptHandler_count = 0u;
    s_init_count = 0u;
    s_initConfig_count = 0u;
    s_initChannelConfig_count = 0u;
    s_updateChannelsDeadTime_count = 0u;
    s_updateChannelsDuty_count = 0u;
}
