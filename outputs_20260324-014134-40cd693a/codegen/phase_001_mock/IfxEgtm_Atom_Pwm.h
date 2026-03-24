#ifndef IFXEGTM_ATOM_PWM_H
#define IFXEGTM_ATOM_PWM_H

#include "IfxEgtm_Atom.h"
#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
typedef struct IfxEgtm_Atom_Driver IfxEgtm_Atom_Driver;
typedef struct IfxEgtm_Atom_Pwm IfxEgtm_Atom_Pwm;            /* opaque if referenced */
typedef struct IfxEgtm_Atom_Pwm_Config IfxEgtm_Atom_Pwm_Config;  /* opaque if referenced */
typedef struct IfxEgtm_Atom_Pwm_Channel IfxEgtm_Atom_Pwm_Channel;/* opaque if referenced */

/* Forward declarations for driver handle types */
/* iLLD API declarations to mock (exact signatures) */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxEgtm_Atom_Pwm_startSyncedChannels(IfxEgtm_Atom_Driver *driver);
void IfxEgtm_Atom_Pwm_interruptHandler(void);
void IfxEgtm_Atom_Pwm_init(void);
void IfxEgtm_Atom_Pwm_initConfig(void);
void IfxEgtm_Atom_Pwm_initChannelConfig(void);
void IfxEgtm_Atom_Pwm_updateChannelsDeadTime(void);
void IfxEgtm_Atom_Pwm_updateChannelsDuty(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_interruptHandler(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_init(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initConfig(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_initChannelConfig(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDeadTime(void);
uint32 IfxEgtm_Atom_Pwm_Mock_GetCallCount_updateChannelsDuty(void);
void   IfxEgtm_Atom_Pwm_Mock_Reset(void);

#endif