/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production-ready eGTM ATOM 3-phase inverter PWM driver (TC4xx - TC387/TC4D7 family).
 *
 * Implements EXACT public API:
 *   void IfxEgtm_Atom_Pwm_init(void);
 *   void IfxEgtm_Atom_Pwm_setDutyCycle(void);
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD - required modules */
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* ==========================================================================
 * Module globals
 * ========================================================================== */
EgtmAtom3phInv g_egtmAtom3phInv = {0};
volatile float32 g_egtmAtom3phInv_requestDuty[NUM_OF_CHANNELS] = {0.0f, 0.0f, 0.0f};

/* ==========================================================================
 * ISR: eGTM ATOM period event
 * Use EXACT macro name for priority ISR_PRIORITY_ATOM
 * ========================================================================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Period event: toggle LED for visual heartbeat */
    IfxPort_togglePin(LED);
}

/* Optional: Unified driver period callback hook (bound via interruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period callback (unused here) */
    (void)data;
}

/* ==========================================================================
 * Public API: Initialization
 * EXACT NAME: IfxEgtm_Atom_Pwm_init
 * Behavior: See SW Detailed Design (enable clocks, configure channels, pins,
 *           dead-time, interrupts; init unified driver; sync start/update; LED
 *           output; status flags).
 * ========================================================================== */
void IfxEgtm_Atom_Pwm_init(void)
{
    /* Clear status flags */
    g_egtmAtom3phInv.pwmInitialized      = FALSE;
    g_egtmAtom3phInv.dtmConfigured       = FALSE;
    g_egtmAtom3phInv.updateInProgress    = FALSE;
    g_egtmAtom3phInv.errorClock          = FALSE;
    g_egtmAtom3phInv.errorPinMap         = FALSE;
    g_egtmAtom3phInv.errorInit           = FALSE;
    g_egtmAtom3phInv.errorDutyOutOfRange = FALSE;

    /* ----------------------------------------------------------------------
     * eGTM module enable + CMU clock setup (MANDATORY for TC4xx)
     * ---------------------------------------------------------------------- */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Set GCLK to module frequency and CLK0 to required 100 MHz, then enable CLK0 */
    {
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, (float32)CLOCK_TARGET_FXCLK0_HZ);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Verify functional clock is present */
    if (!IfxEgtm_Cmu_isFxClockEnabled(&MODULE_EGTM))
    {
        g_egtmAtom3phInv.errorClock = TRUE;
        g_egtmAtom3phInv.errorInit  = TRUE;
        return; /* Early-exit on clock failure */
    }

    /* ----------------------------------------------------------------------
     * Unified PWM configuration structures
     * ---------------------------------------------------------------------- */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* Initialize top-level config defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: single period IRQ on base channel (CH0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output pin configuration (complementary HS/LS per phase) */
    /* Phase V on CH0 (base): HS=P20.10, LS=P20.11 per requirements mapping order U/V/W can be adapted;
       Here we bind CH0->V, CH1->U, CH2->W to match reference pattern. */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[0].polarity                 = Ifx_ActiveState_high;
    output[0].complementaryPolarity    = Ifx_ActiveState_low;
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase U on CH1 */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W on CH2 */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration (hardware CDTM/DTM) */
    dtmConfig[0].deadTime.rising  = EGTM_PWM_DEADTIME_S;
    dtmConfig[0].deadTime.falling = EGTM_PWM_DEADTIME_S;
    dtmConfig[1].deadTime.rising  = EGTM_PWM_DEADTIME_S;
    dtmConfig[1].deadTime.falling = EGTM_PWM_DEADTIME_S;
    dtmConfig[2].deadTime.rising  = EGTM_PWM_DEADTIME_S;
    dtmConfig[2].deadTime.falling = EGTM_PWM_DEADTIME_S;

    /* Channel configuration (ATOM0 CH0/1/2, center aligned, initial duty 0%) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;  /* Base channel with ISR */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 0.0f;                        /* percent */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;  /* Sync channel */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 0.0f;                        /* percent */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;  /* Sync channel */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 0.0f;                        /* percent */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Top-level PWM configuration */
    config.cluster              = IfxEgtm_Cluster_1;                    /* EGTM_CLUSTER = 1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;           /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;         /* Center-aligned */
    config.syncStart            = TRUE;                                  /* Start all channels after init */
    config.syncUpdateEnabled    = TRUE;                                  /* Shadow update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = channelConfig;
    config.frequency            = PWM_FREQUENCY;                         /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                     /* Use CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM/CDTM clock */

    /* Initialize the PWM driver (returns void) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Store initial runtime mirrors */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* Configure LED pin mode for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Install ISR handler (binds function to priority; routing done by driver config) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, ISR_PRIORITY_ATOM);

    /* Update status flags */
    g_egtmAtom3phInv.dtmConfigured  = TRUE;
    g_egtmAtom3phInv.pwmInitialized = TRUE;
}

/* ==========================================================================
 * Public API: Synchronized duty update (normalized input in [0.0 .. 1.0])
 * EXACT NAME: IfxEgtm_Atom_Pwm_setDutyCycle
 * Algorithm (from SW Detailed Design):
 *   - Read requested duty array from shared state
 *   - Clamp each to [0.0 .. 1.0], set error flag if clamped
 *   - Set update_in_progress, write all duties via unified API for next period
 *   - Clear update_in_progress (non-blocking)
 * ========================================================================== */
void IfxEgtm_Atom_Pwm_setDutyCycle(void)
{
    float32 dutyPercent[NUM_OF_CHANNELS];
    boolean anyClamped = FALSE;

    /* Read and clamp normalized requests, convert to percent */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_egtmAtom3phInv_requestDuty[i];
        if (d < 0.0f)
        {
            d = 0.0f; anyClamped = TRUE;
        }
        else if (d > 1.0f)
        {
            d = 1.0f; anyClamped = TRUE;
        }
        dutyPercent[i] = d * 100.0f;        /* unified driver expects percent */
        g_egtmAtom3phInv.dutyCycles[i] = dutyPercent[i];
    }

    g_egtmAtom3phInv.updateInProgress    = TRUE;
    g_egtmAtom3phInv.errorDutyOutOfRange = anyClamped;

    /* Apply immediate synchronized update (shadow -> active at period boundary) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, dutyPercent);

    g_egtmAtom3phInv.updateInProgress    = FALSE;
}
