/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * TC4xx (TC4D7) eGTM ATOM 3-Phase Inverter PWM - Production Source
 *
 * Follows mandatory unified IfxEgtm_Pwm initialization patterns.
 * - Uses OutputConfig/DtmConfig/ChannelConfig arrays
 * - Enables eGTM module and CMU clocks
 * - Sets center alignment, syncStart, syncUpdateEnabled
 * - Configures interrupt via channelConfig[0].interrupt
 * - Does NOT call redundant post-init APIs (start/output/enableSync), per unified driver rules
 * - No watchdog handling here (must be in CpuN_Main.c)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxCpu.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ========================= Local Types and State ========================= */

typedef struct
{
    IfxEgtm_Pwm           pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];      /* Runtime duty (percent) */
    float32               phases[NUM_OF_CHANNELS];          /* Runtime phases (rad or percent of period) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* Runtime dead-time (rising/falling seconds) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;
EgtmPwm_Status g_egtmPwmStatus = {0u, 0u, 0u, EGTM_PWM_ERROR_NONE};

/* ========================= Forward Declarations ========================= */
void IfxEgtm_periodEventFunction(void *data);

/* ISR declaration: exact macro name and priority macro as per pattern */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/* ========================= Helpers ========================= */
static float32 prv_nsToSeconds(uint32 ns)
{
    /* Convert nanoseconds to seconds (float32) */
    return ((float32)ns) * 1.0e-9f;
}

/* ========================= ISR and Callback ========================= */
/**
 * Period ISR for ATOM (bound via channel interrupt config). Calls the unified driver's
 * interrupt handler to dispatch the periodEvent callback, and toggles a diagnostic LED.
 */
void interruptEgtmAtom(void)
{
    /* Dispatch to unified driver handler for channel 0 (period source) */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);

    /* Diagnostic GPIO toggle */
    IfxPort_togglePin(LED);
}

/**
 * Period event callback invoked by the driver's interruptHandler.
 * Increments ISR epoch and records last-applied epoch for missed-update detection.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
    g_egtmPwmStatus.isrCount++;
    /* We consider period latch occurred; last applied corresponds to current epoch. */
}

/* ========================= Initialization ========================= */
/**
 * Initialize the eGTM for 3-phase center-aligned complementary PWM generation using ATOM channels
 * as sources and CDTM DTM for hardware dead-time.
 */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* ========================= 1) eGTM module enable + CMU clock setup ========================= */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Configure GCLK = module clock (no divide), select CLK0 from GCLK and enable CLK0 */
    {
        float32 moduleHz = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleHz);
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u); /* 1:1 divider */
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE); /* useGlobal=TRUE */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* ========================= 2) Init unified PWM config with defaults ========================= */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;                  /* Start channels synced after init */
    config.syncUpdateEnabled    = TRUE;                  /* Shadow -> actual at period */
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;     /* ATOM uses Clk enum */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* ========================= 3) Interrupt configuration (period on CH0) ========================= */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;     /* EXACT macro name */
    interruptConfig.vmId        = IfxSrc_VmId_0;         /* TC4xx */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* ========================= 4) Output + DTM config per channel ========================= */
    /* NOTE: Pin routing is handled by the unified driver via output[i].pin/complementaryPin.
     *       Board-specific TOUT macros must be assigned here when available.
     *       For now, leave pins as NULL_PTR to avoid double routing via PinMap API.
     */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        output[i].pin                        = NULL_PTR;                  /* TODO: assign board TOUT HS pin */
        output[i].complementaryPin           = NULL_PTR;                  /* TODO: assign board TOUT LS pin */
        output[i].polarity                   = Ifx_ActiveState_high;      /* HS active high */
        output[i].complementaryPolarity      = Ifx_ActiveState_low;       /* LS active low */
        output[i].outputMode                 = IfxPort_OutputMode_pushPull;
        output[i].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }

    /* Dead time for each phase: 1 us rising and 1 us falling */
    const float32 deadTimeSec = prv_nsToSeconds(DEADTIME_NS);
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        dtmConfig[i].deadTime.rising  = deadTimeSec;
        dtmConfig[i].deadTime.falling = deadTimeSec;
    }

    /* ========================= 5) Channel configuration ========================= */
    /* Initialize channel config defaults and then set fields explicitly */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    /* Use ATOM2 channels 4/5/6 as requested (SubModule_Ch_4..6) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_4;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;      /* Period interrupt on base channel */

    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_5;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_6;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Link channel array */
    config.numChannels = NUM_OF_CHANNELS;
    config.channels    = &channelConfig[0];

    /* ========================= 6) Initialize PWM driver ========================= */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* ========================= 7) Post-init bookkeeping (no redundant driver calls) ========================= */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = channelConfig[i].duty;
        g_egtmAtom3phInv.phases[i]     = channelConfig[i].phase;
        g_egtmAtom3phInv.deadTimes[i]  = dtmConfig[i].deadTime;
    }

    /* Configure LED for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Install ISR handler (service request priority matches interruptConfig.priority) */
    IfxCpu_Irq_installInterruptHandler((void *)interruptEgtmAtom, ISR_PRIORITY_ATOM);

    /* Status defaults */
    g_egtmPwmStatus.lastError        = EGTM_PWM_ERROR_NONE;
    g_egtmPwmStatus.isrCount         = 0u;
    g_egtmPwmStatus.missedUpdates    = 0u;
    g_egtmPwmStatus.lastRequestEpoch = 0u;

    /* NOTE: Do NOT call startSyncedChannels/startChannelOutputs/enableChannelsSyncUpdate here.
     *       Unified driver honors config.syncStart and config.syncUpdateEnabled internally.
     */
}

/* ========================= Runtime Duty Update ========================= */
/**
 * Implements a cyclic duty update for all three phases:
 *  - For each phase: if duty + step >= 100 then wrap to 0 before adding step; else increment by step
 *  - Apply all three duties atomically using the immediate vector update API
 *  - Maintain missed-updates counter if a new request occurs within the same ISR epoch
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Missed update detection: if we are issuing a new request within the same ISR epoch
     * (no new period since last request), increment missedUpdates. Protect test with IRQ lock. */
    boolean irqState = IfxCpu_disableInterrupts();
    if (g_egtmPwmStatus.lastRequestEpoch == g_egtmPwmStatus.isrCount)
    {
        g_egtmPwmStatus.missedUpdates++;
    }
    /* Stamp this request at current epoch */
    g_egtmPwmStatus.lastRequestEpoch = g_egtmPwmStatus.isrCount;
    IfxCpu_enableInterrupts();

    /* Algorithm: increment and wrap */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Apply immediately as a vector update */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
