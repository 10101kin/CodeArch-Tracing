/* Minimal StdIf definitions used by TOM Timer and PwmHl */
#ifndef IFXSTDIF_H
#define IFXSTDIF_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

typedef float32 Ifx_TimerValue;

typedef enum
{
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct
{
    float32                 frequency;
    IfxStdIf_Timer_CountDir countDir;
} IfxStdIf_Timer_Config;

typedef enum
{
    Ifx_Pwm_Mode_leftAligned = 0,
    Ifx_Pwm_Mode_centerAligned,
    Ifx_Pwm_Mode_edgeAligned
} Ifx_Pwm_Mode;

typedef struct
{
    uint8   channelCount;
} IfxStdIf_PwmHl_Config;

#endif /* IFXSTDIF_H */
