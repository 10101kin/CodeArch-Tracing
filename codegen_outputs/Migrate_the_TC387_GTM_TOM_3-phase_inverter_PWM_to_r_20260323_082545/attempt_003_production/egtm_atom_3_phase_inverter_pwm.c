#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* LED for ISR diagnostic toggle (TC4xx example pin) */
#define LED &MODULE_P03, 9

/* Proposed 3-phase pin mappings on P02.x high-side and P02.{7,4,5} low-side (complementary) */
/* Use GENERIC PinMap headers (no family-specific suffixes) */
#define PHASE_U_HS              (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS              (&IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS              (&IfxEgtm_ATOM0_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS              (&IfxEgtm_ATOM0_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS              (&IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS              (&IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT)

/* Driver/application state (unified IfxEgtm_Pwm high-level driver) */
typedef struct {
    IfxEgtm_Pwm          pwm;                               /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel state (filled by init) */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* Duty cycle values in percent */
    float32              phases[NUM_OF_CHANNELS];          /* Phase shift values (deg or percent as configured) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Dead-time values (rising/falling) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* Module-private instance */

/* ISR: CPU0, priority defined by ISR_PRIORITY_ATOM. Toggle LED for visibility. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback hook (unified driver can invoke this on period events) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* User period-event handling (optional). Keep minimal for real-time constraints. */
    (void)data;
}

/* Init function implementing SW Detailed Design algorithm exactly */
void initEgtmAtom3phInv(void)
{
    /* 1) Enable eGTM and configure CMU clock input (Clk1) at required frequency */
    IfxEgtm_enable(&MODULE_EGTM);
    IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, (float32)TIMING_CMU_CLOCK_HZ);
    /* Route ATOM to use global GCLK via CLk1 and enable CLk1 */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_1, TRUE);
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK1);

    /* 2) Initialize unified PWM configuration and set core attributes */
    IfxEgtm_Pwm_Config config;
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Local per-channel configuration containers */
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* 3) Configure output mapping and electrical characteristics for each phase */
    output[0].pin                       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[0].complementaryPin          = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[0].polarity                  = Ifx_ActiveState_high;
    output[0].complementaryPolarity     = Ifx_ActiveState_low;
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

    /* 4) Attach DTM with 1.0 us symmetric dead-time for all channels */
    dtmConfig[0].deadTime.rising        = 1.0e-6f;
    dtmConfig[0].deadTime.falling       = 1.0e-6f;
    dtmConfig[1].deadTime.rising        = 1.0e-6f;
    dtmConfig[1].deadTime.falling       = 1.0e-6f;
    dtmConfig[2].deadTime.rising        = 1.0e-6f;
    dtmConfig[2].deadTime.falling       = 1.0e-6f;

    /* 5) Single period interrupt on base channel only */
    interruptConfig.mode                = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider         = IfxSrc_Tos_cpu0;
    interruptConfig.priority            = ISR_PRIORITY_ATOM;
    interruptConfig.vmId                = IfxSrc_VmId_0;    /* TC4xx specific */
    interruptConfig.periodEvent         = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent           = NULL_PTR;

    /* 6) Initialize per-channel configs (base channel then synchronous channels) */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; ++i)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    /* Channel 0: base (Phase V in this ordering) */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_V_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* Period ISR on base channel */

    /* Channel 1: synchronous (Phase U) */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_U_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2: synchronous (Phase W) */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Apply pin mux using eGTM PinMap API (explicit, as per SW design) */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 8) Populate global config and initialize the unified PWM driver */
    config.cluster             = IfxEgtm_Cluster_1;                 /* Use Cluster 1 as proposed */
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;        /* ATOM submodule */
    config.alignment           = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned (up-down) */
    config.syncStart           = TRUE;                               /* Start after init */
    config.syncUpdateEnabled   = TRUE;                               /* Shadow register update */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY;                      /* 20 kHz */
    config.clockSource.atom    = IfxEgtm_Cmu_Clk_1;                  /* Use CLk1 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock */

    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 9) Program dead-time for all channels via bulk update API */
    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;
    IfxEgtm_Pwm_updateChannelsDeadTime(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.deadTimes[0]);

    /* 10) Install ISR into CPU0 vector (period event on base channel) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* 11) Start synchronized PWM channels */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Store initial runtime values and configure LED output */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* Runtime duty update: increment with wrap, synchronous apply on next period */
void updateEgtmAtom3phInvDuty(void)
{
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; ++i)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
    }

    for (uint8 i = 0U; i < NUM_OF_CHANNELS; ++i)
    {
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Apply via unified driver; values will latch and update synchronously */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.dutyCycles[0]);
}
