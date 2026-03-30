/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready EGTM/ATOM 3-phase complementary, center-aligned PWM inverter driver
 * Target: TC4xx (e.g., TC4D7) using IfxEgtm_Pwm high-level driver
 * Notes:
 *  - No watchdog changes here (must be done only in CpuX main files as per AURIX standard)
 *  - Pins use validated symbols when available; placeholders are provided for non-listed pins
 */
#include "egtm_atom_3_phase_inverter_pwm.h"

/* Required dependencies (as per module design) */
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =========================================================================================
 * Configuration Macros (constants from requirements)
 * ========================================================================================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED pin (compound form: port, pin) - toggle on period ISR */
#define LED                      &MODULE_P03, 9

/* =========================================================================================
 * Pin Routing Macros (validated symbols or placeholders)
 * User-requested mapping (KIT_A3G_TC4D7_LITE):
 *  U_HS=P20.8 (TOUT64), U_LS=P20.9 (TOUT65)
 *  V_HS=P21.4 (TOUT55), V_LS=P20.11 (TOUT67)
 *  W_HS=P20.12 (TOUT68), W_LS=P20.13 (TOUT69)
 *
 * Use ONLY symbols from the validated list. For non-listed pins, provide NULL_PTR placeholder
 * to be replaced during integration with the correct IfxEgtm pin-map symbol.
 * ========================================================================================= */
#define PHASE_U_HS   (NULL_PTR)                        /* Replace with &IfxEgtm_ATOM0_x_TOUT64_P20_8_OUT */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS   (NULL_PTR)                        /* Replace with &IfxEgtm_ATOM0_x_TOUT55_P21_4_OUT */
#define PHASE_V_LS   (NULL_PTR)                        /* Replace with &IfxEgtm_ATOM0_xN_TOUT67_P20_11_OUT */
#define PHASE_W_HS   (NULL_PTR)                        /* Replace with &IfxEgtm_ATOM0_x_TOUT68_P20_12_OUT */
#define PHASE_W_LS   (NULL_PTR)                        /* Replace with &IfxEgtm_ATOM0_xN_TOUT69_P20_13_OUT */

/* =========================================================================================
 * Module State (persistent) - must use IFX_STATIC
 * ========================================================================================= */
typedef struct
{
    IfxEgtm_Pwm           pwm;                                  /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];            /* Persistent channel data */
    float32               dutyCycles[NUM_OF_CHANNELS];           /* Duty in percent */
    float32               phases[NUM_OF_CHANNELS];               /* Phase in [0..1) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];           /* Dead-times per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =========================================================================================
 * Private ISR and callback (declared before init)
 * ========================================================================================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty: driver routes interrupt; ISR toggles LED */
}

/* =========================================================================================
 * Public API: initEgtmAtom3phInv
 * ========================================================================================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxEgtm_Pwm_Config           config;                                  /* Main config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];          /* Per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];              /* Per-channel DTM */
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;                         /* Interrupt config */
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];                 /* Output routing */

    /* 2) Initialize main config with driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Fill OutputConfig for each logical channel (0..2): HS active-high, LS active-low */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Set DTM dead-time per channel: 1 us rising/falling */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure interrupt parameters: pulse notify, CPU0, priority=20, period callback set, duty=NULL */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = &IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Configure three logical PWM channels: indices 0,1,2 with U/V/W duties respectively */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* Interrupt only on base channel */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main config: ATOM in Cluster0, center-aligned, 3 channels, sync start/update, 20 kHz, clocks */
    config.cluster            = IfxEgtm_Cluster_0;                       /* ATOM0 Cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* Use ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* Center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;                           /* Requested PWM frequency */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* ATOM clock from CMU.CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock from CMU.CLK0 */
    config.syncUpdateEnabled  = TRUE;                                     /* Synchronized updates */
    config.syncStart          = TRUE;                                     /* Synchronized start */

    /* 8) Enable guard: enable eGTM and configure CMU clocks inside the guard */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set global clock to module frequency (no division) and ATOM CLK0 */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        /* Also program dividers explicitly (unity) and ECLK for DTM timing */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
        /* Enable FXCLK and CLK0 as required (FXCLK for TOM, CLK0 for ATOM/DTM) */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM: applies duties, phasing, alignment, sync, routing and DTM dead-time */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store persistent state for later updates */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO pin used by ISR as output (no explicit set-high/low) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =========================================================================================
 * Public API: updateEgtmAtom3phInvDuty
 * ========================================================================================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Step and wrap duties exactly as specified */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties atomically to all channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)&g_egtmAtom3phInv.dutyCycles[0]);
}
