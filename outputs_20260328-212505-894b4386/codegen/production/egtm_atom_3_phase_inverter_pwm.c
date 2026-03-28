/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief eGTM ATOM 3-phase inverter PWM driver (TC4xx) using unified IfxEgtm_Pwm
 *
 * - Generates six complementary, center-aligned PWM outputs at 20 kHz
 * - Dead-time: rising=1e-6 s, falling=1e-6 s
 * - Initial duties: U=25%, V=50%, W=75%
 * - syncStart and syncUpdate enabled
 * - HS active-high, LS active-low; push-pull, cmosAutomotiveSpeed1
 * - Interrupt: pulse notify on base channel; ISR toggles a debug LED
 *
 * Notes:
 * - Pin routing is done via IfxEgtm_Pwm_OutputConfig (no PinMap_set calls)
 * - GTM/eGTM CMU clocks are enabled using the documented guard pattern
 * - No watchdog API calls here (must be in CpuX main only)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and configuration constants ========================= */
#define NUM_OF_CHANNELS              (3u)
#define PWM_FREQUENCY_HZ             (20000.0f)    /* 20 kHz */
#define ISR_PRIORITY_ATOM            (20)

/* Initial duties in percent */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)

/* Duty update step in percent for ramping */
#define PHASE_DUTY_STEP              (5.0f)

/* Debug LED: compound macro for port/pin */
#define LED                          &MODULE_P13, 0u

/* ========================= Validated pin selection ========================= */
/*
 * Use ONLY validated pin symbols. For KIT_A3G_TC4D7_LITE, P02.0/P02.1 are validated below.
 * For remaining phases (V/W), replace NULL_PTR with validated symbols during board integration.
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)    /* High-side: ATOM0 CH0 on P02.0 */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)   /* Low-side (complementary): ATOM0 CH0N on P02.1 */

#define PHASE_V_HS   (NULL_PTR) /* Replace with validated eGTM ATOM pin, e.g., &IfxEgtm_ATOMx_y_TOUTz_Pxx_y_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with validated eGTM ATOM complementary pin, e.g., &IfxEgtm_ATOMx_yN_TOUTz_Pxx_y_OUT */

#define PHASE_W_HS   (NULL_PTR) /* Replace with validated eGTM ATOM pin, e.g., &IfxEgtm_ATOMx_y_TOUTz_Pxx_y_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with validated eGTM ATOM complementary pin, e.g., &IfxEgtm_ATOMx_yN_TOUTz_Pxx_y_OUT */

/* ========================= Module state ========================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                                /* unified driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];          /* persistent channel state (driver writes into it) */
    float32                  dutyCycles[NUM_OF_CHANNELS];        /* percent: 0..100 */
    float32                  phases[NUM_OF_CHANNELS];            /* phase offsets (deg or fraction as driver expects); here 0.0 */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];         /* stored copy of configured dead-times */
} EgtmAtom3phInv_State;

/* IFX_STATIC required by architecture pattern */
IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and callback declarations ========================= */
/* Interrupt declaration with required naming and priority. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period-event callback (assigned in interrupt config). Must have empty body. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR toggles the debug LED only. No additional processing. */
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API implementations ========================= */

/**
 * @brief Initialize 3-phase inverter PWM (eGTM ATOM) using unified IfxEgtm_Pwm driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config           config;                                 /* main config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];         /* per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];             /* per-channel DTM */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;                                  /* one interrupt config for base channel */
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];                 /* per-channel output pins */

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output pin configuration for three logical channels (U, V, W) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;    /* LS active low  */
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

    /* 4) Dead-time configuration (1e-6 s rising and falling for all three channels) */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (pulse notify, CPU0, priority, period callback) */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration (timerCh ordinal indices 0..2, phase=0.0) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;       /* base channel gets interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;      /* no separate interrupt */

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;      /* no separate interrupt */

    /* 7) Main PWM configuration fields */
    config.cluster              = IfxEgtm_Cluster_0;                 /* Match ATOM0 pins used above */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;        /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;      /* center-aligned */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.atom     = (uint32)IfxEgtm_Cmu_Fxclk_0;       /* FXCLK0 as source */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock from CMU CLK0 */
    config.syncUpdateEnabled    = TRUE;                              /* latch updates on period */
    config.syncStart            = TRUE;                              /* start channels in sync */

    /* 8) Enable guard: enable and configure eGTM CMU clocks if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamic module frequency read, set GCLK and CLK0 to the module frequency */
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        /* Divider configuration (1:1) as per migration guideline */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
        /* Enable FXCLK and CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM (driver writes into persistent channels array) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    /* 11) Configure debug LED pin as output (ISR will toggle it) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * @brief Update (ramp) three-phase PWM duty cycles and apply immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap rule: if d+step >= 100 then reset to 0, then always add step */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties atomically on driver's immediate update */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
