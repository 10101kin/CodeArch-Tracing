#ifndef IFXEGTM_ATOM_PWM_H
#define IFXEGTM_ATOM_PWM_H

#include "IfxEgtm_Atom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* iLLD API declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxEgtm_Atom_Pwm_startSyncedChannels(IfxEgtm_Atom_Driver *driver);
void IfxEgtm_Atom_Pwm_init(void);
void IfxEgtm_Atom_Pwm_initConfig(void);
void IfxEgtm_Atom_Pwm_initChannelConfig(void);
void IfxEgtm_Atom_Pwm_updateChannelsDutyImmediate(void);
void IfxEgtm_Atom_Pwm_interruptHandler(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_interruptHandler(void);
void   IfxEgtm_Atom_Pwm_Mock_Reset(void);

#endif