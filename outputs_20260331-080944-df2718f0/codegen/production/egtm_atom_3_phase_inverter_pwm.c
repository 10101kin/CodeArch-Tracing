/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver for TC4xx EGTM ATOM 3-Phase Inverter PWM using IfxEgtm_Pwm
 *
 * Notes:
 * - No watchdog handling here (must be in CpuX_Main.c only)
 * - Uses authoritative iLLD initialization patterns and EGTM CMU enable guard
 * - Interrupt callback and ISR are minimal and separate by design
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* =========================================
 * Numeric configuration macros (user-confirmed)
 * ========================================= */
#define NUM_PWM_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ            (20000.0f)
#define ISR_PRIORITY_ATOM           (20)
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)
#define PHASE_DUTY_STEP             (10.0f)
#define WAIT_TIME_MS                (500.0f)

/* =========================================
 * LED (P03.9), active-low, push-pull, cmosAutomotiveSpeed1
 * Compound macro used with IfxPort_ API: IfxPort_togglePin(LED);
 * ========================================= */
#define LED                         &MODULE_P03, 9

/* =========================================
 * Complementary PWM outputs
 * Use ONLY validated pin symbols; for unavailable ones use NULL_PTR placeholders.
 * Validated examples available for ATOM0 channel 0 normal and 0N complementary on P02.x.
 * Replace NULL_PTR with proper &IfxEgtm_ATOM0_x[_N]_TOUTy_Pzz_k_OUT for your board routing.
 * ========================================= */
#define PHASE_U_HS                  (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                  (&IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT*_P**_**_OUT */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT*_P**_**_OUT */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT*_P**_**_OUT */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT*_P**_**_OUT */

/* =========================================
 * Module persistent state (IFX_STATIC as required)
 * ========================================= */
typedef struct
{
    IfxEgtm_Pwm               pwm;                                     /* driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_PWM_CHANNELS];              /* persistent channel handles */
    float32                   dutyCycles[NUM_PWM_CHANNELS];            /* duty in percent */
    float32                   phases[NUM_PWM_CHANNELS];                /* phase in percent/deg eq (driver expects fraction or degrees per API) */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_PWM_CHANNELS];             /* rising/falling dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =========================================
 * Period-event callback (empty body, routed by driver)
 * Must be visible and match signature: void IfxEgtm_periodEventFunction(void *data)
 * ========================================= */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================
 * ISR: hardware interrupt handler (priority = ISR_PRIORITY_ATOM, CPU0)
 * Body MUST be exactly: toggle LED pin. Do not call driver ISR handlers here.
 * ========================================= */
IFX_INTERRUPT(EgtmAtomIsr, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr(void)
{
    IfxPort_togglePin(LED);
}

/* =========================================
 * Internal helpers (local)
 * ========================================= */
static void egtmCmuEnableGuard(void)
{
    /* Enable EGTM and configure CMU only if not already enabled */
    if (FALSE == IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        /* 1) Enable EGTM module */
        IfxEgtm_enable(&MODULE_EGTM);

        /* 2) Read module frequency dynamically */
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);

        /* 3) Set GCLK divider to unity (derive GCLK directly from module clock) */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);

        /* 4) Program CLK0 to module frequency so functional clock domain is available */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, moduleFreq);

        /* 5) Enable required EGTM clocks (FXCLK and CLK0) */
        /* Note: Use device-provided CLKEN mask macros; placeholders used if not defined */
#ifndef IFXEGTM_CMU_CLKEN_FXCLK
#   define IFXEGTM_CMU_CLKEN_FXCLK   (0x1u)
#endif
#ifndef IFXEGTM_CMU_CLKEN_CLK0
#   define IFXEGTM_CMU_CLKEN_CLK0    (0x2u)
#endif
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }
}

/* =========================================
 * Public API implementation
 * ========================================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare ALL configuration structures as locals */
    IfxEgtm_Pwm_Config           config;                                    /* main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_PWM_CHANNELS];           /* per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_PWM_CHANNELS];               /* per-channel DTM */
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;                           /* period event on base channel */
    IfxEgtm_Pwm_OutputConfig     output[NUM_PWM_CHANNELS];                  /* pin routing per channel */

    /* 2) Initialize main config with defaults for EGTM */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration: complementary pairs, HS active-high, LS active-low */
    /* Phase U (logical channel 0) */
    output[0].pin                   = PHASE_U_HS;                            /* high-side pin */
    output[0].complementaryPin      = PHASE_U_LS;                            /* low-side pin  */
    output[0].polarity              = Ifx_ActiveState_high;                  /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;                   /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (logical channel 1) */
    output[1].pin                   = PHASE_V_HS;                            /* replace NULL_PTR if routing is known */
    output[1].complementaryPin      = PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (logical channel 2) */
    output[2].pin                   = PHASE_W_HS;                            /* replace NULL_PTR if routing is known */
    output[2].complementaryPin      = PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1e-6 s both edges for each channel */
    dtmConfig[0].deadTime.rising    = 1e-6f;
    dtmConfig[0].deadTime.falling   = 1e-6f;
    dtmConfig[0].fastShutOff        = NULL_PTR;

    dtmConfig[1].deadTime.rising    = 1e-6f;
    dtmConfig[1].deadTime.falling   = 1e-6f;
    dtmConfig[1].fastShutOff        = NULL_PTR;

    dtmConfig[2].deadTime.rising    = 1e-6f;
    dtmConfig[2].deadTime.falling   = 1e-6f;
    dtmConfig[2].fastShutOff        = NULL_PTR;

    /* 5) Interrupt configuration: period event on base channel using pulse-notify */
    interruptConfig.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: contiguous logical indices starting from SubModule_Ch_0 */
    /* Base channel (0): with interrupt */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    /* Channel 1 */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;      /* only base channel has interrupt */

    /* Channel 2 */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;      /* only base channel has interrupt */

    /* 7) Main PWM configuration fields */
    config.cluster             = IfxEgtm_Cluster_0;                          /* target cluster 0 */
    config.subModule           = IfxEgtm_SubModule_atom;                     /* ATOM submodule */
    config.alignment           = IfxEgtm_Pwm_Alignment_centerAligned;        /* center-aligned */
    config.syncStart           = TRUE;                                       /* start all together */
    config.syncUpdateEnabled   = TRUE;                                       /* update at period end */
    config.frequency           = PWM_FREQUENCY_HZ;                           /* 20 kHz */
    config.numChannels         = NUM_PWM_CHANNELS;                           /* 3 channels */
    config.channels            = channelConfig;                              /* per-channel configuration array */

    /* Clock sources: ATOM from FXCLK0; DTM from first clock source */
    config.clockSource.atom    = IfxEgtm_Atom_Ch_ClkSrc_cmuFxclk0;
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Optional: provide persistent channel handle storage to the driver */
    /* Some iLLD variants require passing handle storage; keep for compatibility */
    config.channelHandles      = g_egtmAtom3phInv.channels;

    /* 8) EGTM CMU enable guard and clocks setup (inside guard only) */
    egtmCmuEnableGuard();

    /* 9) Initialize unified PWM driver once with persistent channels storage */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &config);

    /* 10) Store initial duty and dead-time values into module state for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinMode(LED, IfxPort_OutputMode_pushPull);
    IfxPort_setPinPadDriver(LED, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
