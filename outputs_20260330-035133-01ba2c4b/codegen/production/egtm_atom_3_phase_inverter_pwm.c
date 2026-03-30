/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for TC4xx eGTM ATOM 3-phase complementary PWM using IfxEgtm_Pwm.
 *
 * - Submodule: ATOM0, Cluster 0
 * - Alignment: Center-aligned
 * - Frequency: 20 kHz
 * - 3 complementary pairs with 1 us rising/falling dead-time
 * - Initial duties: U=25%, V=50%, W=75%
 * - Duty step on update: 10%
 * - Interrupt: period-event, priority 20, provider CPU0, toggles LED P03.9
 *
 * Notes:
 * - No watchdog disable calls here (must be in CpuX_Main.c only).
 * - No STM timing here; scheduling belongs to CpuX_Main.c.
 * - Persistent channels[] stored in module state because driver keeps references.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================
 * Configuration macros
 * ========================== */
#define NUM_OF_CHANNELS     (3U)
#define PWM_FREQUENCY       (20000.0f)            /* 20 kHz */
#define ISR_PRIORITY_ATOM   (20)

#define PHASE_U_DUTY        (25.0f)
#define PHASE_V_DUTY        (50.0f)
#define PHASE_W_DUTY        (75.0f)
#define PHASE_DUTY_STEP     (10.0f)

/* LED P03.9 compound macro for IfxPort_* calls (port, pin) */
#define LED                 &MODULE_P03, 9

/* ==========================
 * Pin routing macros (user-requested pins)
 * Use validated eGTM ATOM TOUT symbols for TC4xx.
 * ========================== */
#define PHASE_U_HS  (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS  (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS  (&IfxEgtm_ATOM0_1_TOUT55_P21_4_OUT)
#define PHASE_V_LS  (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS  (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS  (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ==========================
 * Module state
 * ========================== */
typedef struct
{
    IfxEgtm_Pwm            pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel    channels[NUM_OF_CHANNELS];         /* persistent channel storage */
    float32                dutyCycles[NUM_OF_CHANNELS];        /* duty in percent */
    float32                phases[NUM_OF_CHANNELS];            /* phase in percent (0..100) */
    IfxEgtm_Pwm_DeadTime   deadTimes[NUM_OF_CHANNELS];         /* stored dead-times (s) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ==========================
 * ISR and period-event callback
 * ========================== */

/*
 * Period-event ISR for eGTM ATOM PWM
 * Body must be minimal (toggle debug LED only).
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback registered in InterruptConfig.
 * Must take void *data and have an empty body.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ==========================
 * Public API
 * ========================== */

/** \brief Initialize 3-phase complementary PWM on eGTM ATOM0 Cluster 0
 *
 * Follows the unified IfxEgtm_Pwm initialization pattern and TC4xx migration requirements.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure complementary outputs: HS active-high, LS active-low, push-pull, cmosAutomotiveSpeed1 */
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

    /* 4) Configure DTM dead-time: 1 us rising, 1 us falling per channel */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure period-event interrupt (pulse notify on CPU0, priority 20), dutyEvent unused */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2), initial phases and duties, attach dtm/output */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;              /* base channel gets the interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;            /* no interrupt on this channel */

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;            /* no interrupt on this channel */

    /* 7) Complete main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;                /* Hz */
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;            /* ATOM channel clock from CMU CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock 0 */
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;

    /* 8) eGTM enable guard and CMU clock configuration */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);                       /* GCLK pass-through */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);     /* ATOM CLK0 */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial duties, phases, and dead-times in module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (do not drive level here) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/** \brief Update the duty cycle of all three phases by +10% with wrap at 100%.
 *
 * Implements atomic multi-channel immediate update using the unified driver.
 */
void updateEgtmAtom3phInvDuty(void)
{
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
