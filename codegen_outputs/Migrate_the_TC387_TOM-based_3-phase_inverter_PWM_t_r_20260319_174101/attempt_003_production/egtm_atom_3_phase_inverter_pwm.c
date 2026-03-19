/**********************************************************************************************************************
 * Module: EGTM_ATOM_3_Phase_Inverter_PWM
 * File:   egtm_atom_3_phase_inverter_pwm.c
 * Target: TC4xx (TC4D7/TC387 family)
 * Desc:   3-Phase inverter PWM using eGTM ATOM with unified IfxEgtm_Pwm driver.
 *
 * Implementation rules followed:
 *  - Unified IfxEgtm_Pwm driver with OutputConfig/ChannelConfig/DtmConfig arrays
 *  - eGTM CMU clock enable block before init
 *  - Center-aligned, complementary outputs, 1 us dead-time via DTM
 *  - Interrupt configured via channelConfig.interrupt (period event), ISR calls driver handler
 *  - No watchdog code in this driver (per architecture rule)
 *  - No direct SFR access; no PinMap calls in init (routing via OutputConfig)
 **********************************************************************************************************************/

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxSrc.h"   /* For IfxSrc_Tos_cpu0 and IfxSrc_VmId_0 */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ============================================================================
 * Internal state
 * ==========================================================================*/
IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;
EgtmAtom3phInv_Status     g_egtmAtom3phInvStatus = { EGTM_ATOM_PWM_STATUS_OK, 0u, 0u };

/* Initialization flag */
static volatile boolean s_initialized = FALSE;

/* Epoch counters to detect missed updates (advanced in ISR/callback) */
static volatile uint32 s_updateEpoch      = 0u;
static volatile uint32 s_lastAppliedEpoch = 0u;

/* ============================================================================
 * Period Event Callback (unified driver calls this from its interrupt handler)
 * ==========================================================================*/
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
    ++s_updateEpoch; /* Advance epoch each PWM period */
}

/* ============================================================================
 * ISR: Period interrupt for ATOM (CPU0, ISR_PRIORITY_ATOM)
 * ==========================================================================*/
IFX_INTERRUPT(interruptEgtmAtomPeriod, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtomPeriod(void)
{
    /* Minimal ISR: account, service driver handler, and optional LED toggle */
    g_egtmAtom3phInvStatus.isrCount++;
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
    IfxPort_togglePin(LED);
}

/* ============================================================================
 * Helper: Configure eGTM clocks (MANDATORY for TC4xx before PWM init)
 * ==========================================================================*/
static void egtm_enableClocks(void)
{
    /* Enable module if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Set GCLK to current module frequency (no divide), route to CLK0 and enable CLK0 */
    {
        float32 modFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, modFreq);
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u); /* 1:1 divider */
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE); /* Use GCLK for CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);    /* Enable CLK0 */
    }
}

/* ============================================================================
 * Public API: initEgtmAtom3phInv
 * Behavior per SW Detailed Design
 * ==========================================================================*/
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* Reset status and epochs */
    g_egtmAtom3phInvStatus.lastError    = EGTM_ATOM_PWM_STATUS_OK;
    g_egtmAtom3phInvStatus.isrCount     = 0u;
    g_egtmAtom3phInvStatus.missedUpdates= 0u;
    s_updateEpoch                       = 0u;
    s_lastAppliedEpoch                  = 0u;

    /* 1) Enable eGTM and CMU clocks */
    egtm_enableClocks();

    /* 2) Initialize unified PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration per phase (complementary, active-high HS, active-low LS) */
    /* NOTE: Pin routing is provided via OutputConfig; no direct PinMap calls here. */
    output[0].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                   = Ifx_ActiveState_high;  /* HS active-high */
    output[0].complementaryPolarity      = Ifx_ActiveState_low;   /* LS active-low */
    output[0].outputMode                 = IfxPort_OutputMode_pushPull;
    output[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                   = Ifx_ActiveState_high;
    output[1].complementaryPolarity      = Ifx_ActiveState_low;
    output[1].outputMode                 = IfxPort_OutputMode_pushPull;
    output[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                   = Ifx_ActiveState_high;
    output[2].complementaryPolarity      = Ifx_ActiveState_low;
    output[2].outputMode                 = IfxPort_OutputMode_pushPull;
    output[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration for each phase: 1 us rising and falling */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SECONDS;

    /* 5) Interrupt configuration (period event on channel 0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0; /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: ATOM2 channels 4/5/6, phase=0, initial duty=50% */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_4; /* ATOM2 Ch4 */
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* Period ISR on base channel */

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_5; /* ATOM2 Ch5 */
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_6; /* ATOM2 Ch6 */
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;                     /* Auto-start after init */
    config.syncUpdateEnabled    = TRUE;                     /* Shadow-to-actual enabled */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = TIMING_PWM_FREQUENCY_HZ;
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;        /* Use CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* 8) Initialize the PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 9) Install ISR handler (CPU0, priority matches interruptConfig) */
    IfxCpu_Irq_installInterruptHandler((void*)&interruptEgtmAtomPeriod, ISR_PRIORITY_ATOM);

    /* Store initial values for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    /* LED as diagnostic output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

/* ============================================================================
 * Public API: updateEgtmAtom3phInvDuty
 * Behavior: increment duty by fixed step with wrap at 100%, apply immediate vector update
 * ==========================================================================*/
void updateEgtmAtom3phInvDuty(void)
{
    uint8 i;
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init not done */
    }

    /* Detect if previous update missed the last PWM period */
    if (s_lastAppliedEpoch != s_updateEpoch)
    {
        g_egtmAtom3phInvStatus.missedUpdates++;
    }

    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP;
        if (next >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f; /* wrap */
        }
        else
        {
            g_egtmAtom3phInv.dutyCycles[i] = next;
        }
    }

    /* Apply new duties immediately as a vector (all channels together) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);

    /* Mark as applied in current epoch snapshot */
    s_lastAppliedEpoch = s_updateEpoch;
}
