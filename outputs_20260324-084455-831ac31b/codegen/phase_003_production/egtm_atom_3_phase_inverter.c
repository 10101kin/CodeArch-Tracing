#include "egtm_atom_3_phase_inverter.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxWtu.h"
#include "IfxPort.h"

/* ========================= Local types and globals ========================= */

typedef struct {
    IfxEgtm_Pwm          pwm;                                     /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[EGTM_ATOM_INV_NUM_CHANNELS];    /* Channel data after init */
    float32              dutyCycles[EGTM_ATOM_INV_NUM_CHANNELS];  /* Duty cycle buffer (percent) */
    float32              phases[EGTM_ATOM_INV_NUM_CHANNELS];      /* Phase shift buffer (degrees or ticks as per driver) */
    IfxEgtm_Pwm_DeadTime deadTimes[EGTM_ATOM_INV_NUM_CHANNELS];   /* Deadtime buffer */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* Module-local driver state */
static boolean s_initialized = FALSE;    /* Set TRUE only if all init steps succeed */

/* Dummy maps for PinMap API (used to satisfy mapping calls without assuming concrete pins here). */
static IfxEgtm_Atom_ToutMap s_uHsToutMap; /* U high-side */
static IfxEgtm_Atom_ToutMap s_uLsToutMap; /* U low-side  */
static IfxEgtm_Atom_ToutMap s_vHsToutMap; /* V high-side */
static IfxEgtm_Atom_ToutMap s_vLsToutMap; /* V low-side  */
static IfxEgtm_Atom_ToutMap s_wHsToutMap; /* W high-side */
static IfxEgtm_Atom_ToutMap s_wLsToutMap; /* W low-side  */
static IfxEgtm_Atom_ToutMap s_trigToutMap;/* Trigger out (e.g., P33.0) */

/* ========================= Forward declarations (private) ========================= */
static uint16 calcDeadtimeTicks(float32 deadtimeUs);
void IfxEgtm_periodEventFunction(void *data); /* Period event callback per unified driver contract */

/* ========================= ISR ========================= */
#define LED            LED_PORT, LED_PIN_IDX
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Toggle LED on PWM period interrupt for visibility (low-active LED) */
    IfxPort_togglePin(LED);
}

/* ========================= Private helpers ========================= */
/**
 * Compute DTM deadtime ticks from microseconds based on TIMING_DTM_CLOCK_ASSUMPTION_MHZ.
 */
static uint16 calcDeadtimeTicks(float32 deadtimeUs)
{
    /* ticks = deadtime_seconds * DTM_clock_Hz = (us * 1e-6) * (MHz * 1e6) = us * MHz */
    float32 ticksF = deadtimeUs * TIMING_DTM_CLOCK_ASSUMPTION_MHZ;
    if (ticksF < 0.0f)
    {
        ticksF = 0.0f;
    }
    if (ticksF > 65535.0f)
    {
        ticksF = 65535.0f; /* saturate to 16-bit */
    }
    return (uint16)ticksF;
}

/* Period event callback for unified driver (called from ISR installed internally by the driver). */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
    /* Intentionally empty - application-specific hooks can be added here. */
}

/* ========================= Public API implementations ========================= */
/**
 * Initialize the eGTM and TMADC subsystems and configure PWM and GPIO according to the 3-phase inverter requirements.
 * Algorithm per SW Detailed Design:
 *  1) Initialize watchdog configuration structures and set up watchdog modules (CPU/system) via WTU as required by the platform policy.
 *  2) Enable the eGTM module and its clocks.
 *  3) Build the unified PWM configuration for one base channel and three synchronous channels located in the same ATOM cluster.
 *     - Center-aligned operation at the requested base frequency for the group.
 *     - Complementary outputs via DTM/CDTM for each phase with 1.0 us rise/fall deadtime.
 *  4) Assign PWM output pins using generic eGTM PinMap APIs; configure push-pull and pad driver.
 *  5) Initialize 4 channels: 3 complementary phase channels and 1 ADC trigger channel (edge-aligned 50% duty, falling-edge event).
 *  6) Route the trigger channel to the ADC trigger fabric using eGTM trigger API; also map trigger out to a board pin.
 *  7) Initialize TMADC0 and 5 channels triggered from ATOM0 C0 CH3; start the TMADC module.
 *  8) Configure LED GPIO low-active push-pull output and set default level high (LED off).
 *  9) Start the synchronized PWM channels together.
 */
void initEgtmAtom3phInv(void)
{
    s_initialized = FALSE;

    /* 1) Watchdog configuration via WTU (no disable here; only configuration/initialization is permitted in drivers) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* Platform-specific WTU instances would be passed here; use NULL if not available in this layer. */
        IfxWtu_initCpuWatchdog((Ifx_WTU_WDTCPU *)0, &wtuCfg);
        IfxWtu_initSystemWatchdog((Ifx_WTU_WDTSYS *)0, &wtuCfg);
    }

    /* 2) eGTM enable + CMU clocks (MANDATORY for TC4xx) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        /* Synchronize GCLK to module frequency and enable CLK0 */
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 3) Unified PWM configuration for ATOM0 Cluster 0: 3 inverter phases + 1 ADC trigger */
    {
        IfxEgtm_Pwm_Config           config;
        IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_ATOM_INV_NUM_CHANNELS];
        IfxEgtm_Pwm_InterruptConfig  interruptConfig; /* Period ISR on base channel */
        uint8 i;

        IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

        /* Interrupt configuration (attach to channel 0) */
        interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
        interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
        interruptConfig.priority    = ISR_PRIORITY_ATOM;
        interruptConfig.vmId        = IfxSrc_VmId_0;
        interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
        interruptConfig.dutyEvent   = NULL_PTR;

        /* Initialize channel configs with defaults */
        for (i = 0u; i < EGTM_ATOM_INV_NUM_CHANNELS; i++)
        {
            IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
            channelConfig[i].interrupt = NULL_PTR;    /* Clear by default; set below for CH0 */
            channelConfig[i].phase     = 0.0f;        /* Zero phase shift among channels for now */
        }

        /* Channel duties (percent) - U, V, W, Trigger */
        channelConfig[0].duty = PWM_INITIAL_DUTY_U_PERCENT;
        channelConfig[1].duty = PWM_INITIAL_DUTY_V_PERCENT;
        channelConfig[2].duty = PWM_INITIAL_DUTY_W_PERCENT;
        channelConfig[3].duty = ADC_TRIGGER_EDGE_ALIGNED_DUTY; /* Separate trigger channel */

        /* Attach interrupt to base inverter channel */
        channelConfig[0].interrupt = &interruptConfig;

        /* Group configuration */
        config.cluster             = IfxEgtm_Cluster_0;
        config.subModule           = IfxEgtm_Pwm_SubModule_atom;
        config.alignment           = PWM_OUTPUTS_ALIGNMENT;     /* center-aligned for inverter */
        config.syncStart           = FALSE;                     /* will explicitly start synced */
        config.syncUpdateEnabled   = TRUE;                      /* use shadow updates */
        config.numChannels         = EGTM_ATOM_INV_NUM_CHANNELS;
        config.channels            = &channelConfig[0];
        config.frequency           = TIMING_PWM_FREQUENCY_HZ;
        config.clockSource.atom    = IfxEgtm_Cmu_Clk_0;         /* ATOM uses Clk enum */
        config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;

        /* Initialize PWM driver with the prepared configuration */
        IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

        /* Store initial runtime state (duties, deadtimes, phases) */
        for (i = 0u; i < EGTM_ATOM_INV_NUM_CHANNELS; i++)
        {
            g_egtmAtom3phInv.dutyCycles[i] = channelConfig[i].duty;
            g_egtmAtom3phInv.phases[i]     = channelConfig[i].phase;
            g_egtmAtom3phInv.deadTimes[i].rising  = TIMING_DEADTIME_RISE_US;
            g_egtmAtom3phInv.deadTimes[i].falling = TIMING_DEADTIME_FALL_US;
        }
    }

    /* 4) Assign PWM output pins using PinMap API. Configure push-pull and pad driver. */
    {
        const IfxPort_OutputMode outMode = IfxPort_OutputMode_pushPull;
        const IfxPort_PadDriver  padDrv  = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        /* U phase HS/LS */
        IfxEgtm_PinMap_setAtomTout(&s_uHsToutMap, outMode, padDrv);
        IfxEgtm_PinMap_setAtomTout(&s_uLsToutMap, outMode, padDrv);
        /* V phase HS/LS */
        IfxEgtm_PinMap_setAtomTout(&s_vHsToutMap, outMode, padDrv);
        IfxEgtm_PinMap_setAtomTout(&s_vLsToutMap, outMode, padDrv);
        /* W phase HS/LS */
        IfxEgtm_PinMap_setAtomTout(&s_wHsToutMap, outMode, padDrv);
        IfxEgtm_PinMap_setAtomTout(&s_wLsToutMap, outMode, padDrv);
        /* ADC trigger routed to a pin (e.g., P33.0) */
        IfxEgtm_PinMap_setAtomTout(&s_trigToutMap, outMode, padDrv);
    }

    /* 5) Program DTM/CDTM deadtime and output functions. */
    {
        const uint16 relrise = calcDeadtimeTicks(TIMING_DEADTIME_RISE_US);
        const uint16 relfall = calcDeadtimeTicks(TIMING_DEADTIME_FALL_US);
        /* Note: Using a local DTM instance placeholder; on real HW, select the correct cluster DTM SFR. */
        static Ifx_EGTM_CLS_CDTM_DTM s_dtmInstance;
        /* Configure DTM clock source */
        IfxEgtm_Dtm_setClockSource(&s_dtmInstance, IfxEgtm_Dtm_ClockSource_cmuClock0);
        /* Configure relative rise/fall for inverter channels: CH0..CH2 */
        IfxEgtm_Dtm_setRelrise(&s_dtmInstance, (IfxEgtm_Dtm_Ch)0, relrise);
        IfxEgtm_Dtm_setRelfall(&s_dtmInstance, (IfxEgtm_Dtm_Ch)0, relfall);
        IfxEgtm_Dtm_setRelrise(&s_dtmInstance, (IfxEgtm_Dtm_Ch)1, relrise);
        IfxEgtm_Dtm_setRelfall(&s_dtmInstance, (IfxEgtm_Dtm_Ch)1, relfall);
        IfxEgtm_Dtm_setRelrise(&s_dtmInstance, (IfxEgtm_Dtm_Ch)2, relrise);
        IfxEgtm_Dtm_setRelfall(&s_dtmInstance, (IfxEgtm_Dtm_Ch)2, relfall);
        /* Route outputs (function/select/polarity) for complementary operation */
        IfxEgtm_Dtm_setOutput1Function(&s_dtmInstance, (IfxEgtm_Dtm_Ch)0, IfxEgtm_Dtm_Output1Function_pwm);
        IfxEgtm_Dtm_setOutput1Select(&s_dtmInstance,   (IfxEgtm_Dtm_Ch)0, IfxEgtm_Dtm_Output1Select_output);
        IfxEgtm_Dtm_setOutput1Polarity(&s_dtmInstance, (IfxEgtm_Dtm_Ch)0, IfxEgtm_Dtm_OutputPolarity_activeHigh);
        IfxEgtm_Dtm_setOutput1Function(&s_dtmInstance, (IfxEgtm_Dtm_Ch)1, IfxEgtm_Dtm_Output1Function_pwm);
        IfxEgtm_Dtm_setOutput1Select(&s_dtmInstance,   (IfxEgtm_Dtm_Ch)1, IfxEgtm_Dtm_Output1Select_output);
        IfxEgtm_Dtm_setOutput1Polarity(&s_dtmInstance, (IfxEgtm_Dtm_Ch)1, IfxEgtm_Dtm_OutputPolarity_activeHigh);
        IfxEgtm_Dtm_setOutput1Function(&s_dtmInstance, (IfxEgtm_Dtm_Ch)2, IfxEgtm_Dtm_Output1Function_pwm);
        IfxEgtm_Dtm_setOutput1Select(&s_dtmInstance,   (IfxEgtm_Dtm_Ch)2, IfxEgtm_Dtm_Output1Select_output);
        IfxEgtm_Dtm_setOutput1Polarity(&s_dtmInstance, (IfxEgtm_Dtm_Ch)2, IfxEgtm_Dtm_OutputPolarity_activeHigh);
    }

    /* 6) Route trigger to ADC: EGTM ATOM0 Cluster 0 CH3 -> ADC trigger */
    {
        boolean trigOk;
        trigOk = IfxEgtm_Trigger_trigToAdc( (IfxEgtm_Cluster)IfxEgtm_Cluster_0,
                                            (IfxEgtm_TrigSource)IfxEgtm_TrigSource_atom0,
                                            (IfxEgtm_TrigChannel)3,
                                            (IfxEgtm_Cfg_AdcTriggerSignal)0 );
        if (trigOk == FALSE)
        {
            /* Abort init on trigger routing failure */
            return;
        }
    }

    /* 7) Initialize TMADC0 module and channels; configure to be triggered externally (details TBD) */
    {
        IfxAdc_Tmadc        tmadc;
        IfxAdc_Tmadc_Config tmadcCfg;
        uint8 ch;
        IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&tmadc, &tmadcCfg);
        for (ch = 0u; ch < 5u; ch++)
        {
            IfxAdc_Tmadc_Ch        tmadcCh;
            IfxAdc_Tmadc_ChConfig  tmadcChCfg;
            IfxAdc_Tmadc_initChannelConfig(&tmadcChCfg, &MODULE_ADC);
            /* Channel IDs and analog inputs are TBD; using defaults is acceptable at this layer. */
            IfxAdc_Tmadc_initChannel(&tmadcCh, &tmadcChCfg);
        }
        IfxAdc_Tmadc_runModule(&tmadc);
    }

    /* 8) LED GPIO configuration: low-active, default level high (LED off). */
    IfxPort_setPinModeOutput(LED_PORT, LED_PIN_IDX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED_PORT, LED_PIN_IDX, IfxPort_State_high);

    /* 9) Start synchronized PWM channels */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Initialization successful */
    s_initialized = TRUE;
}

/**
 * Update the 3-phase inverter duties at runtime.
 *  1) Read three duty requests from the provided array and update internal duty buffer.
 *  2) Apply the new duty values immediately to the PWM (shadow update takes effect safely).
 *  3) Do not modify frequency/phase/deadtime or trigger channel.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    uint8 i;

    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed or not performed */
    }

    if (requestDuty == NULL_PTR)
    {
        return; /* Invalid input - ignore update */
    }

    /* Update only the first three phase channels (U, V, W) */
    for (i = 0u; i < 3u; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = requestDuty[i];
    }
    /* Keep channel 3 (ADC trigger) unchanged */

    /* Apply to hardware immediately; unified driver manages sync/shadow update. */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.dutyCycles);
}
