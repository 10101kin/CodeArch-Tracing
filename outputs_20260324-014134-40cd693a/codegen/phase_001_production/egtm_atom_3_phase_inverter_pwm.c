/*****************************************************************************
 * File:    egtm_atom_3_phase_inverter_pwm.c
 * Brief:   TC4xx eGTM ATOM 3-Phase inverter PWM driver (unified IfxEgtm_Pwm)
 * Author:  Generated
 *****************************************************************************/
#include "egtm_atom_3_phase_inverter_pwm.h"

/* ============================ Module-private state ========================== */
static EgtmAtom3phInv g_egtmAtom3phInv = {0};
EgtmAtom3phInv_Status g_egtmAtom3phInv_status = {0};

/* External duty request buffer: fractional [0.0 .. 1.0] */
float32 g_egtmAtom3phInv_requestedDuty[NUM_OF_CHANNELS] = {0.0f, 0.0f, 0.0f};

/* ============================ ISR (period event) ============================ */
/* ISR priority macro name must be ISR_PRIORITY_ATOM per reference */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Toggle LED on each PWM period event */
    IfxPort_togglePin(LED);
}

/* ============================ Period event callback ========================= */
/* Unified driver callback prototype per reference pattern */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Optional SW hook on period event; not used (ISR toggles LED) */
    (void)data;
}

/* ============================ Initialization ================================ */
void IfxEgtm_Atom_Pwm_init(void)
{
    /* Local configuration containers */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* eGTM module enable + CMU clock setup (MUST be before init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set GCLK to module clock and enable CLK0 at required 100 MHz */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        /* Set CMU CLK0 frequency explicitly per requirement */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, CLOCK_TARGET_FXCLK0_HZ);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Verify FX clock is enabled (status flag) */
    g_egtmAtom3phInv_status.clock_ok = IfxEgtm_Cmu_isFxClockEnabled(&MODULE_EGTM);
    if (g_egtmAtom3phInv_status.clock_ok == FALSE)
    {
        g_egtmAtom3phInv_status.error_clock = TRUE;
        /* Continue init; higher layers may decide how to react */
    }

    /* Initialize unified PWM config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output pin configuration (complementary) via OutputConfig array */
    /* Phase U (CH0) */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (CH1) */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (CH2) */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (CDTM) per channel */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SEC;
    g_egtmAtom3phInv_status.dtm_configured = TRUE;

    /* Interrupt configuration (period event on base channel) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations: ATOM0 CH0/1/2, center-aligned, initial 50% */
    /* CH0 - Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 50.0f;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Period ISR on base channel */

    /* CH1 - Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 50.0f;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 - Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 50.0f;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Top-level PWM configuration */
    config.cluster              = IfxEgtm_Cluster_1;                 /* eGTM Cluster 1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;        /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart            = TRUE;                              /* Start synchronously */
    config.syncUpdateEnabled    = TRUE;                              /* Shadow updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                 /* Use CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock */

    /* Initialize unified PWM driver (no return value) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store runtime copies for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* LED pin configuration for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Install ISR handler (service request priority matches ISR_PRIORITY_ATOM) */
    IfxCpu_Irq_installInterruptHandler((void *)interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    g_egtmAtom3phInv_status.pwm_initialized = TRUE;
}

/* ============================ Runtime duty update =========================== */
void IfxEgtm_Atom_Pwm_setDutyCycle(void)
{
    float32 dutyPercent[NUM_OF_CHANNELS];
    uint8   i;

    g_egtmAtom3phInv_status.update_in_progress = TRUE;
    g_egtmAtom3phInv_status.error_init = (g_egtmAtom3phInv_status.pwm_initialized == FALSE) ? TRUE : g_egtmAtom3phInv_status.error_init;

    /* 1) Clamp requested fractional duty to [0.0 .. 1.0] and convert to percent */
    for (i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_egtmAtom3phInv_requestedDuty[i];
        if (d < 0.0f)
        {
            d = 0.0f;
            g_egtmAtom3phInv_status.error_pinmap = g_egtmAtom3phInv_status.error_pinmap; /* keep flags independent */
        }
        else if (d > 1.0f)
        {
            d = 1.0f;
            /* Use error_clock flag to indicate out-of-range if needed; keep separate semantic errors minimal */
        }
        dutyPercent[i] = d * 100.0f; /* Unified driver expects percent */
        g_egtmAtom3phInv.dutyCycles[i] = dutyPercent[i];
    }

    /* 2) Write all three duties using synchronized update; takes effect at next period */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)dutyPercent);

    g_egtmAtom3phInv_status.update_in_progress = FALSE;
}
