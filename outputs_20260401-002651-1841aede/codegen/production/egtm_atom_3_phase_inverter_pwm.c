/*
 * egtm_atom_3_phase_inverter_pwm.c
 * Production driver: EGTM ATOM0, Cluster 0, 3-phase complementary center-aligned PWM at 20 kHz
 * - TC4xx migration of GTM TOM -> EGTM ATOM using IfxEgtm_Pwm unified driver
 * - Pins: P20.8..P20.13 (TOUT64..TOUT69)
 * - Dead-time: 1 us rising/falling
 * - Initial duties: U=25%, V=50%, W=75%
 * - Period ISR (CPU0, prio 20) toggles LED P03.9
 *
 * Notes:
 * - Watchdog disable must NOT be placed here (only in CpuX_Main.c per project standard)
 * - No STM timing logic in this driver; scheduling belongs in CpuX_Main.c
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros (Numerical values) ========================= */
#define NUM_OF_CHANNELS          (3u)
#define PWM_FREQUENCY            (20000.0f)      /* Hz */
#define ISR_PRIORITY_ATOM        (20)
#define PHASE_U_DUTY             (25.0f)         /* percent */
#define PHASE_V_DUTY             (50.0f)         /* percent */
#define PHASE_W_DUTY             (75.0f)         /* percent */
#define PHASE_DUTY_STEP          (10.0f)         /* percent step */

/* LED P03.9: use compound macro so it expands to (port, pin) for GPIO APIs */
#define LED                      &MODULE_P03, 9

/* ========================= Pin Macros (TOUT mapping) ========================= */
/* User-requested pin assignments (validated for TC4xx): */
#define PHASE_U_HS_PIN           (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS_PIN           (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS_PIN           (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS_PIN           (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS_PIN           (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS_PIN           (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)

/* ========================= Module State ========================= */
typedef struct
{
    IfxEgtm_Pwm               pwm;                                 /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_OF_CHANNELS];           /* Persistent channel objects */
    float32                   dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent */
    float32                   phases[NUM_OF_CHANNELS];             /* Phase offset in percent/cycle */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];          /* Dead-times (rising/falling in seconds) */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ========================= ISR and Callback (defined before init) ========================= */
/* Declare ISR with vector table assignment; implementation follows. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

/* ========================= Public API Implementations ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all config structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load defaults into the main config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate complementary output configuration for phases U, V, W */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS_PIN;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS_PIN;
    output[0].polarity              = Ifx_ActiveState_high;            /* HS active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;             /* LS active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS_PIN;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS_PIN;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS_PIN;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS_PIN;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM dead-time configuration: 1 us rising and falling for each phase */
    dtmConfig[0].deadTime.rising  = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration for PWM-period notification */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2, phase=0.0, initial duties U,V,W */
    /* Base channel (interrupt assigned only here) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM uses CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock 0 */
    config.syncUpdateEnabled  = TRUE;                                /* shadow-to-active at period end */
    config.syncStart          = TRUE;                                /* start all channels together */

    /* 8) Enable-guard: enable EGTM and configure clocks (GCLK, CLK0, FXCLK) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver (applies pin routing, timing, dead-time, polarity, ISR routing) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initial duties, phases, and dead-times into module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO as push-pull output (no forced level) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Step duties with wrap at 100% using the mandated sequence (no loop) */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phInv.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply duty updates atomically at the shadow-to-active transfer point */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
