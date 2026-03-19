/*
 * egtm_atom_3_phase_inverter_pwm.c
 * TC4xx (TC387 board), eGTM ATOM 3-phase inverter PWM using unified IfxEgtm_Pwm driver
 *
 * Notes:
 * - Uses unified IfxEgtm_Pwm high-level driver per iLLD patterns
 * - EGTM CMU clock enable block is mandatory before init
 * - Pin routing via OutputConfig; DO NOT call IfxEgtm_PinMap_setAtomTout() with unified driver
 * - Watchdog APIs must NOT be used here (only in CpuN_Main.c files)
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ===============================
 * Pin selection placeholders (TBD)
 * ===============================
 * Per requirements, TOUT pin mappings are TBD pending board availability.
 * With the unified driver, routing is handled from OutputConfig.pin fields.
 * Set them to NULL_PTR for now; board integration shall replace with valid
 * (IfxEgtm_ATOMx_y[_N]_TOUTz_Paa_b_OUT) symbols from IfxEgtm_PinMap.h
 */
#define PHASE_U_HS   NULL_PTR
#define PHASE_U_LS   NULL_PTR
#define PHASE_V_HS   NULL_PTR
#define PHASE_V_LS   NULL_PTR
#define PHASE_W_HS   NULL_PTR
#define PHASE_W_LS   NULL_PTR

/* ===============================
 * Globals
 * =============================== */
EgtmAtom3phInv g_egtmAtom3phInv;  /* Zero-initialized by C startup */

/* ===============================
 * ISR declaration (TC4xx): use EXACT macro name ISR_PRIORITY_ATOM
 * =============================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period event callback (invoked from driver's interrupt handler) */
void IfxEgtm_periodEventFunction(void *data)
{
    EgtmAtom3phInv *ctx = (EgtmAtom3phInv *)data;
    if (ctx != NULL_PTR)
    {
        ctx->status.isrCount++;
    }
}

/* ISR definition: call driver's interrupt handler then toggle LED */
void interruptEgtmAtom(void)
{
    /* Period interrupt is linked to channel 0 via channelConfig[0].interrupt */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], (void *)&g_egtmAtom3phInv);
    IfxPort_togglePin(LED);
}

void initEgtmAtom3phInv(void)
{
    /* Status defaults */
    g_egtmAtom3phInv.status.lastError      = EgtmAtom3phInv_Error_none;
    g_egtmAtom3phInv.status.isrCount       = 0U;
    g_egtmAtom3phInv.status.missedUpdates  = 0U;
    g_egtmAtom3phInv.lastAppliedEpoch      = 0U;

    /* ===============================
     * Mandatory eGTM enable + CMU clock setup (TC4xx)
     * =============================== */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }
    {
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Drive GCLK from module clock (no divide) and use as input for CLK0 */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1U, 1U);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE); /* useGlobal = TRUE */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* ===============================
     * Unified PWM configuration
     * =============================== */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* Initialize config with defaults and module reference */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration per channel (complementary, HS active-high, LS active-low) */
    /* U phase */
    output[0].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;  /* DTM_OUT0 -> HS */
    output[0].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;  /* DTM_OUT1 -> LS */
    output[0].polarity                   = Ifx_ActiveState_high;
    output[0].complementaryPolarity      = Ifx_ActiveState_low;
    output[0].outputMode                 = IfxPort_OutputMode_pushPull;
    output[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                   = Ifx_ActiveState_high;
    output[1].complementaryPolarity      = Ifx_ActiveState_low;
    output[1].outputMode                 = IfxPort_OutputMode_pushPull;
    output[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                        = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin           = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                   = Ifx_ActiveState_high;
    output[2].complementaryPolarity      = Ifx_ActiveState_low;
    output[2].outputMode                 = IfxPort_OutputMode_pushPull;
    output[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us rising and falling for each channel */
    dtmConfig[0].deadTime.rising         = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling        = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising         = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling        = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising         = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling        = DEAD_TIME_SEC;

    /* Interrupt configuration: period event on CPU0, priority 20 (ISR_PRIORITY_ATOM) */
    interruptConfig.mode                 = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider          = IfxSrc_Tos_cpu0;
    interruptConfig.priority             = ISR_PRIORITY_ATOM;
    interruptConfig.vmId                 = IfxSrc_VmId_0;
    interruptConfig.periodEvent          = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent            = NULL_PTR;

    /* Per-channel configuration: ATOM2 channels 4/5/6, phase=0, duty=50% */
    /* U phase -> CH4 */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    channelConfig[0].timerCh             = IfxEgtm_Pwm_SubModule_Ch_4;
    channelConfig[0].phase               = 0.0f;
    channelConfig[0].duty                = PHASE_U_DUTY;
    channelConfig[0].dtm                 = &dtmConfig[0];
    channelConfig[0].output              = &output[0];
    channelConfig[0].mscOut              = NULL_PTR;
    channelConfig[0].interrupt           = &interruptConfig;  /* bind IRQ to base channel */

    /* V phase -> CH5 */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    channelConfig[1].timerCh             = IfxEgtm_Pwm_SubModule_Ch_5;
    channelConfig[1].phase               = 0.0f;
    channelConfig[1].duty                = PHASE_V_DUTY;
    channelConfig[1].dtm                 = &dtmConfig[1];
    channelConfig[1].output              = &output[1];
    channelConfig[1].mscOut              = NULL_PTR;
    channelConfig[1].interrupt           = NULL_PTR;

    /* W phase -> CH6 */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);
    channelConfig[2].timerCh             = IfxEgtm_Pwm_SubModule_Ch_6;
    channelConfig[2].phase               = 0.0f;
    channelConfig[2].duty                = PHASE_W_DUTY;
    channelConfig[2].dtm                 = &dtmConfig[2];
    channelConfig[2].output              = &output[2];
    channelConfig[2].mscOut              = NULL_PTR;
    channelConfig[2].interrupt           = NULL_PTR;

    /* Top-level PWM configuration */
    config.cluster                       = IfxEgtm_Cluster_0;
    config.subModule                     = IfxEgtm_Pwm_SubModule_atom;
    config.alignment                     = IfxEgtm_Pwm_Alignment_center;    /* center-aligned */
    config.syncStart                     = TRUE;                             /* auto-start after init */
    config.syncUpdateEnabled             = TRUE;                             /* shadow updates */
    config.numChannels                   = NUM_OF_CHANNELS;
    config.channels                      = &channelConfig[0];
    config.frequency                     = PWM_FREQUENCY;
    config.clockSource.atom              = IfxEgtm_Cmu_Clk_0;                /* ATOM uses Clk enum */
    config.dtmClockSource                = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Initialize PWM driver (returns void; unified driver applies all settings) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initial runtime copies */
    g_egtmAtom3phInv.dutyCycles[0]       = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1]       = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2]       = channelConfig[2].duty;
    g_egtmAtom3phInv.phases[0]           = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]           = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]           = channelConfig[2].phase;
    g_egtmAtom3phInv.deadTimes[0]        = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]        = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]        = channelConfig[2].dtm->deadTime;

    /* Configure LED pin for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* Install ISR in vector table (priority matches interruptConfig.priority) */
    IfxCpu_Irq_installInterruptHandler((void *)interruptEgtmAtom, ISR_PRIORITY_ATOM);

    /* Do NOT call redundant APIs after init (startSyncedChannels/startChannelOutputs/etc.) */
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Missed-update detection: if no new period occurred since last apply */
    if (g_egtmAtom3phInv.lastAppliedEpoch == g_egtmAtom3phInv.status.isrCount)
    {
        g_egtmAtom3phInv.status.missedUpdates++;
    }

    /* Cyclic increment with wrap at 100% for all three phases */
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

    /* Apply new duties immediately as a vector (unified driver updates synchronously) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);

    /* Stamp the epoch at which this update was requested */
    g_egtmAtom3phInv.lastAppliedEpoch = g_egtmAtom3phInv.status.isrCount;
}
