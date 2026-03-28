/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for eGTM ATOM 3-phase inverter PWM on TC4xx.
 *
 * Requirements implemented:
 * - Unified IfxEgtm_Pwm high-level driver
 * - 3 complementary, center-aligned symmetric PWMs at 20 kHz
 * - Dead-time = 0.5 us, Min Pulse = 1.0 us via DTM
 * - Active-high polarity for HS, active-low for LS
 * - Sync start + sync updates
 * - eGTM CMU clocking from GCLK without extra prescaling
 * - Persistent channels array in module state
 * - ISR + empty period callback per template
 * - No watchdog handling here (must be in CpuX_Main.c only)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Macros and Constants ========================= */

/* Channel count */
#define EGTM3PH_NUM_CHANNELS            (3u)

/* PWM timing (20 kHz) */
#define EGTM3PH_PWM_FREQUENCY_HZ        (20000.0f)

/* Dead-time and min pulse (seconds) */
#define EGTM3PH_DEADTIME_S              (0.5e-6f)  /* 0.5 microseconds */
#define EGTM3PH_MINPULSE_S              (1.0e-6f)  /* 1.0 microseconds */

/* Initial duty (percent) */
#define EGTM3PH_DUTY_U_INIT             (25.0f)
#define EGTM3PH_DUTY_V_INIT             (50.0f)
#define EGTM3PH_DUTY_W_INIT             (75.0f)

/* Duty step (percent) for updates */
#define EGTM3PH_DUTY_STEP               (5.0f)

/* Interrupt priority for ATOM PWM (used for both config and IFX_INTERRUPT) */
#define ISR_PRIORITY_ATOM               (10)

/* Debug LED (Port, Pin) in a compound macro form */
#define LED                             &MODULE_P13, 0

/* ========================= Pin Mapping (User-requested) ========================= */
/*
 * NOTE: The following pin symbols must exist in the installed iLLD PinMap headers.
 * They follow the user-requested Port/Pin to TOUT mapping for KIT_A3G_TC4D7_LITE.
 *
 * U_H: P02.0 -> IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
 * U_L: P02.1 -> IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
 * V_H: P02.2 -> IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT
 * V_L: P02.3 -> IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT
 * W_H: P02.4 -> IfxEgtm_ATOM0_4_TOUT4_P02_4_OUT
 * W_L: P02.5 -> IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT
 */
#define PHASE_U_HS                      &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS                      &IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
#define PHASE_V_HS                      &IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT
#define PHASE_V_LS                      &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT
#define PHASE_W_HS                      &IfxEgtm_ATOM0_4_TOUT4_P02_4_OUT
#define PHASE_W_LS                      &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT

/* ========================= Module State ========================= */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                       /* Driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM3PH_NUM_CHANNELS];            /* Persistent channels array */
    float32                   dutyCycles[EGTM3PH_NUM_CHANNELS];          /* Duty in percent for U,V,W */
    float32                   phases[EGTM3PH_NUM_CHANNELS];              /* Phase offsets in degrees or fraction (unused = 0) */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM3PH_NUM_CHANNELS];           /* Stored dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtm3ph = {0};

/* ========================= ISR and Callback ========================= */

/* ISR: toggle LED only (priority via ISR_PRIORITY_ATOM). */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback: must be empty body as per template. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API ========================= */

void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration objects (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM3PH_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM3PH_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM3PH_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load defaults for main config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration for complementary pairs (polarity per template) */
    output[0].pin                    = PHASE_U_HS;  /* U high-side */
    output[0].complementaryPin      = PHASE_U_LS;  /* U low-side  */
    output[0].polarity              = Ifx_ActiveState_high; /* HS active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;  /* LS active LOW  */

    output[1].pin                    = PHASE_V_HS;  /* V high-side */
    output[1].complementaryPin      = PHASE_V_LS;  /* V low-side  */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;

    output[2].pin                    = PHASE_W_HS;  /* W high-side */
    output[2].complementaryPin      = PHASE_W_LS;  /* W low-side  */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;

    /* 4) DTM configuration per channel (dead-time + min pulse) */
    dtmConfig[0].deadTime.rise  = EGTM3PH_DEADTIME_S;
    dtmConfig[0].deadTime.fall  = EGTM3PH_DEADTIME_S;
    dtmConfig[0].minPulse       = EGTM3PH_MINPULSE_S;
    dtmConfig[0].fastShutOff    = NULL_PTR;

    dtmConfig[1].deadTime.rise  = EGTM3PH_DEADTIME_S;
    dtmConfig[1].deadTime.fall  = EGTM3PH_DEADTIME_S;
    dtmConfig[1].minPulse       = EGTM3PH_MINPULSE_S;
    dtmConfig[1].fastShutOff    = NULL_PTR;

    dtmConfig[2].deadTime.rise  = EGTM3PH_DEADTIME_S;
    dtmConfig[2].deadTime.fall  = EGTM3PH_DEADTIME_S;
    dtmConfig[2].minPulse       = EGTM3PH_MINPULSE_S;
    dtmConfig[2].fastShutOff    = NULL_PTR;

    /* 5) Interrupt configuration (single period ISR via channel 0) */
    irqCfg.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = &IfxEgtm_periodEventFunction; /* empty callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration (logical indices 0..2 on same ATOM cluster) */
    /* U phase */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = EGTM3PH_DUTY_U_INIT; /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;            /* hook ISR via ch0 */

    /* V phase */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = EGTM3PH_DUTY_V_INIT; /* percent */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;           /* single shared ISR via ch0 */

    /* W phase */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = EGTM3PH_DUTY_W_INIT; /* percent */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main config fields (alignment, clocking, sync) */
    config.egtmSFR           = &MODULE_EGTM;
    config.cluster           = IfxEgtm_Cluster_0;                      /* same ATOM cluster */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;             /* ATOM submodule */
    config.alignment         = IfxEgtm_Pwm_Alignment_centerAlignedSymmetric;
    config.numChannels       = (uint8)EGTM3PH_NUM_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = EGTM3PH_PWM_FREQUENCY_HZ;               /* 20 kHz */
    config.clockSource.atom  = IfxEgtm_Pwm_ClockSource_cmuFxclk0;      /* Fxclk0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* DTM clock */
    config.syncUpdateEnabled = TRUE;                                    /* shadow to active at period end */
    config.syncStart         = TRUE;                                    /* synchronized start on init */

    /* 8) eGTM enable-guard + CMU clock setup from module frequency (no hardcoding) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);                                 /* 1:1 divider to GCLK */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);               /* CLK0 from GCLK */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver with persistent channels array in module state */
    IfxEgtm_Pwm_init(&g_egtm3ph.pwm, &g_egtm3ph.channels[0], &config);

    /* 10) Store initial state for runtime updates */
    g_egtm3ph.dutyCycles[0] = EGTM3PH_DUTY_U_INIT;
    g_egtm3ph.dutyCycles[1] = EGTM3PH_DUTY_V_INIT;
    g_egtm3ph.dutyCycles[2] = EGTM3PH_DUTY_W_INIT;

    g_egtm3ph.phases[0] = channelConfig[0].phase;
    g_egtm3ph.phases[1] = channelConfig[1].phase;
    g_egtm3ph.phases[2] = channelConfig[2].phase;

    g_egtm3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtm3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtm3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Optional: configure diagnostic LED GPIO as push-pull output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Duty wrap rule (percent domain 0..100): check->maybe reset->unconditional add */
    if ((g_egtm3ph.dutyCycles[0] + EGTM3PH_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[0] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[1] + EGTM3PH_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[1] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[2] + EGTM3PH_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[2] = 0.0f; }

    g_egtm3ph.dutyCycles[0] += EGTM3PH_DUTY_STEP;
    g_egtm3ph.dutyCycles[1] += EGTM3PH_DUTY_STEP;
    g_egtm3ph.dutyCycles[2] += EGTM3PH_DUTY_STEP;

    /* Apply updated duties immediately; change takes effect synchronously at next period boundary */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtm3ph.pwm, (float32 *)g_egtm3ph.dutyCycles);
}
