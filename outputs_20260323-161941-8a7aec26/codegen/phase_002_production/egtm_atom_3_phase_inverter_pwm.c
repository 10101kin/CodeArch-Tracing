/*
 * egtm_atom_3_phase_inverter_pwm.c
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"
#include "IfxEgtm_Pwm.h"

/* =========================
 * Module-local state
 * ========================= */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;  /* static driver instance */
IFX_STATIC boolean        s_initialized = FALSE;

/* =========================
 * ISR (diagnostic toggle)
 * ========================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =========================
 * Local helpers
 * ========================= */
static float32 clampDutyToMinPulse(float32 duty)
{
    /* Duty expressed in percent [0..100]. Clamp to enforce min pulse both sides. */
    if (duty < MIN_DUTY_PERCENT)
    {
        duty = MIN_DUTY_PERCENT;
    }
    else if (duty > MAX_DUTY_PERCENT)
    {
        duty = MAX_DUTY_PERCENT;
    }
    return duty;
}

/* =========================
 * Public API implementation
 * ========================= */
void initEgtmAtom3phInv(void)
{
    /*
     * STEP 1: eGTM module enable + CMU clock setup (must be before PWM init)
     */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Configure GCLK and select CLK0 from global, then enable CLK0 for ATOM */
    {
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Drive GCLK to module frequency (no divide) */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        /* Optional explicit divider = 1/1 (kept for clarity) */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        /* Route ATOM CLK0 from GCLK */
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
        /* Enable CLK0 domain */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)IFXEGTM_CMU_CLKEN_CLK0);
    }

    /*
     * STEP 2: Prepare unified PWM configuration structures
     */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output pin and complementary pin assignment; polarity active-high for both (per requirements) */
    /* Phase U */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_high;  /* requirement: low-side active-high */
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_high;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_high;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time attach via DTM configuration (hardware) */
    {
        const float32 deadTimeSeconds = (TIMING_DEADTIME_US * 1.0e-6f);
        for (uint32 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            /* Unified driver DTM configuration uses single value; driver applies symmetrically */
            dtmConfig[i].deadTime = deadTimeSeconds;
        }
    }

    /*
     * STEP 3: Channel configuration (center-aligned, synchronized start/update via config flags)
     */
    for (uint32 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].timerCh   = (IfxEgtm_Pwm_SubModule_Ch)i;   /* Use CH0, CH1, CH2 for three phase-pairs */
        channelConfig[i].phase     = 0.0f;                          /* No phase shift at init */
        channelConfig[i].duty      = (i == 0u) ? DUTY_25_PERCENT : ((i == 1u) ? DUTY_50_PERCENT : DUTY_75_PERCENT);
        channelConfig[i].dtm       = &dtmConfig[i];
        channelConfig[i].output    = &output[i];
        channelConfig[i].mscOut    = NULL_PTR;
        channelConfig[i].interrupt = NULL_PTR;                      /* No ISR bound via driver in this init */
    }

    config.cluster            = IfxEgtm_Cluster_0;                       /* ATOM cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* Use ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* Center-aligned */
    config.syncStart          = TRUE;                                    /* Auto-start after init */
    config.syncUpdateEnabled  = TRUE;                                    /* Shadow update */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = TIMING_PWM_FREQUENCY_HZ;                 /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                       /* ATOM clock */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock */

    /* STEP 4: Initialize PWM driver (single call reads all config structures) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* STEP 5: Snapshot config values for runtime; set LED pin for ISR toggle */
    for (uint32 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = channelConfig[i].duty;         /* percent */
        g_egtmAtom3phInv.phases[i]     = channelConfig[i].phase;        /* degrees */
        g_egtmAtom3phInv.deadTimes[i]  = dtmConfig[i].deadTime;         /* structure-compatible */
    }

    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

void updateEgtmAtom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init did not run */
    }

    /* Compute clamped ramped duties (percent) */
    for (uint32 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_egtmAtom3phInv.dutyCycles[i] + DUTY_STEP;  /* ramp up */
        if (d > MAX_DUTY_PERCENT)
        {
            d = MIN_DUTY_PERCENT;  /* wrap to min if exceeded */
        }
        d = clampDutyToMinPulse(d);
        g_egtmAtom3phInv.dutyCycles[i] = d;
    }

    /* Apply updated duties synchronously (driver handles shadow update at period) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
