#ifndef IFXSTDIF_H
#define IFXSTDIF_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* StdIf Timer count direction */
typedef enum { IfxStdIf_Timer_CountDir_up=0, IfxStdIf_Timer_CountDir_down=1, IfxStdIf_Timer_CountDir_upAndDown=2 } IfxStdIf_Timer_CountDir;

/* StdIf Timer config (minimal) */
typedef struct
{
    IfxStdIf_Timer_CountDir countDir;
    Ifx_TimerValue          period;
    float32                 frequency;
} IfxStdIf_Timer_Config;

/* StdIf PwmHl config with required fields (addresses previous build errors) */
typedef struct
{
    Ifx_TimerValue        deadtime;    /* required */
    Ifx_TimerValue        minPulse;    /* required */
    IfxPort_OutputMode    outputMode;  /* required */
    IfxPort_PadDriver     padDriver;   /* required */
    uint8                 channelCount;/* required by PwmHl */
} IfxStdIf_PwmHl_Config;

#endif /* IFXSTDIF_H */
