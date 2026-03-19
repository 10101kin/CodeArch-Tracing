/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production 3-phase inverter PWM driver implementation for AURIX TC4xx eGTM ATOM.
 *
 * This module follows the unified IfxEgtm_Pwm configuration pattern:
 *  - Declare Config/ChannelConfig/OutputConfig/DtmConfig/InterruptConfig
 *  - Initialize with IfxEgtm_Pwm_initConfig()
 *  - Customize fields (alignment, frequency, syncStart/syncUpdateEnabled)
 *  - Configure channels with pins (OutputConfig), DTM dead-time, and interrupt
 *  - Initialize driver with IfxEgtm_Pwm_init()
 *  - Runtime duty updates via IfxEgtm_Pwm_updateChannelsDutyImmediate()
 *
 * Mandatory TC4xx eGTM clock block is applied before PWM init.
 * Pins are routed via OutputConfig only (no direct PinMap_setAtomTout usage here).
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD dependencies (generic headers only) */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu.h"
#include "IfxPort.h"

/* ==========================
 * Module scope data
 * ==========================
 */
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* ==========================
 * Forward declarations
 * ==========================
 */
void IfxEgtm_periodEventFunction(void *data);

/* ISR: Period event on ATOM (use EXACT macro name ISR_PRIORITY_ATOM) */
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtomPeriod(void)
{
    /* Driver's IRQ handler: link to base channel (CH0) */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);

    /* Diagnostic: toggle LED and book-keeping */
    IfxPort_togglePin(LED);
    g_egtmAtom3phInv.status.isrCount++;
    g_egtmAtom3phInv.epoch++;
}

/* Period event callback (optional) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Application-specific period handling can be placed here if needed. */
    (void)data;
}

/* ==========================
 * Initialization
 * ==========================
 */
void initEgtmAtom3phInv(void)
{
    /* Step 0: Clear runtime state and status */
    g_egtmAtom3phInv.status.lastError      = EGTM_ATOM_PWM_STATUS_OK;
    g_egtmAtom3phInv.status.isrCount       = 0u;
    g_egtmAtom3phInv.status.missedUpdates  = 0u;
    g_egtmAtom3phInv.epoch                 = 0u;
    g_egtmAtom3phInv.lastAppliedEpoch      = 0u;

    /* =============================
     * Mandatory eGTM clock enable
     * ============================= */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_enable(&MODULE_EGTM);
        /* Configure GCLK = module clock (1:1) and select Clk0 from GCLK */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* =============================
     * Unified PWM configuration
     * ============================= */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Initialize defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration (period ISR on CPU0, priority from requirements) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output pin configuration per phase (complementary) */
    /* Phase U */
    output[0].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS; /* HS */
    output[0].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS; /* LS */
    output[0].polarity                   = Ifx_ActiveState_high;
    output[0].complementaryPolarity      = Ifx_ActiveState_low;   /* LS active-low */
    output[0].outputMode                 = IfxPort_OutputMode_pushPull;
    output[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                   = Ifx_ActiveState_high;
    output[1].complementaryPolarity      = Ifx_ActiveState_low;
    output[1].outputMode                 = IfxPort_OutputMode_pushPull;
    output[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                   = Ifx_ActiveState_high;
    output[2].complementaryPolarity      = Ifx_ActiveState_low;
    output[2].outputMode                 = IfxPort_OutputMode_pushPull;
    output[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration per phase (1 us both edges) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        dtmConfig[i].deadTime.rising  = DEAD_TIME_SECONDS;
        dtmConfig[i].deadTime.falling = DEAD_TIME_SECONDS;
    }

    /* Channel configurations: ATOM2 channels 4/5/6, phase=0, initial duty from macros */
    /* Attach interrupt only to the first channel (base) */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);

    /* Phase U -> ATOM2 CH4 */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_4;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;

    /* Phase V -> ATOM2 CH5 */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_5;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Phase W -> ATOM2 CH6 */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_6;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Top-level PWM configuration */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;                        /* Start after init */
    config.syncUpdateEnabled    = TRUE;                        /* Shadow updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;               /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;           /* ATOM uses Clk enum */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Initialize the PWM driver (no return value) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime mirrors (duty and dead-time) */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0]  = *(&channelConfig[0].dtm->deadTime);
    g_egtmAtom3phInv.deadTimes[1]  = *(&channelConfig[1].dtm->deadTime);
    g_egtmAtom3phInv.deadTimes[2]  = *(&channelConfig[2].dtm->deadTime);

    /* Optional: install ISR handler pointer (tests might validate this call) */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptEgtmAtomPeriod, (uint32)ISR_PRIORITY_ATOM);

    /* Configure LED pin for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ==========================
 * Runtime duty update logic
 * ==========================
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Missed update detection: compare ISR epoch vs last applied stamp */
    {
        boolean irqs = IfxCpu_disableInterrupts();
        uint32 epochNow = g_egtmAtom3phInv.epoch;
        if (epochNow == g_egtmAtom3phInv.lastAppliedEpoch)
        {
            g_egtmAtom3phInv.status.missedUpdates++;
        }
        g_egtmAtom3phInv.lastAppliedEpoch = epochNow;
        if (irqs == TRUE)
        {
            IfxCpu_enableInterrupts();
        }
    }

    /* Algorithm: for each phase, if (duty + STEP) >= 100 -> wrap to 0, then add STEP */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[0] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[1] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediate vector update so all channels latch together */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
