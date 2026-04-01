/*
 * egtm_atom_tmadc_consolidated.c
 *
 * Consolidated eGTM ATOM + TMADC driver for TC4xx
 * Implements a 3-phase complementary, center-aligned PWM using eGTM ATOM with DTM,
 * an ATOM trigger channel for TMADC synchronization, and basic TMADC module init.
 *
 * Notes:
 * - Watchdog handling must be done only in CpuX_Main.c (IfxWtu_* APIs). No watchdog code here.
 * - This module avoids dynamic memory and follows iLLD config/init patterns.
 */

#include "egtm_atom_tmadc_consolidated.h"

/* iLLD base types */
#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* eGTM high-level PWM driver */
#include "IfxEgtm_Pwm.h"
/* ATOM timer helper for trigger generation */
#include "IfxEgtm_Atom_Timer.h"
/* eGTM trigger routing to ADC */
#include "IfxEgtm_Trigger.h"
/* eGTM PinMap low-level helper for ATOM TOUT routing */
#include "IfxEgtm_PinMap.h"

/* TMADC module */
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"

/* ========================= Macros and configuration constants ========================= */

/* Channel count and PWM frequency */
#define IFXEGTM_NUM_CHANNELS             (3U)
#define PWM_FREQUENCY_HZ                 (20000.0f)

/* Initial duty cycles (percent) */
#define PHASE_U_DUTY_PCT                 (25.0f)
#define PHASE_V_DUTY_PCT                 (50.0f)
#define PHASE_W_DUTY_PCT                 (75.0f)

/* Duty increment step (percent) - kept for application use */
#define PHASE_DUTY_STEP_PCT              (0.01f)

/* Dead-time for complementary outputs (seconds) */
#define PWM_DEAD_TIME_SEC                (1.0e-6f)

/* ISR priority for eGTM ATOM PWM (debug ISR) */
#define ISR_PRIORITY_ATOM                (25)

/* LED macro (port, pin) required by structural rules; configured after PWM init */
#define LED                               &MODULE_P13, 0

/* ========================= Pin selection macros ========================= */
/*
 * Use ONLY validated pin symbols provided. Where requested pins are not listed,
 * leave the mapping as NULL_PTR for integration-time binding.
 */

/* Phase U: CH0 high/low (validated) */
#define PHASE_U_HS_TOUT   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS_TOUT   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V: CH1 high/low (user requested P20.10/P20.11 not present in validated subset) */
#define PHASE_V_HS_TOUT   (NULL_PTR)  /* TODO: Bind to valid ATOM0 CH1 TOUT during board integration */
#define PHASE_V_LS_TOUT   (NULL_PTR)  /* TODO: Bind to valid ATOM0 CH1N TOUT during board integration */

/* Phase W: CH2 high/low (user requested P20.12/P20.13 not present in validated subset) */
#define PHASE_W_HS_TOUT   (NULL_PTR)  /* TODO: Bind to valid ATOM0 CH2 TOUT during board integration */
#define PHASE_W_LS_TOUT   (NULL_PTR)  /* TODO: Bind to valid ATOM0 CH2N TOUT during board integration */

/* ATOM trigger channel output (user requested P33.0 not present in validated subset) */
#define ATOM_TRIG_TOUT    (NULL_PTR)  /* TODO: Bind to ATOM0 CH3 TOUT pad as available */

/* ========================= Module state ========================= */

typedef struct
{
    IfxEgtm_Pwm            pwm;                                       /* PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[IFXEGTM_NUM_CHANNELS];            /* Persistent channels (driver stores address) */
    float32                dutyCycles[IFXEGTM_NUM_CHANNELS];          /* Percent 0..100 */
    float32                phases[IFXEGTM_NUM_CHANNELS];              /* Phase offsets in percent (0..100) */
    IfxEgtm_Pwm_DeadTime   deadTimes[IFXEGTM_NUM_CHANNELS];           /* Stored rising/falling dead-times */
} EGTM_ATOM_TMADC_State;

IFX_STATIC EGTM_ATOM_TMADC_State g_egtmAtomTmadc;

/* ========================= ISR and callback ========================= */

/* Debug ISR: must only toggle LED */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: required to exist; must be empty */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Initialization ========================= */

/*
 * Initialize consolidated eGTM ATOM + TMADC setup:
 * - 3 complementary, center-aligned ATOM channels with DTM dead-time
 * - ATOM trigger channel (CH3) at 50% of PWM period, routed to TMADC
 * - TMADC module init (module-level)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configs locally */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[IFXEGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     outputCfg[IFXEGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmCfg[IFXEGTM_NUM_CHANNELS];

    IfxEgtm_Atom_Timer           atomTrigTimer; /* Logical ATOM trigger channel driver */

    IfxAdc_Tmadc                 tmadc;
    IfxAdc_Tmadc_Config          tmadcCfg;

    /* 2) PWM unified config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Prepare output configuration for each logical complementary channel */
    /* Phase U (logical channel 0) */
    outputCfg[0].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS_TOUT;
    outputCfg[0].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS_TOUT;
    outputCfg[0].polarity                = Ifx_ActiveState_high;  /* HS active-high */
    outputCfg[0].complementaryPolarity   = Ifx_ActiveState_low;   /* LS active-low  */
    outputCfg[0].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (logical channel 1) */
    outputCfg[1].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS_TOUT;  /* NULL_PTR placeholder */
    outputCfg[1].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS_TOUT;  /* NULL_PTR placeholder */
    outputCfg[1].polarity                = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[1].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (logical channel 2) */
    outputCfg[2].pin                     = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS_TOUT;  /* NULL_PTR placeholder */
    outputCfg[2].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS_TOUT;  /* NULL_PTR placeholder */
    outputCfg[2].polarity                = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[2].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM dead-time configuration for each channel (1 us rising/falling) */
    dtmCfg[0].deadTime.rising = PWM_DEAD_TIME_SEC; dtmCfg[0].deadTime.falling = PWM_DEAD_TIME_SEC; dtmCfg[0].fastShutOff = NULL_PTR;
    dtmCfg[1].deadTime.rising = PWM_DEAD_TIME_SEC; dtmCfg[1].deadTime.falling = PWM_DEAD_TIME_SEC; dtmCfg[1].fastShutOff = NULL_PTR;
    dtmCfg[2].deadTime.rising = PWM_DEAD_TIME_SEC; dtmCfg[2].deadTime.falling = PWM_DEAD_TIME_SEC; dtmCfg[2].fastShutOff = NULL_PTR;

    /* Channel configurations (indices 0..2) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;                  /* center-aligned phase offset if used */
    channelConfig[0].duty       = PHASE_U_DUTY_PCT;
    channelConfig[0].dtm        = &dtmCfg[0];
    channelConfig[0].output     = &outputCfg[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR;              /* Interrupt routing not used here */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_PCT;
    channelConfig[1].dtm        = &dtmCfg[1];
    channelConfig[1].output     = &outputCfg[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_PCT;
    channelConfig[2].dtm        = &dtmCfg[2];
    channelConfig[2].output     = &outputCfg[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration fields */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;          /* ATOM submodule */
    config.alignment           = IfxEgtm_Pwm_Alignment_center;        /* center-aligned */
    config.numChannels         = (uint8)IFXEGTM_NUM_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;           /* CMU Clock 0 for ATOM */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM uses CMU Clock 0 */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE
    config.highResEnable       = FALSE;
    config.dtmHighResEnable    = FALSE;
#endif
    config.syncUpdateEnabled   = TRUE;                                /* keep sync update enabled */
    config.syncStart           = TRUE;                                /* sync start channels */

    /* 3) ATOM trigger timer setup: frequency matches PWM period; 50% trigger point */
    /* The ATOM timer driver is configured via setFrequency/ setTrigger/ run APIs */
    {
        boolean ok;
        ok = IfxEgtm_Atom_Timer_setFrequency(&atomTrigTimer, PWM_FREQUENCY_HZ);
        if (ok == FALSE)
        {
            /* In production, handle error (e.g., assert/log). For now, continue. */
        }
        /* Trigger point is implementation-specific ticks; using 50% duty ratio placeholder */
        IfxEgtm_Atom_Timer_setTrigger(&atomTrigTimer, (uint32)50U);
    }

    /* 4) Configure routing from ATOM trigger source to ADC trigger signal */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                    (IfxEgtm_TrigSource)0,         /* EGTM.ATOM0.CH3 source selector: to be refined in integration */
                                    (IfxEgtm_TrigChannel)3,        /* logical trigger channel index */
                                    (IfxEgtm_Cfg_AdcTriggerSignal)0/* AdcTriggerSignal_0 */);

    /* 5) eGTM enable guard + CMU clocking (inside guard only) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamic frequency context from module */
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            /* Set GCLK and CMU CLK0 to module frequency, then enable FXCLK and CLK0 */
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 6) Initialize PWM with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtomTmadc.pwm, &g_egtmAtomTmadc.channels[0], &config);

    /* 7) Start the ATOM trigger timer */
    IfxEgtm_Atom_Timer_run(&atomTrigTimer);

    /* 8) Route trigger channel output to pad (if mapping available) */
    if (ATOM_TRIG_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ATOM_TRIG_TOUT,
                                   IfxPort_OutputMode_pushPull,
                                   IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* 9) Initialize TMADC module */
    IfxAdc_enableModule(&MODULE_ADC);
    IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
    /* Channel-level one-shot configuration and ISR wiring are performed at integration stage
       using TMADC channel APIs (not part of the current mock set). */
    IfxAdc_Tmadc_initModule(&tmadc, &tmadcCfg);

    /* 10) Configure the LED GPIO (debug ISR toggles this LED) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Persist initial state for runtime updates */
    g_egtmAtomTmadc.dutyCycles[0] = PHASE_U_DUTY_PCT; g_egtmAtomTmadc.phases[0] = 0.0f;
    g_egtmAtomTmadc.dutyCycles[1] = PHASE_V_DUTY_PCT; g_egtmAtomTmadc.phases[1] = 0.0f;
    g_egtmAtomTmadc.dutyCycles[2] = PHASE_W_DUTY_PCT; g_egtmAtomTmadc.phases[2] = 0.0f;

    g_egtmAtomTmadc.deadTimes[0].rising = PWM_DEAD_TIME_SEC; g_egtmAtomTmadc.deadTimes[0].falling = PWM_DEAD_TIME_SEC;
    g_egtmAtomTmadc.deadTimes[1].rising = PWM_DEAD_TIME_SEC; g_egtmAtomTmadc.deadTimes[1].falling = PWM_DEAD_TIME_SEC;
    g_egtmAtomTmadc.deadTimes[2].rising = PWM_DEAD_TIME_SEC; g_egtmAtomTmadc.deadTimes[2].falling = PWM_DEAD_TIME_SEC;
}
