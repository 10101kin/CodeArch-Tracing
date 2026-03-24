/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * TC4xx (TC4D7/TC387) eGTM ATOM 3-phase inverter PWM - Production Source
 *
 * Implements:
 *   - void initEgtmAtom3phInv(void)
 *   - void updateEgtmAtom3phInvDuty(void)
 *
 * Follows mandatory unified IfxEgtm_Pwm init pattern and CMU clock enable.
 * Interrupt configuration is bound via channelConfig[0].interrupt.
 *
 * Notes:
 * - Do NOT place any watchdog code here (handled only in CpuN_Main.c)
 * - Do NOT call redundant post-init APIs (startSyncedChannels, startChannelOutputs, etc.)
 * - Pin routing is handled via OutputConfig; do NOT call PinMap_setAtomTout directly
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxCpu_Irq.h"  /* For IFX_INTERRUPT macro */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ========================================================================== */
/* Driver/application state                                                   */
/* ========================================================================== */

typedef struct
{
    IfxEgtm_Pwm          pwm;                         /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];   /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS]; /* Normalized duty fractions [0..1] */
    float32              phases[NUM_OF_CHANNELS];     /* Phase shift fractions [0..1] */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];  /* Dead-time values (seconds) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* Internal driver instance */

/* ========================================================================== */
/* ISR and callbacks                                                          */
/* ========================================================================== */

IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    /* Period event callback - optional user hook. Keep minimal processing. */
    (void)data;
}

/* ========================================================================== */
/* Local helpers                                                              */
/* ========================================================================== */

/* Clamp a normalized duty (fraction [0..1]) to meet the configured min pulse */
static float32 clampDutyToMinPulse(float32 duty)
{
    const float32 period_s   = 1.0f / TIMING_PWM_FREQUENCY_HZ;
    const float32 minPulse_s = TIMING_MIN_PULSE_US * 1.0e-6f;
    const float32 minDuty    = (minPulse_s / period_s);
    const float32 maxDuty    = 1.0f - minDuty;

    /* Wrap to min if exceeded max to create the ramp wrap-around behavior */
    if (duty > maxDuty)
    {
        return minDuty;
    }

    /* Clamp inside [minDuty, maxDuty] */
    if (duty < minDuty)
    {
        return minDuty;
    }
    if (duty > maxDuty)
    {
        return maxDuty;
    }
    return duty;
}

/* ========================================================================== */
/* Public API implementations                                                 */
/* ========================================================================== */

void initEgtmAtom3phInv(void)
{
    /* ---------------------------------------------------------------------- */
    /* 1) Mandatory eGTM enable + CMU clock setup (before PWM init)           */
    /* ---------------------------------------------------------------------- */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Route ATOM CLK0 from GCLK and set GCLK 1:1 divider, then enable CLK0 */
    IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1U, 1U);              /* GCLK = module clk */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE); /* CLK0 from GCLK */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);    /* Enable CLK0 */

    /* ---------------------------------------------------------------------- */
    /* 2) Prepare unified eGTM PWM configuration                              */
    /* ---------------------------------------------------------------------- */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt bound to base channel (period event) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* ---------------------------------------------------------------------- */
    /* 3) Output pin configuration (complementary pairs)                       */
    /*    Pin routing via OutputConfig only (no direct PinMap calls)           */
    /* ---------------------------------------------------------------------- */
    /* Phase U */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high; /* HS active-high */
    output[0].complementaryPolarity    = Ifx_ActiveState_low;  /* LS complementary polarity */
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* ---------------------------------------------------------------------- */
    /* 4) Dead-time config per complementary pair                              */
    /* ---------------------------------------------------------------------- */
    {
        const float32 deadTime_s = TIMING_DEADTIME_US * 1.0e-6f;
        dtmConfig[0].deadTime = deadTime_s;
        dtmConfig[1].deadTime = deadTime_s;
        dtmConfig[2].deadTime = deadTime_s;
    }

    /* ---------------------------------------------------------------------- */
    /* 5) Channel configuration (channel-centric; attach dtm, output, interrupt) */
    /* ---------------------------------------------------------------------- */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].phase     = 0.0f;                  /* no phase shift */
        channelConfig[i].dtm       = &dtmConfig[i];         /* attach dead-time */
        channelConfig[i].output    = &output[i];            /* attach outputs */
        channelConfig[i].mscOut    = NULL_PTR;              /* not used */
        channelConfig[i].interrupt = NULL_PTR;              /* set on [0] below */
    }

    /* Assign specific base channels within ATOM1 for each pair (1,3,5 as example) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_5;

    /* Initial duty commands (percent) */
    channelConfig[0].duty = DUTY_25_PERCENT;
    channelConfig[1].duty = DUTY_50_PERCENT;
    channelConfig[2].duty = DUTY_75_PERCENT;

    /* Attach interrupt only to first channel for period event */
    channelConfig[0].interrupt = &interruptConfig;

    /* ---------------------------------------------------------------------- */
    /* 6) Global PWM configuration                                            */
    /* ---------------------------------------------------------------------- */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;                       /* auto-start after init */
    config.syncUpdateEnabled  = TRUE;                       /* shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;          /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* ---------------------------------------------------------------------- */
    /* 7) Initialize PWM driver (no return value)                             */
    /* ---------------------------------------------------------------------- */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime values (normalized fractions) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty / 100.0f;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty / 100.0f;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty / 100.0f;

    g_egtmAtom3phInv.deadTimes[0].rising  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[0].falling = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1].rising  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[1].falling = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2].rising  = dtmConfig[2].deadTime;
    g_egtmAtom3phInv.deadTimes[2].falling = dtmConfig[2].deadTime;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    /* LED output for ISR diagnostics */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Do NOT call redundant APIs: startSyncedChannels/startChannelOutputs/etc. */
}

void updateEgtmAtom3phInvDuty(void)
{
    /* 1) Increment normalized duty for a ramp */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] += (DUTY_STEP / 100.0f); /* convert step[%] -> fraction */
        g_egtmAtom3phInv.dutyCycles[i]  = clampDutyToMinPulse(g_egtmAtom3phInv.dutyCycles[i]);
    }

    /* 2) Convert to percent for driver API and update synchronously */
    float32 dutiesPercent[NUM_OF_CHANNELS];
    dutiesPercent[0] = g_egtmAtom3phInv.dutyCycles[0] * 100.0f;
    dutiesPercent[1] = g_egtmAtom3phInv.dutyCycles[1] * 100.0f;
    dutiesPercent[2] = g_egtmAtom3phInv.dutyCycles[2] * 100.0f;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &dutiesPercent[0]);
}
