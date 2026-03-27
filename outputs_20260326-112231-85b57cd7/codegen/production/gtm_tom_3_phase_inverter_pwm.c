/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: 3-phase complementary PWM on GTM TOM using unified IfxGtm_Pwm
 *
 * Notes:
 * - Follows iLLD initialization pattern for GTM CMU clocking and IfxGtm_Pwm driver.
 * - Uses complementary outputs with 1us hardware DTM dead-time, center-aligned at 20kHz.
 * - Provides a PWM period callback (empty) and a minimal ATOM ISR that toggles an LED.
 * - No watchdog operations are performed in this driver (CPU file responsibility).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================================================
 * Macros and configuration constants (from requirements)
 * ======================================================== */
#define NUM_OF_CHANNELS           (3)
#define PWM_FREQUENCY_HZ          (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM         (20)

/* LED pin macro (port, pin) */
#define LED                       &MODULE_P13, 0

/* Initial duty percent values and duty step (percent domain) */
#define PHASE_U_DUTY_INIT         (25.0f)
#define PHASE_V_DUTY_INIT         (50.0f)
#define PHASE_W_DUTY_INIT         (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/*
 * Complementary TOM1 Cluster_1 mapping on Port 02 (verify per target board):
 * High-side uses TOM1_x_TOUTx_P02_y_OUT; Low-side uses TOM1_xN_TOUTz_P02_y_OUT
 * These symbols are provided by the iLLD PinMap. Adjust if board routing differs.
 */
#define PHASE_U_HS                (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                (&IfxGtm_TOM1_1N_TOUT4_P02_4_OUT)
#define PHASE_W_HS                (&IfxGtm_TOM1_2_TOUT2_P02_2_OUT)
#define PHASE_W_LS                (&IfxGtm_TOM1_2N_TOUT5_P02_5_OUT)

/* ========================================================
 * Module state
 * ======================================================== */

typedef struct
{
    IfxGtm_Pwm           pwm;                              /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];        /* persistent channel storage */
    float32              dutyCycles[NUM_OF_CHANNELS];      /* duty in percent */
    float32              phases[NUM_OF_CHANNELS];          /* phase in percent/deg if used */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];       /* stored dead-time values */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_state;

/* ========================================================
 * Forward declarations (ISR and period callback)
 * ======================================================== */

/* Minimal ISR: toggles LED; priority macro used in declaration. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Keep ISR very short: toggle LED only */
    IfxPort_togglePin(LED);
}

/* Empty PWM period callback as required */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty */
}

/* ========================================================
 * Public API implementation
 * ======================================================== */

/*
 * Initialize 3-phase complementary PWM on TOM1 with center alignment and hardware dead-time.
 * - Uses IfxGtm_Pwm unified driver
 * - Sync start and sync update enabled
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Load driver defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary HS/LS pairs, active-high HS, active-low LS */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;         /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;          /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1us rising and falling dead-time for all channels */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration: base channel only, pulse notify, CPU0, priority 20 */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;  /* required empty callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;                /* base channel only */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;               /* base-channel-only pattern */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;               /* base-channel-only pattern */

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                 /* TOM1 cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;                              /* sync start */
    config.syncUpdateEnabled    = TRUE;                              /* sync update */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock source */

    /* 8) GTM enable and CMU setup guard block */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 frequency;
        IfxGtm_enable(&MODULE_GTM);
        frequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, frequency);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, frequency);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver with persistent channels storage */
    IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

    /* Store initial state copies (applied by init already) */
    g_state.dutyCycles[0]   = channelConfig[0].duty;
    g_state.dutyCycles[1]   = channelConfig[1].duty;
    g_state.dutyCycles[2]   = channelConfig[2].duty;
    g_state.phases[0]       = channelConfig[0].phase;
    g_state.phases[1]       = channelConfig[1].phase;
    g_state.phases[2]       = channelConfig[2].phase;
    g_state.deadTimes[0]    = channelConfig[0].dtm->deadTime;
    g_state.deadTimes[1]    = channelConfig[1].dtm->deadTime;
    g_state.deadTimes[2]    = channelConfig[2].dtm->deadTime;

    /* 10) Configure LED GPIO as push-pull output; initial state kept as default */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* 11) Periodic ATOM0 CH0 2Hz interrupt for LED toggle:
     * This driver declares the ISR (interruptGtmAtom). The timer/SRC setup is expected
     * to be handled by system-level initialization per project policy.
     */
}

/*
 * Percent-based duty stepper: +10% with wrap rule, update all three channels together.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule checks (three separate if-blocks) */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[0] = 0.0f; }
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[1] = 0.0f; }
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_state.dutyCycles[2] = 0.0f; }

    /* Unconditional add after potential reset */
    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply as a synchronized immediate update (percent domain) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
