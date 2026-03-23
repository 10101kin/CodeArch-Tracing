/*
 * egtm_atom_3_phase_inverter_pwm.c
 * TC4xx - EGTM ATOM 3-Phase Inverter PWM (Unified IfxEgtm_Pwm)
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/*
 * Internal module state
 */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;  /* Driver instance */
IFX_STATIC boolean        s_initialized = FALSE;

/* Forward declarations */
static void configureEgtmClocking(void);
void IfxEgtm_periodEventFunction(void *data);

/*
 * ISR: Period event on base channel
 * - Use exact ISR priority macro name: ISR_PRIORITY_ATOM
 */
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtomPeriod(void)
{
    /* Minimal ISR work: service unified PWM interrupt handler */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);

    /* Optional: diagnostic LED toggle */
    IfxPort_togglePin(LED);
}

/* Period event callback (linked via InterruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period callback (optional). Intentionally left minimal. */
    (void)data;
}

/*
 * Private: Enable EGTM and configure CMU clocks for ATOM and DTM
 */
static void configureEgtmClocking(void)
{
    /* Mandatory EGTM module enable + CMU clock setup */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        /* Use module frequency for GCLK; select CLK0 from GCLK and enable it */
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);  /* useGlobal = TRUE */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }
}

/*
 * Initialize EGTM ATOM 3-phase inverter PWM per SW Detailed Design
 */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* 1) EGTM module + clocking */
    configureEgtmClocking();

    /* 2) Initialize unified PWM configuration */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3a) Interrupt configuration: period event on base channel (CH0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;   /* exact macro name */
    interruptConfig.vmId        = IfxSrc_VmId_0;       /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 3b) Output configuration for each phase (complementary) */
    /* NOTE: Replace NULL placeholders with validated IfxEgtm_PinMap symbols for your HW */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;   /* HS active high */
    output[0].complementaryPolarity    = Ifx_ActiveState_low;    /* LS active low */
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Deadtime configuration per channel (symmetric) */
    dtmConfig[0].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = DEADTIME_FALLING_S;

    /* 5) Channel configuration: base + two synchronous channels */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    /* Base channel CH0 */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* Period event ISR on base channel */

    /* Sync channel CH1 */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Sync channel CH2 */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = TIMING_INITIAL_DUTY_CYCLE_PERCENT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Top-level PWM configuration */
    config.cluster              = EGTM_CLUSTER_USED;                /* Cluster 1 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;       /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;     /* Center-aligned */
    config.syncStart            = TRUE;                             /* Auto synchronous start */
    config.syncUpdateEnabled    = TRUE;                             /* Shadow update at period */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                    /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                /* ATOM CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;/* DTM CLK0 */

    /* 7) Initialize PWM driver (unified). Returns void. */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED pin */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);

    /* Install period ISR handler (cpu0, priority=ISR_PRIORITY_ATOM) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtomPeriod, ISR_PRIORITY_ATOM);

    s_initialized = TRUE;
}

/*
 * Runtime update: increment duty per phase by PHASE_DUTY_STEP, wrap at 100%,
 * then queue synchronous update via unified driver.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if not initialized */
    }

    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Queue synchronous shadow update; hardware applies at next period boundary */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
