#ifndef IFXSTDIF_H
#define IFXSTDIF_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Common standard interface base types */
typedef uint32 Ifx_TimerValue; /* timer tick type */

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct {
    float32 frequency;   /* desired frequency in Hz */
    IfxStdIf_Timer_CountDir countDir;
} IfxStdIf_Timer_Config;

typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_edgeAligned,
    Ifx_Pwm_Mode_centerAligned,
    Ifx_Pwm_Mode_centerAlignedInverted
} Ifx_Pwm_Mode;

typedef struct {
    uint8 channelCount; /* number of PWM outputs (HL pairs count*2) */
} IfxStdIf_PwmHl_Config;

#endif /* IFXSTDIF_H */
