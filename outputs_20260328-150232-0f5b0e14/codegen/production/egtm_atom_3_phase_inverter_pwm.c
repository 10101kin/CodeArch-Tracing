/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: eGTM ATOM 3-Phase Inverter PWM using IfxEgtm_Pwm
 * Targets: AURIX TC4xx
 * Notes:
 *  - Uses center-aligned symmetric PWM at 20 kHz
 *  - Complementary outputs with DTM dead-time (0.5 us) and min pulse (1.0 us)
 *  - Initial duties: U=25%, V=50%, W=75%
 *  - Clocks: Fxclk0 from GCLK (1:1) with CMU enable guard
 *  - ISR toggles a diagnostic LED; period callback is empty (assigned to driver)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)

/* Dead-time and minimum pulse (requirements specify microseconds) */
#define PWM_DEADTIME_US                (0.5f)
#define PWM_MIN_PULSE_US               (1.0f)
#define PWM_DEADTIME_S                 (PWM_DEADTIME_US * 1.0e-6f)
#define PWM_MIN_PULSE_S                (PWM_MIN_PULSE_US * 1.0e-6f)

/* Initial duty (percent) for U/V/W */
#define PHASE_U_DUTY_INIT_PERCENT      (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT      (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT      (75.0f)

/* Duty step (percent) for runtime update */
#define PHASE_DUTY_STEP                (5.0f)

/* Diagnostic LED: P13.0 (compound macro: port, pin) */
#define LED                            &MODULE_P13, 0

/* Interrupt priority for ATOM PWM */
#define ISR_PRIORITY_ATOM              (10)

/* ========================= Pin Routing Macros (validated pin symbols only) ========================= */
/*
 * Note: Final board-specific pinout to be confirmed. Placeholder selections use validated symbols.
 * U phase uses ATOM0 ch0 pair on P02.0 (HS) and P02.1 (LS = complementary N)
 * V and W placeholder pins are assigned from the validated list; adjust to target hardware design.
 */
#define PHASE_U_HS   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM0_0N_TOUT81_P14_1_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM0_0N_TOUT97_P16_1_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM0_0N_TOUT112_P01_4_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM0_0N_TOUT113_P01_5_OUT

/* ========================= Module State ========================= */
/* Persistent driver state; channels[] must persist for the driver lifetime */
typedef struct
{
    IfxEgtm_Pwm                pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel        channels[NUM_OF_CHANNELS];         /* driver-owned channel objects (persistent) */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* percent 0..100 */
    float32                    phases[NUM_OF_CHANNELS];           /* phase offset placeholder (not used at runtime) */
    IfxEgtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];        /* stored dead-time configuration */
} EgtmAtom3ph_StateT;

IFX_STATIC EgtmAtom3ph_StateT g_egtmAtom3phState;

/* ========================= ISR and Period Callback (must appear before init) ========================= */
/* ISR: toggle diagnostic LED only */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback: must be present and empty */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API: Initialization ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 2) Output/pin configuration for three complementary pairs */
    /* U phase */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */

    /* V phase (placeholder pins; adjust per hardware) */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;

    /* W phase (placeholder pins; adjust per hardware) */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;

    /* 3) DTM configuration per channel (dead-time and minimum pulse) */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; ++i)
    {
        dtmConfig[i].deadTime.rising  = PWM_DEADTIME_S;
        dtmConfig[i].deadTime.falling = PWM_DEADTIME_S;
        dtmConfig[i].minPulse         = PWM_MIN_PULSE_S;
        dtmConfig[i].fastShutOff      = NULL_PTR;   /* Not used */
    }

    /* 4) Interrupt configuration (period notification to ISR via driver routing) */
    irqCfg.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = &IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 5) Channel configurations: indices 0..2 (contiguous logical channels on same ATOM cluster) */
    /* U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT_PERCENT;  /* percent */
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;

    /* V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT_PERCENT;  /* percent */
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = &irqCfg;

    /* W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT_PERCENT;  /* percent */
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = &irqCfg;

    /* 6) Main config fields */
    config.egtmSFR            = &MODULE_EGTM;
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_centerAligned; /* symmetric center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource        = IfxEgtm_Pwm_ClockSource_cmuFxclk0;   /* ATOM clock source */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock source */
    config.syncUpdateEnabled  = TRUE;                                 /* shadow transfer at period end */
    config.syncStart          = TRUE;                                 /* start synchronously after init */

    /* 7) Enable-guard for eGTM + CMU clocks (MANDATORY pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM driver (uses persistent channels array in module state) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwm, g_egtmAtom3phState.channels, &config);

    /* 9) Persist initial duties, phases, and dead-times for runtime updates */
    g_egtmAtom3phState.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_egtmAtom3phState.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_egtmAtom3phState.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    g_egtmAtom3phState.phases[0] = 0.0f;
    g_egtmAtom3phState.phases[1] = 0.0f;
    g_egtmAtom3phState.phases[2] = 0.0f;

    g_egtmAtom3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Optional: configure diagnostic LED (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ========================= Public API: Runtime duty update ========================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Duty wrap rule (percent): reset to 0 if (duty + STEP) >= 100, then always add STEP */
    if ((g_egtmAtom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately with synchronous multi-channel update at next period boundary */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phState.pwm, (float32*)g_egtmAtom3phState.dutyCycles);
}
