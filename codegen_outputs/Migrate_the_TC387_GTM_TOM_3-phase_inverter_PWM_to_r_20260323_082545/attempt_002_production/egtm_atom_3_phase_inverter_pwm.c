/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief eGTM ATOM 3-Phase Inverter PWM (TC4xx) - Production Source
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ==========================================================================
 * Driver/application state
 * ========================================================================== */
typedef struct {
    IfxEgtm_Pwm          pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel state */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values in percent */
    float32              phases[NUM_OF_CHANNELS];          /* Phase shift values in percent */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Deadtime values (rising/falling in seconds) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;
static boolean         s_initialized = FALSE;

/* ==========================================================================
 * ISR and callback
 * ========================================================================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
    /* Period event callback hook (user extension point) */
}

/* ==========================================================================
 * Local helpers
 * ========================================================================== */
static void prv_configurePinsMux(void)
{
    /* Apply pin-mux for all PWM outputs using PinMap API (as per SDD Step 7) */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ==========================================================================
 * Public functions
 * ========================================================================== */
void initEgtmAtom3phInv(void)
{
    /* Step 1: Enable eGTM and configure CMU clock input (Clk1) at target frequency */
    IfxEgtm_enable(&MODULE_EGTM);
    IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, (float32)TIMING_CMU_CLOCK_HZ);
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_1, TRUE); /* Use GCLK for CLK1 */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK1);

    /* Step 2: Initialize unified PWM configuration */
    IfxEgtm_Pwm_Config config;
    IfxEgtm_Pwm_ChannelConfig channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Step 3: Configure output mappings, polarity, and pad */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Step 4: Attach DTM with 1 us rising/falling deadtimes */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SECONDS;

    /* Step 5: Set up a single period interrupt on base channel */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Step 6: Initialize per-channel config entries */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;  /* Base channel */
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;  /* Sync channel */
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;  /* Sync channel */
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Step 7: Apply pin mux through PinMap API */
    prv_configurePinsMux();

    /* Step 8: Finalize main config fields */
    config.cluster           = IfxEgtm_Cluster_1;
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;
    config.alignment         = IfxEgtm_Pwm_Alignment_center;
    config.syncStart         = TRUE;
    config.syncUpdateEnabled = TRUE;
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = channelConfig;
    config.frequency         = PWM_FREQUENCY;               /* 20 kHz */
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_1;           /* Use ATOM CLK1 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM source */

    /* Step 8 (cont.): Initialize PWM driver with prepared configuration */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Step 9: Program deadtime for all channels via bulk update API */
    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;
    IfxEgtm_Pwm_updateChannelsDeadTime(&g_egtmAtom3phInv.pwm, (IfxEgtm_Pwm_DeadTime *)g_egtmAtom3phInv.deadTimes);

    /* Step 10: Install ISR into CPU interrupt vector (base channel priority) */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* Step 11: Start synchronized PWM channels */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Store initial duty and phase values for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    /* Configure LED GPIO for ISR diagnostic toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

void updateEgtmAtom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if not initialized */
    }

    /* Wrap then increment per-phase duty as specified */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Latch new duties; take effect synchronously at next period event */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
