#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* for float32, uint32 */

/* ============= Type Definitions ============= */
typedef struct IfxGtm_Pwm         IfxGtm_Pwm;
typedef struct IfxGtm_Pwm_Channel IfxGtm_Pwm_Channel;
typedef struct IfxGtm_Pwm_Config
{
    float32 frequency;    /* Hz */

/* Opaque handle/channel types and a minimal Config to support Pattern D capture */
    uint32  numChannels;  /* channel count */
    uint32  alignment;    /* cast from enum in real driver */
    uint32  syncStart;    /* boolean-like */
} IfxGtm_Pwm_Config;
/* API declarations */
/* Mock controls */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif