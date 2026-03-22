#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint32, float32 */

/* ============= Type Definitions ============= */
typedef struct IfxGtm_Pwm          IfxGtm_Pwm;
typedef struct IfxGtm_Pwm_Channel  IfxGtm_Pwm_Channel;
typedef struct IfxGtm_Pwm_Config   IfxGtm_Pwm_Config;

/* Opaque forward declarations for PWM types (no definitions needed for mocks) */
/* iLLD function declarations to mock */
/* Mock control functions */
/* Array capture + history accessors for updateChannelsDutyImmediate */
/* Reset all counters and captured values */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif