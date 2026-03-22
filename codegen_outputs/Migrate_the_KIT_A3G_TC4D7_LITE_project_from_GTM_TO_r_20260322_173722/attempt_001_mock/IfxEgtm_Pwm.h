#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "IfxEgtm_Atom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
typedef struct IfxEgtm_Atom_PwmHl              IfxEgtm_Atom_PwmHl;
typedef struct IfxEgtm_Atom_PwmHl_Channel      IfxEgtm_Atom_PwmHl_Channel;
typedef enum {
    IfxEgtm_Atom_PwmHl_Alignment_leftAligned = 0,
    IfxEgtm_Atom_PwmHl_Alignment_centerAligned = 1
} IfxEgtm_Atom_PwmHl_Alignment;
typedef struct {
    float32 frequency;
    uint32  numChannels;
    IfxEgtm_Atom_PwmHl_Alignment alignment;
    boolean syncStart;
} IfxEgtm_Atom_PwmHl_Config;
typedef struct IfxEgtm_Atom_PwmHl_ChannelConfig IfxEgtm_Atom_PwmHl_ChannelConfig; /* forward */

/* Minimal forward/definition set for Atom PwmHl */
/* iLLD API mocks (exact signatures) */
/* Mock controls */
/* Config capture (Pattern D) */

/* ============= Function Declarations ============= */
void IfxEgtm_Atom_PwmHl_updateChannelsDutyImmediate(IfxEgtm_Atom_PwmHl *pwm, float32 *requestDuty);
void IfxEgtm_Atom_PwmHl_initConfig(IfxEgtm_Atom_PwmHl_Config *config, Ifx_EGTM *egtmSFR);
void IfxEgtm_Atom_PwmHl_startSyncedChannels(IfxEgtm_Atom_PwmHl *pwm);
void IfxEgtm_Atom_PwmHl_initChannelConfig(IfxEgtm_Atom_PwmHl_ChannelConfig *channelConfig);
void IfxEgtm_Atom_PwmHl_init(IfxEgtm_Atom_PwmHl *pwm, IfxEgtm_Atom_PwmHl_Channel *channels, IfxEgtm_Atom_PwmHl_Config *config);
uint32  IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_updateChannelsDutyImmediate(void);
float32 IfxEgtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxEgtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxEgtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_initConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_startSyncedChannels(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_initChannelConfig(void);
uint32 IfxEgtm_Pwm_Mock_GetCallCount_Atom_PwmHl_init(void);
float32 IfxEgtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxEgtm_Pwm_Mock_GetLastArg_init_syncStart(void);
void IfxEgtm_Pwm_Mock_Reset(void);


/* ── auto-injected missing function stubs ── */
void IfxEgtm_Pwm_updateChannelsDutyImmediate(void);
void IfxEgtm_Pwm_initConfig(void);
void IfxEgtm_Pwm_startSyncedChannels(void);
void IfxEgtm_Pwm_initChannelConfig(void);
void IfxEgtm_Pwm_init(void);

#endif