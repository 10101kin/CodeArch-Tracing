#include "IfxEgtm_Atom_Pwm.h"

static uint32 s_startSyncedChannels_count = 0;
static uint32 s_init_count = 0;
static uint32 s_initConfig_count = 0;
static uint32 s_initChannelConfig_count = 0;
static uint32 s_updateChannelsDutyImmediate_count = 0;
static uint32 s_interruptHandler_count = 0;

void IfxEgtm_Atom_Pwm_startSyncedChannels(IfxEgtm_Atom_Driver *driver) {
    (void)driver;
    s_startSyncedChannels_count++;
}

void IfxEgtm_Atom_Pwm_init(void) {
    s_init_count++;
}

void IfxEgtm_Atom_Pwm_initConfig(void) {
    s_initConfig_count++;
}

void IfxEgtm_Atom_Pwm_initChannelConfig(void) {
    s_initChannelConfig_count++;
}

void IfxEgtm_Atom_Pwm_updateChannelsDutyImmediate(void) {
    s_updateChannelsDutyImmediate_count++;
}

void IfxEgtm_Atom_Pwm_interruptHandler(void) {
    s_interruptHandler_count++;
}

uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels(void) { return s_startSyncedChannels_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_init(void) { return s_init_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig(void) { return s_initConfig_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initChannelConfig(void) { return s_initChannelConfig_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void) { return s_updateChannelsDutyImmediate_count; }
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_interruptHandler(void) { return s_interruptHandler_count; }

void IfxEgtm_Atom_Pwm_Mock_Reset(void) {
    s_startSyncedChannels_count = 0;
    s_init_count = 0;
    s_initConfig_count = 0;
    s_initChannelConfig_count = 0;
    s_updateChannelsDutyImmediate_count = 0;
    s_interruptHandler_count = 0;
}
