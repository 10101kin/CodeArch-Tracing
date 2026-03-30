/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx)
 * - Unified IfxEgtm_Pwm driver
 * - ATOM0 Cluster 0, 3 complementary channels, center-aligned, 20 kHz
 * - Dead-time: 1 us rising/falling
 * - Initial duties: U=25%, V=50%, W=75%
 * - Duty update: +10% with wrap-around
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =========================
 * Configuration Macros
 * ========================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED for ISR/period indication: P03.9 (compound macro: port, pin) */
#define LED                        &MODULE_P03, 9

/* =========================
 * Pin Routing Macros (Validated Pin Symbols Only)
 * Note: User-requested pins that are not present in the validated list
 * are left as NULL_PTR placeholders to be replaced during integration.
 * ========================= */
/* U phase: HS=P20.8 (TOUT64), LS=P20.9 (TOUT65) */
#define PHASE_U_HS   (NULL_PTR) /* Replace during integration with the pin-map symbol for P20.8 / TOUT64 */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* V phase: HS=P21.4 (TOUT55), LS=P20.11 (TOUT67) */
#define PHASE_V_HS   (NULL_PTR) /* Replace during integration with the pin-map symbol for P21.4 / TOUT55 */
#define PHASE_V_LS   (NULL_PTR) /* Replace during integration with the pin-map symbol for P20.11 / TOUT67 */

/* W phase: HS=P20.12 (TOUT68), LS=P20.13 (TOUT69) */
#define PHASE_W_HS   (NULL_PTR) /* Replace during integration with the pin-map symbol for P20.12 / TOUT68 */
#define PHASE_W_LS   (NULL_PTR) /* Replace during integration with the pin-map symbol for P20.13 / TOUT69 */

/* =========================
 * Module State
 * ========================= */
typedef struct
{
    IfxEgtm_Pwm                 pwm;                                 /* driver handle */
    IfxEgtm_Pwm_Channel         channels[NUM_OF_CHANNELS];           /* persistent channel objects */
    float32                     dutyCycles[NUM_OF_CHANNELS];         /* percent [0..100) */
    float32                     phases[NUM_OF_CHANNELS];             /* phase offsets in fraction [0..1) */
    IfxEgtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];          /* stored dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* =========================
 * Internal period-event callback (assigned via InterruptConfig)
 * Spec requires this callback to toggle LED each PWM period.
 * ========================= */
void interruptEgtmAtomPeriod(void)
{
    IfxPort_togglePin(LED);
}

/* =========================
 * Public Functions
 * ========================= */

/**
 * Initialize a 3-channel complementary, center-aligned PWM on eGTM ATOM0 Cluster 0
 * - 20 kHz base frequency
 * - 1 us rising/falling dead-time
 * - U/V/W initial duty = 25%/50%/75%
 * - Complementary outputs using OutputConfig array
 * - Interrupt on base channel with priority 20, CPU0 provider, toggling LED P03.9
 */
void initEgtmAtom3phInv(void)
{
    /* (1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* (2) Load default values */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* (3) Configure complementary output pins for each logical channel */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS; /* high-side */
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS; /* low-side  */
    output[0].polarity              = Ifx_ActiveState_high;              /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;               /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* (4) Dead-time configuration */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* (5) Interrupt configuration: base channel only */
    irqCfg.mode        = IfxEgtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = interruptEgtmAtomPeriod;
    irqCfg.dutyEvent   = NULL_PTR;

    /* (6) Channel configurations: logical indices 0..2, phase=0, duties U/V/W */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;             /* no MSC connection */
    channelConfig[0].interrupt = &irqCfg;              /* base channel interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;             /* only base channel has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* (7) Main PWM configuration */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;        /* ATOM uses CMU CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* (8) eGTM enable guard + CMU clock configuration (inside guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        {
            float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
            IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
            IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
            IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
        }
    }

    /* (9) Initialize PWM driver with persistent state */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* (10) Store initial duties, phases, and dead-times in module state */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* (11) Configure LED GPIO as push-pull output (no set-high/low at init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update three phase duties in 10% steps with wrap-around, then apply immediately
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* wrap-around rule: if (duty + STEP) >= 100 => reset to 0, then always add STEP */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately using unified PWM driver */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
