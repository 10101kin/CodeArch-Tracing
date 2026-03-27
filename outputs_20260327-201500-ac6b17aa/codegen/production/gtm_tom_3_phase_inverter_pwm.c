/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production 3-phase inverter PWM driver for TC3xx GTM TOM using unified IfxGtm_Pwm API.
 *
 * - Follows authoritative iLLD initialization patterns for GTM CMU and PWM.
 * - Center-aligned, synchronized start/updates, complementary HS/LS with dead-time.
 * - Period-event callback provided (empty by design). ISR wrapper provided.
 * - No watchdog handling here (must be in CpuX_Main.c).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)    /* 20 kHz */
#define ISR_PRIORITY_ATOM              (20)

/* Initial duty cycle percentages */
#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)

/* Duty update step (percentage) */
#define PHASE_DUTY_STEP                (10.0f)

/* LED/debug pin macro (port, pin) */
#define LED                            &MODULE_P13, 0

/*
 * Pin routing macros
 * Note: For production safety, use only validated/reference pin symbols.
 * The following TOM1 TOUTs on Port 00 are taken from a reference project mapping.
 * These provide three complementary HS/LS pairs on TOM1 channels.
 */
#define PHASE_U_HS                     (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS                     (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS                     (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS                     (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS                     (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS                     (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* =============================
 * Module State
 * ============================= */

typedef struct
{
    IfxGtm_Pwm                  pwm;                                /* Driver handle */
    IfxGtm_Pwm_Channel          channels[NUM_OF_CHANNELS];          /* Persistent channel handles */
    float32                     dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent */
    IfxGtm_Pwm_DeadTime         deadTimes[NUM_OF_CHANNELS];         /* Stored dead-times */
    IfxGtm_Pwm_SubModule_Ch     chIndex[NUM_OF_CHANNELS];           /* Logical channel indices */
} GtmTom3ph_StateT;

IFX_STATIC GtmTom3ph_StateT g_tom3ph;

/* =============================
 * Forward Declarations
 * ============================= */
void IfxGtm_periodEventFunction(void *data); /* Empty callback per design */

/* ISR wrapper: priority from ISR_PRIORITY_ATOM. The handler services the PWM. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
static void serviceGtmPwmInterrupt(void);

/* Optional private function (kept for migration compatibility) */
static void interruptGtmTomPwm(void);

/* =============================
 * ISR Implementations
 * ============================= */
void interruptGtmAtom(void)
{
    /* Minimal ISR: service driver interrupt. LED toggle, if any, is platform-specific and omitted. */
    IfxGtm_Pwm_interruptHandler(&g_tom3ph.channels[0], NULL_PTR);
}

static void serviceGtmPwmInterrupt(void)
{
    IfxGtm_Pwm_interruptHandler(&g_tom3ph.channels[0], NULL_PTR);
}

static void interruptGtmTomPwm(void)
{
    /* Legacy-compatible local ISR wrapper (not installed). */
    serviceGtmPwmInterrupt();
}

/* =============================
 * Period-event callback (empty)
 * ============================= */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty by design */
}

/* =============================
 * Initialization
 * ============================= */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Initialize config defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: three complementary HS/LS pairs (active-high HS, active-low LS) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* HS active-high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* LS active-low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising/falling dead time on each pair */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Period-event interrupt configuration: single base channel routing */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;         /* single-base-channel routing */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                  /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;          /* Use TOM */
    config.alignment            = IfxGtm_Pwm_Alignment_center;       /* Center-aligned */
    config.syncStart            = TRUE;                               /* Synchronous start */
    config.syncUpdateEnabled    = TRUE;                               /* Shadow-to-active at period */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    /* Clock source selections */
    config.clockSource.atom     = IfxGtm_Cmu_Clk_0;                   /* Use CLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM from CMU clock0 */

    /* 8) Enable guard: enable GTM and configure CMU clocks only if disabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver with persistent handles */
    IfxGtm_Pwm_init(&g_tom3ph.pwm, &g_tom3ph.channels[0], &config);

    /* 10) Store initial duties, dead-times, and logical channel indices in state */
    g_tom3ph.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_tom3ph.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_tom3ph.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_tom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    g_tom3ph.chIndex[0]   = channelConfig[0].timerCh;
    g_tom3ph.chIndex[1]   = channelConfig[1].timerCh;
    g_tom3ph.chIndex[2]   = channelConfig[2].timerCh;

    /* 11) Configure LED/debug pin as push-pull output for ISR debug use */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Duty update (immediate)
 * ============================= */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100, set duty = 0; then unconditional += step */
    if ((g_tom3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[0] = 0.0f; }
    if ((g_tom3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[1] = 0.0f; }
    if ((g_tom3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3ph.dutyCycles[2] = 0.0f; }

    g_tom3ph.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3ph.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3ph.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately and coherently to the synchronized group */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3ph.pwm, (float32 *)g_tom3ph.dutyCycles);
}
