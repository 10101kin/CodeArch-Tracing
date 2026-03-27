#ifndef IFXSTDIF_H
#define IFXSTDIF_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Standard-interface shared typedefs/enums used by multiple GTM drivers */

typedef float32 Ifx_TimerValue; /* tick/time value used in timers/PWM */

typedef enum {
    IfxStdIf_Timer_CountDir_up = 0,
    IfxStdIf_Timer_CountDir_down = 1
} IfxStdIf_Timer_CountDir;

typedef struct {
    uint8 channelCount; /* number of PWM channels in std-if config */
} IfxStdIf_PwmHl_Config;

typedef struct {
    uint32 dummy; /* placeholder for standard timer config fields */
} IfxStdIf_Timer_Config;

typedef enum {
    Ifx_Pwm_Mode_off = 0,
    Ifx_Pwm_Mode_symmetric,
    Ifx_Pwm_Mode_asymmetric,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

#endif /* IFXSTDIF_H */
