/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production-ready eGTM ATOM 3-phase inverter PWM driver (TC4xx / TC387)
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* =========================
 * Driver state structure
 * ========================= */
typedef struct {
    IfxEgtm_Pwm          pwm;                               /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel runtime handles */
    float32              dutyCycles[NUM_OF_CHANNELS];       /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];           /* Phase shift values (deg or %) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead-time (rising/falling) */
} EgtmAtom3phInv;

IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* =========================
 * Forward declarations
 * ========================= */
static void configureEgtmClocking(void);
static void IfxEgtm_periodEventFunction(void *data);

/* =========================
 * Interrupt Service Routine
 * ========================= */
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtomPeriod(void)
{
    /* Minimal ISR work: driver handler + optional LED toggle for diagnostics */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
    IfxPort_togglePin(LED);
}

/* =========================
 * Private: eGTM clocking
 * ========================= */
static void configureEgtmClocking(void)
{
    /* Mandatory eGTM module enable + CMU clock setup (before IfxEgtm_Pwm_init) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Use module frequency for GCLK; route CLK0 from GCLK */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }
}

/* =========================
 * Period event callback (optional)
 * ========================= */
static void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period callback; keep minimal for real-time constraints. */
    (void)data;
}

/* =========================
 * Initialization (public)
 * ========================= */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* 1) Enable eGTM and configure clocks */
    configureEgtmClocking();

    /* 2) Initialize unified PWM configuration with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3a) Interrupt configuration (period-event on base channel, CPU0, priority 20) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 3b) Output pin routing and polarities via OutputConfig (no direct PinMap calls) */
    /* Phase U */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;   /* Order: V,U,W per reference pattern */
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[0].polarity                  = Ifx_ActiveState_high;               /* HS active high */
    output[0].complementaryPolarity     = Ifx_ActiveState_low;                /* LS active low */
    output[0].outputMode                = IfxPort_OutputMode_pushPull;
    output[0].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[1].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[1].polarity                  = Ifx_ActiveState_high;
    output[1].complementaryPolarity     = Ifx_ActiveState_low;
    output[1].outputMode                = IfxPort_OutputMode_pushPull;
    output[1].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                  = Ifx_ActiveState_high;
    output[2].complementaryPolarity     = Ifx_ActiveState_low;
    output[2].outputMode                = IfxPort_OutputMode_pushPull;
    output[2].padDriver                 = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time (symmetric 1us) per channel via DTM/CDTM */
    dtmConfig[0].deadTime.rising  = (DEADTIME_RISING_US  * 1.0e-6f);
    dtmConfig[0].deadTime.falling = (DEADTIME_FALLING_US * 1.0e-6f);
    dtmConfig[1].deadTime.rising  = (DEADTIME_RISING_US  * 1.0e-6f);
    dtmConfig[1].deadTime.falling = (DEADTIME_FALLING_US * 1.0e-6f);
    dtmConfig[2].deadTime.rising  = (DEADTIME_RISING_US  * 1.0e-6f);
    dtmConfig[2].deadTime.falling = (DEADTIME_FALLING_US * 1.0e-6f);

    /* 3c) Per-channel configuration (base channel CH0 + two synchronous channels) */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;     /* Period ISR on base channel */

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;            /* No interrupt on sync channels */

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 4) Top-level driver configuration (alignment, sync behavior, clocks) */
    config.cluster           = IfxEgtm_Cluster_1;                     /* Per requirements */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;            /* ATOM */
    config.alignment         = IfxEgtm_Pwm_Alignment_center;          /* Center-aligned */
    config.syncStart         = TRUE;                                  /* Auto-start after init */
    config.syncUpdateEnabled = TRUE;                                  /* Shadow update at period */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_0;                     /* ATOM uses Clk enum */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;     /* DTM CLK0 */

    /* 5) Initialize the PWM driver (unified high-level) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 6) Store initial runtime values */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* 7) Configure LED pin for ISR diagnostics (active high, push-pull) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);

    /* Install ISR in interrupt controller (CPU0, priority = ISR_PRIORITY_ATOM) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtomPeriod, ISR_PRIORITY_ATOM);

    /* Note: Do NOT call IfxEgtm_Pwm_startSyncedChannels(); syncStart=TRUE handles start. */
}

/* =========================
 * Runtime duty update (public)
 * ========================= */
void updateEgtmAtom3phInvDuty(void)
{
    uint8 i;
    for (i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP;
        if (next >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        else
        {
            g_egtmAtom3phInv.dutyCycles[i] = next;
        }
    }

    /* Queue new duties for synchronous update at next period boundary */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
