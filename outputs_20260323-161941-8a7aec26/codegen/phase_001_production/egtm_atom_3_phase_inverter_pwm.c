/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 *
 * Follows mandatory unified IfxEgtm_Pwm initialization pattern with
 * OutputConfig, DtmConfig, ChannelConfig arrays and EGTM CMU clock setup.
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxCpu_Irq.h"   /* For IFX_INTERRUPT macro */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/*=============================
 * Driver state (global, internal linkage)
 *=============================*/
typedef struct {
    IfxEgtm_Pwm          pwm;                               /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel runtime handles */
    float32              dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent [0..100] */
    float32              phases[NUM_OF_CHANNELS];            /* Phase shift in percent or deg (unused here) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Deadtime values (driver copy) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* Internal instance */

/*=============================
 * ISR (optional diagnostic toggle)
 *=============================*/
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*=============================
 * Period event callback (optional hook)
 *=============================*/
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/*=============================
 * Local helpers
 *=============================*/
static float32 egtm_computeMinDutyPercent(void)
{
    /* minDuty = min_pulse_time / period = min_pulse_us * 1e-6 * PWM_FREQUENCY */
    const float32 minDutyFraction = (TIMING_MIN_PULSE_US * 1.0e-6f) * (TIMING_PWM_FREQUENCY_HZ);
    float32 minDutyPercent = minDutyFraction * 100.0f;
    /* Ensure sane bounds [0, 50]% to preserve complementary pair timing */
    if (minDutyPercent < 0.0f) { minDutyPercent = 0.0f; }
    if (minDutyPercent > 50.0f) { minDutyPercent = 50.0f; }
    return minDutyPercent;
}

static float32 clampDutyToMinPulse(float32 duty)
{
    const float32 minDuty = egtm_computeMinDutyPercent();
    const float32 maxDuty = 100.0f - minDuty;
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

/*=============================
 * Public API implementations
 *=============================*/
void initEgtmAtom3phInv(void)
{
    /*=========================
     * 0) Mandatory EGTM enable + CMU clock setup
     *=========================*/
    if (!IfxEgtm_isEnabled(&RESOURCE_ASSUMPTIONS_EGTM_MODULE))
    {
        float32 moduleFreq = 0.0f;
        IfxEgtm_enable(&RESOURCE_ASSUMPTIONS_EGTM_MODULE);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&RESOURCE_ASSUMPTIONS_EGTM_MODULE);
        /* Route CLK0 from GCLK and set GCLK = module frequency (divider 1:1) */
        IfxEgtm_Cmu_setGclkFrequency(&RESOURCE_ASSUMPTIONS_EGTM_MODULE, moduleFreq);
        IfxEgtm_Cmu_selectClkInput(&RESOURCE_ASSUMPTIONS_EGTM_MODULE, IfxEgtm_Cmu_Clk_0, TRUE);
        IfxEgtm_Cmu_setGclkDivider(&RESOURCE_ASSUMPTIONS_EGTM_MODULE, 1U, 1U);
        /* Enable CLK0 for ATOM */
        IfxEgtm_Cmu_enableClocks(&RESOURCE_ASSUMPTIONS_EGTM_MODULE, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /*=========================
     * 1) Prepare unified IfxEgtm_Pwm configuration
     *=========================*/
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &RESOURCE_ASSUMPTIONS_EGTM_MODULE);

    /*=========================
     * 2) Interrupt configuration (attach on base channel)
     *=========================*/
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /*=========================
     * 3) Output configuration for complementary pairs (pin + complementaryPin)
     *    Note: Polarity per requirements: both active high
     *=========================*/
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_high;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_high;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_high;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /*=========================
     * 4) Dead-time configuration (hardware DTM) per pair
     *=========================*/
    {
        const float32 deadTimeSec = TIMING_DEADTIME_US * 1.0e-6f;
        for (uint32 i = 0; i < NUM_OF_CHANNELS; i++)
        {
            dtmConfig[i].deadTime = deadTimeSec; /* unified driver deadtime field */
        }
    }

    /*=========================
     * 5) Channel configuration (timerCh, duty, phase, dtm, output, interrupt)
     *=========================*/
    for (uint32 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].timerCh   = (IfxEgtm_Pwm_SubModule_Ch)i;  /* ATOM1 base channels 0..2 (pins route actual TOUTs) */
        channelConfig[i].phase     = 0.0f;                         /* Center aligned, in-phase start */
        channelConfig[i].duty      = (i == 0) ? DUTY_25_PERCENT : ((i == 1) ? DUTY_50_PERCENT : DUTY_75_PERCENT);
        channelConfig[i].dtm       = &dtmConfig[i];
        channelConfig[i].output    = &output[i];
        channelConfig[i].mscOut    = NULL_PTR;
        channelConfig[i].interrupt = (i == 0) ? &interruptConfig : NULL_PTR;
    }

    /*=========================
     * 6) Top-level driver config fields
     *=========================*/
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;                   /* Auto start after init */
    config.syncUpdateEnabled  = TRUE;                   /* Shadow update */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;      /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /*=========================
     * 7) Initialize PWM driver (unified) - no return value
     *=========================*/
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /*=========================
     * 8) Store initial runtime values (for updates)
     *=========================*/
    for (uint32 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = channelConfig[i].duty;
        g_egtmAtom3phInv.phases[i]     = channelConfig[i].phase;
        /* Copy dead-time value for reference */
        g_egtmAtom3phInv.deadTimes[i]  = *channelConfig[i].dtm;
    }

    /*=========================
     * 9) Configure LED pin for ISR toggle (optional)
     *=========================*/
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Note: Do NOT call startSyncedChannels/startChannelOutputs/update during init.
       Unified driver performs start and sync handling via config fields. */
}

void updateEgtmAtom3phInvDuty(void)
{
    /* 1) Increment duty of each complementary pair to create a ramp */
    for (uint32 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_egtmAtom3phInv.dutyCycles[i] + DUTY_STEP;
        const float32 minDuty = egtm_computeMinDutyPercent();
        const float32 maxDuty = 100.0f - minDuty;

        /* 2) Wrap to minDuty if exceeding maxDuty, else clamp within [minDuty, maxDuty] */
        if (next > maxDuty)
        {
            next = minDuty;
        }
        else
        {
            next = clampDutyToMinPulse(next);
        }

        g_egtmAtom3phInv.dutyCycles[i] = next;
    }

    /* 3) Apply updated duties immediately (unified driver will sync at period boundary) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
