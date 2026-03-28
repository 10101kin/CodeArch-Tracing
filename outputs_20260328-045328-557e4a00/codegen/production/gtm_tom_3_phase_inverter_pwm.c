/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm
 * - Center-aligned, complementary outputs
 * - DTM dead-time = 1 us
 * - 20 kHz on TOM1 Cluster_1
 * - Synchronous start and update
 * - Period-event callback stub and minimal ISR that toggles a debug LED
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================ Macros ============================ */
#define G3PHINV_NUM_CHANNELS          (3u)
#define G3PHINV_PWM_FREQUENCY_HZ      (20000.0f)
#define ISR_PRIORITY_ATOM             (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT             (25.0f)
#define PHASE_V_DUTY_INIT             (50.0f)
#define PHASE_W_DUTY_INIT             (75.0f)
#define PHASE_DUTY_STEP               (10.0f)

/* LED: compound macro (port, pin) */
#define LED                           &MODULE_P13, 0

/*
 * Pin routing macros (TOUT maps).
 * Note: No validated pin symbols were provided in the context for TOM1 on TC3xx.
 * To keep the driver portable and compilable without board-specific pin-map headers,
 * the pin pointers are set to NULL here. Replace with valid IfxGtm_TOM1_*_TOUT*_Pxx_y_OUT
 * symbols for your target board/package when integrating.
 */
#define PHASE_U_HS    ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_U_LS    ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_HS    ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_LS    ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_HS    ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_LS    ((IfxGtm_Pwm_ToutMap*)0)

/* ============================ Module State ============================ */
typedef struct
{
    IfxGtm_Pwm                 pwm;                                        /* PWM handle */
    IfxGtm_Pwm_Channel         channels[G3PHINV_NUM_CHANNELS];             /* Persistent channels array (driver keeps pointer) */
    float32                    dutyCycles[G3PHINV_NUM_CHANNELS];           /* duty in percent [0..100] */
    float32                    phases[G3PHINV_NUM_CHANNELS];               /* phase in fraction/degree as configured (stored for state trace) */
    IfxGtm_Pwm_DeadTime        deadTimes[G3PHINV_NUM_CHANNELS];            /* per-channel dead-time settings */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ============================ Internal ISR/Callback ============================ */
/* Period-event callback: empty body as required. Must be visible (no static). */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR declaration and implementation: toggles the LED pin only. */
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

/* ============================ Public API ============================ */
/*
 * Initialize the GTM TOM 3-phase inverter PWM using IfxGtm_Pwm
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration objects as locals (setup only) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_InterruptConfig  irqCfg;
    IfxGtm_Pwm_ChannelConfig    channelConfig[G3PHINV_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outputCfg[G3PHINV_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[G3PHINV_NUM_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output pin routing and polarity for complementary outputs */
    /* Phase U */
    outputCfg[0].pin                     = PHASE_U_HS;          /* high-side */
    outputCfg[0].complementaryPin        = PHASE_U_LS;          /* low-side  */
    outputCfg[0].polarity                = Ifx_ActiveState_high;/* HS active HIGH */
    outputCfg[0].complementaryPolarity   = Ifx_ActiveState_low; /* LS active LOW  */
    outputCfg[0].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase V */
    outputCfg[1].pin                     = PHASE_V_HS;
    outputCfg[1].complementaryPin        = PHASE_V_LS;
    outputCfg[1].polarity                = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[1].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Phase W */
    outputCfg[2].pin                     = PHASE_W_HS;
    outputCfg[2].complementaryPin        = PHASE_W_LS;
    outputCfg[2].polarity                = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity   = Ifx_ActiveState_low;
    outputCfg[2].outputMode              = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM/dead-time configuration (1 us on both edges, FXCLK0) */
    dtmCfg[0].deadTime.rising = 1e-6f;
    dtmCfg[0].deadTime.falling = 1e-6f;
    dtmCfg[1].deadTime.rising = 1e-6f;
    dtmCfg[1].deadTime.falling = 1e-6f;
    dtmCfg[2].deadTime.rising = 1e-6f;
    dtmCfg[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration (period event on base channel only) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: indices 0..2, initial duties 25/50/75 % */
    /* Ch 0 - Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;  /* percent */
    channelConfig[0].dtm        = &dtmCfg[0];
    channelConfig[0].output     = &outputCfg[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;            /* base channel period event */

    /* Ch 1 - Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmCfg[1];
    channelConfig[1].output     = &outputCfg[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;           /* only base channel uses interrupt */

    /* Ch 2 - Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmCfg[2];
    channelConfig[2].output     = &outputCfg[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Complete main configuration */
    config.gtmSFR             = &MODULE_GTM;
    config.cluster            = IfxGtm_Cluster_1;                       /* TOM1 Cluster_1 */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;               /* use TOM */
    config.alignment          = IfxGtm_Pwm_Alignment_center;            /* center aligned */
    config.syncStart          = TRUE;                                   /* sync start at init end */
    config.syncUpdateEnabled  = TRUE;                                   /* buffered update */
    config.numChannels        = (uint8)G3PHINV_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = G3PHINV_PWM_FREQUENCY_HZ;               /* 20 kHz */
    /* Use first FXCLK (Clk_0) for PWM clock */
    config.clockSource        = (IfxGtm_Pwm_ClockSource)IfxGtm_Cmu_Clk_0;
    /* DTM clock source tied to CMU Clock0 */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) Enable guard: enable GTM and CMU clocks dynamically if not yet enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM with persistent handle and persistent channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store configured duties/phases/deadTimes into module state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.phases[0] = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1] = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2] = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0] = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmCfg[2].deadTime;

    /* 11) Configure debug LED as push-pull output (after PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three phase duties in +10% steps with wrap rule and apply immediately
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 then duty = 0; then always add step */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }
    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediately to all configured outputs (pass percent array directly) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
