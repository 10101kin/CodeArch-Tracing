/*
 * egtm_atom_3_phase_inverter_pwm.c
 * EGTM ATOM 3-Phase Inverter PWM driver (TC4xx, migration from TC3xx GTM TOM)
 *
 * Implements:
 *  - initEgtmAtom3phInv(): initializes 3-phase complementary PWM on eGTM ATOM0 Cluster 0
 *  - updateEgtmAtom3phInvDuty(): steps duties by 10% with wrap and applies immediately
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ==========================
 * Configuration Macros
 * ==========================
 */
#define NUM_OF_CHANNELS       (3u)
#define PWM_FREQUENCY         (20000.0f)          /* 20 kHz */
#define ISR_PRIORITY_ATOM     (20)

/* User-requested pin assignments (ATOM0, complementary pairs) */
#define PHASE_U_HS            &IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT   /* U high-side  */
#define PHASE_U_LS            &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT  /* U low-side   */
#define PHASE_V_HS            &IfxEgtm_ATOM0_1_TOUT55_P21_4_OUT   /* V high-side  */
#define PHASE_V_LS            &IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT /* V low-side   */
#define PHASE_W_HS            &IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT  /* W high-side  */
#define PHASE_W_LS            &IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT /* W low-side   */

/* Initial duties (percent) */
#define PHASE_U_DUTY          (25.0f)
#define PHASE_V_DUTY          (50.0f)
#define PHASE_W_DUTY          (75.0f)
#define PHASE_DUTY_STEP       (10.0f)

/* LED: P03.9 (compound form: port, pin) */
#define LED                   &MODULE_P03, 9

/* ==========================
 * Module State
 * ==========================
 */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                   /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];             /* Persistent channel storage */
    float32                  dutyCycles[NUM_OF_CHANNELS];            /* Duty in percent */
    float32                  phases[NUM_OF_CHANNELS];                /* Phase in percent of period */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];             /* Dead-time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;                   /* Zero-initialized */

/* ==========================
 * ISR and Callback
 * ==========================
 */

/*
 * ISR: toggles LED. Priority is defined by ISR_PRIORITY_ATOM.
 * The unified driver configures the SRC; user ISR performs minimal action.
 */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback assigned to InterruptConfig.periodEvent.
 * Must take a void *data parameter and have an empty body.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ==========================
 * Public API
 * ==========================
 */

/*
 * Initialize a 3-phase complementary PWM on eGTM ATOM0 Cluster 0 using the unified IfxEgtm_Pwm driver.
 * - Center-aligned 20 kHz
 * - Complementary outputs with 1 us rising/falling dead-time
 * - Initial duties U=25%, V=50%, W=75%
 * - Synchronized start and synchronized updates enabled
 * - Channel clock source: CMU Clk_0; DTM clock: CMU clock 0
 * - One period-event ISR (priority 20, CPU0) toggles LED P03.9
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures as local variables */
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure complementary outputs for three phases */
    /* Phase U */
    output[0].pin                   = PHASE_U_HS;
    output[0].complementaryPin      = PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = PHASE_V_HS;
    output[1].complementaryPin      = PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = PHASE_W_HS;
    output[2].complementaryPin      = PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time for each channel: 1 us rising and falling */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Configure period-event interrupt and assign to base channel only */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration: logical channels 0,1,2 with duties 25/50/75% */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel gets interrupt */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main config fields */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = channelConfig;
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;              /* ATOM uses CMU CLK0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;      /* DTM clock 0 */
    config.syncUpdateEnabled   = TRUE;
    config.syncStart           = TRUE;

    /* 8) Enable guard: enable eGTM and configure CMU clocks if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent channels array from module state */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Persist initial duties, phases, and dead-times in module state */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phInv.phases[0] = 0.0f;
    g_egtmAtom3phInv.phases[1] = 0.0f;
    g_egtmAtom3phInv.phases[2] = 0.0f;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (push-pull) after PWM init; do not drive level here */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Step duty cycles by 10% (wrap at 100% -> 0 then add step) and update all channels immediately.
 */
void updateEgtmAtom3phInvDuty(void)
{
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

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
