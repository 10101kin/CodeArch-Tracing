/**********************************************************************************************************************
 * Module: EGTM_ATOM_3_Phase_Inverter_PWM
 * File:   egtm_atom_3_phase_inverter_pwm.c
 * Brief:  TC4xx eGTM ATOM 3-phase inverter PWM (unified IfxEgtm_Pwm driver)
 *
 * Implementation follows authoritative iLLD patterns and the SW Detailed Design.
 * - Uses unified IfxEgtm_Pwm driver with OutputConfig/DtmConfig/ChannelConfig arrays
 * - Mandatory eGTM enable + CMU clock setup block precedes driver init
 * - Interrupt configured via channelConfig[0].interrupt only (no separate initInterrupts call)
 * - No watchdog handling (must be in CpuN_Main.c only)
 * - No redundant post-init calls (startSyncedChannels, enableSyncUpdate, etc.)
 **********************************************************************************************************************/

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"
#include "IfxEgtm.h"

/* ========================================================================
 * Driver/application state structure (unified driver handle + runtime values)
 * ======================================================================== */

typedef struct
{
    IfxEgtm_Pwm           pwm;                         /* PWM Driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];   /* Channel data populated by init */
    float32               dutyCycles[NUM_OF_CHANNELS]; /* Runtime duty cycles (percent) */
    float32               phases[NUM_OF_CHANNELS];     /* Phase shift values (not used -> 0) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];  /* Dead-time values (rising/falling) */
} EgtmAtom3phInv;

/* Static driver instance */
static EgtmAtom3phInv g_egtmAtom3phInv;

/* ========================================================================
 * Forward declarations (private)
 * ======================================================================== */
static void configureEgtmClocking(void);
static void IfxEgtm_periodEventFunction(void *data);

/* ========================================================================
 * ISR declaration (use EXACT macro name for priority symbol)
 * ======================================================================== */
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);

void interruptEgtmAtomPeriod(void)
{
    /* Delegate housekeeping to unified PWM driver for base channel */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
}

/* Period event callback (linked via interrupt config; may be empty by design) */
static void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentional no-op: application can add hooks if needed */
}

/* ========================================================================
 * Mandatory EGTM enable + CMU clock setup (must be called before IfxEgtm_Pwm_init)
 * ======================================================================== */
static void configureEgtmClocking(void)
{
    /* Enable eGTM if not already enabled, then set GCLK and select/use CLK0 from GCLK */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
    }

    /* Route ATOM CLK0 from GCLK and enable it */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
}

/* ========================================================================
 * Public API implementations
 * ======================================================================== */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* 1) Mandatory eGTM module enable + CMU clock setup */
    configureEgtmClocking();

    /* 2) Initialize unified PWM configuration with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure outputs (complementary pairs) via OutputConfig array
       - Routing is handled internally by IfxEgtm_Pwm using the provided pin maps
       - HS active-high, LS active-low, push-pull, automotive pad */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure symmetric deadtime via DTM/CDTM */
    dtmConfig[0].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[0].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[1].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[1].deadTime.falling = DEADTIME_FALLING_S;
    dtmConfig[2].deadTime.rising  = DEADTIME_RISING_S;
    dtmConfig[2].deadTime.falling = DEADTIME_FALLING_S;

    /* 5) Configure interrupt on base channel for period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;    /* EXACT macro name */
    interruptConfig.vmId        = IfxSrc_VmId_0;        /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Configure channel settings: base CH0 + sync CH1/CH2 (contiguous) */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].timerCh = (IfxEgtm_Pwm_SubModule_Ch)i;  /* CH0, CH1, CH2 */
        channelConfig[i].phase   = 0.0f;
        channelConfig[i].dtm     = &dtmConfig[i];
        channelConfig[i].output  = &output[i];
        channelConfig[i].mscOut  = NULL_PTR;
        channelConfig[i].interrupt = (i == 0U) ? &interruptConfig : NULL_PTR; /* Only base CH0 has ISR */
    }

    channelConfig[0].duty = PHASE_U_DUTY;
    channelConfig[1].duty = PHASE_V_DUTY;
    channelConfig[2].duty = PHASE_W_DUTY;

    /* 6) Configure global PWM behavior */
    config.cluster            = IfxEgtm_Cluster_1;                 /* CHANNEL_ASSIGNMENT_EGTM_CLUSTER = 1 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;        /* Use ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart          = TRUE;                              /* Auto-synchronous start */
    config.syncUpdateEnabled  = TRUE;                              /* Shadow update on period event */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                 /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM CLK0 */

    /* 7) Initialize the PWM driver (reads all config; no post-init redundant calls) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime values for updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* Install ISR (service request priority configured by interruptConfig/priority) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtomPeriod, ISR_PRIORITY_ATOM);

    /* Configure status LED pin (push-pull, active high) and set low initially */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Algorithm: for each phase, if (current + STEP) >= 100 -> wrap to 0, then add STEP */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Queue new duty cycles; hardware applies them synchronously at next period boundary */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
