/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Migration driver: IfxGtm_Tom_PwmHl -> IfxGtm_Pwm (TOM1, Cluster_1)
 *  - 3-phase complementary PWM, center-aligned, 20 kHz, 1 us dead time
 *  - Synchronous start and synchronous update
 *  - LED P13.0 toggled from ATOM ISR (not installed here)
 *
 * Notes:
 *  - Followed iLLD unified PWM initialization pattern and enable-guard for GTM/CMU.
 *  - No watchdog handling here (must be in CpuX_Main.c only).
 *  - Pin TOUT macros are left as NULL_PTR placeholders and must be replaced during board integration.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and Configuration Constants ========================= */
#define GTM_TOM3PH_NUM_CHANNELS     (3u)
#define PWM_FREQUENCY_HZ            (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM           (20u)

/* User-requested pins (P02.x) — replace NULL_PTR with validated TOUT symbols during integration */
#define PHASE_U_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                  (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_5_OUT */

/* Initial duties and step (percent) */
#define PHASE_U_DUTY_INIT           (25.0f)
#define PHASE_V_DUTY_INIT           (50.0f)
#define PHASE_W_DUTY_INIT           (75.0f)
#define PHASE_DUTY_STEP             (10.0f)

/* LED P13.0 compound macro (port, pin) */
#define LED                         &MODULE_P13, 0

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm               pwm;                                       /* PWM driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM3PH_NUM_CHANNELS];         /* persistent channel handles */
    float32                  dutyCycles[GTM_TOM3PH_NUM_CHANNELS];       /* U,V,W duty in percent */
    float32                  phases[GTM_TOM3PH_NUM_CHANNELS];           /* phase shift in degrees (unused -> 0.0f) */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM3PH_NUM_CHANNELS];        /* configured dead times */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3ph;

/* ========================= Private Prototypes ========================= */

/* ISR: declared with required macro; not installed from this module */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Unified PWM period-event callback: empty body as required */
void IfxGtm_periodEventFunction(void *data);

/* ========================= Private Implementations ========================= */

void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle LED only */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* empty body by design */
}

/* ========================= Public API ========================= */

/**
 * Initialize 3-phase complementary, center-aligned PWM on TOM1, Cluster_1 at 20 kHz with 1 us dead time.
 * Configures synchronous start/update and prepares LED P13.0 as push-pull output (default LOW).
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[GTM_TOM3PH_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* 2) Populate main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: map 3 complementary pairs (U,V,W) */
    /* U phase */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  /* P02.0 */
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  /* P02.7 */
    output[0].polarity              = Ifx_ActiveState_high;              /* high-side active HIGH */
    output[0].complementaryPolarity = Ifx_ActiveState_low;               /* low-side  active LOW  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  /* P02.1 */
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;  /* P02.4 */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  /* P02.2 */
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;  /* P02.5 */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1 us both edges for each channel */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 4) Interrupt configuration: unified period event on base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Per-channel configuration (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;   /* base channel routes callback */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;          /* only channel 0 has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.gtmSFR             = &MODULE_GTM;
    config.cluster            = IfxGtm_Cluster_1;                   /* TGC1 / Cluster_1 */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;           /* TOM */
    config.alignment          = IfxGtm_Pwm_Alignment_center;        /* center-aligned */
    config.syncStart          = TRUE;                                /* start synced */
    config.numChannels        = (uint8)GTM_TOM3PH_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;                    /* 20 kHz */
    config.clockSource.atom   = IfxGtm_Cmu_Fxclk_0;                  /* use FXCLK0 */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;    /* DTM clock */
    config.syncUpdateEnabled  = TRUE;                                /* sync update */

    /* 5) GTM enable guard + CMU clocks (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Apply configuration: init PWM (routes pins, applies complementary + dead-time) */
    IfxGtm_Pwm_init(&g_gtmTom3ph.pwm, &g_gtmTom3ph.channels[0], &config);

    /* 7) Initialize persistent state (duties, phases, dead times) */
    g_gtmTom3ph.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3ph.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3ph.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3ph.phases[0] = 0.0f;
    g_gtmTom3ph.phases[1] = 0.0f;
    g_gtmTom3ph.phases[2] = 0.0f;

    g_gtmTom3ph.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3ph.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3ph.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) LED GPIO: push-pull output, default LOW */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);
}

/**
 * Update U,V,W duties with wrap rule and apply immediately in a synchronized manner.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + STEP) >= 100 -> duty = 0; then add STEP (always) */
    if ((g_gtmTom3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3ph.dutyCycles[2] = 0.0f; }

    g_gtmTom3ph.dutyCycles[0] += PHASE_DUTY_STEP;  /* U */
    g_gtmTom3ph.dutyCycles[1] += PHASE_DUTY_STEP;  /* V */
    g_gtmTom3ph.dutyCycles[2] += PHASE_DUTY_STEP;  /* W */

    /* Apply immediately (synchronized across configured channels) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3ph.pwm, (float32 *)g_gtmTom3ph.dutyCycles);
}
