#ifndef IFXGTM_PWM_H
#define IFXGTM_PWM_H

#include "illd_types/Ifx_Types.h"  /* REQUIRED for uint8, uint16, uint32, boolean, float32 types */

/* ============= Type Definitions ============= */
struct IfxGtm_Pwm { uint32 _opaque; };
struct IfxGtm_Pwm_Channel { uint32 _opaque; };
typedef struct IfxGtm_Pwm_Config {
    float32 frequency;
    uint32  numChannels;
    uint32  alignment;   /* enum in real iLLD; stored as uint32 here */
    uint32  syncStart;   /* boolean/enum in real iLLD; stored as uint32 here */
} IfxGtm_Pwm_Config;

/* Minimal mock struct definitions to support Pattern D config capture */
/* Handles remain opaque; Config exposes key fields for tests. */
/* Function declarations */
/* Mock control functions */

/* ============= Function Declarations ============= */
void IfxGtm_Pwm_updateChannelsDutyImmediate(IfxGtm_Pwm *pwm, float32 *requestDuty);
void IfxGtm_Pwm_startSyncedChannels(IfxGtm_Pwm *pwm);
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config);
uint32  IfxGtm_Pwm_Mock_GetCallCount_updateChannelsDutyImmediate(void);
float32 IfxGtm_Pwm_Mock_GetLastArrayArg_updateChannelsDutyImmediate(uint32 index);
float32 IfxGtm_Pwm_Mock_GetArgHistory_updateChannelsDutyImmediate(uint32 callIdx, uint32 elemIdx);
uint32  IfxGtm_Pwm_Mock_GetArgHistoryCount_updateChannelsDutyImmediate(void);
uint32 IfxGtm_Pwm_Mock_GetCallCount_startSyncedChannels(void);
uint32  IfxGtm_Pwm_Mock_GetCallCount_init(void);
float32 IfxGtm_Pwm_Mock_GetLastArg_init_frequency(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_numChannels(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_alignment(void);
uint32  IfxGtm_Pwm_Mock_GetLastArg_init_syncStart(void);
void IfxGtm_Pwm_Mock_Reset(void);

#endif /* IFXGTM_PWM_H */