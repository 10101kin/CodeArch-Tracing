/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3‑Phase Inverter PWM (TC3xx)
 *
 * - Submodule: TOM (Cluster 1)
 * - 3 complementary channels (U, V, W)
 * - Center aligned, 20 kHz
 * - Dead-time from migration config
 * - Synchronous start and updates enabled
 * - Period callback stub (IfxGtm_periodEventFunction)
 * - Minimal ISR toggles LED P13.0
 */

/* Includes (only selected iLLD headers and own header) */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ========================= Macros and Constants ========================= */

/* Channel count */
#define NUM_OF_CHANNELS            (3U)

/* PWM frequency (Hz) — user-confirmed migration value */
#define PWM_FREQUENCY              (20000.0f)

/* Dead-time (seconds) — user-confirmed migration value (overrides defaults) */
#define PWM_DEAD_TIME              (5.0e-7f)

/* Initial duties (percent) — user-confirmed migration values */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty step and limits (percent) — user-confirmed migration values */
#define PHASE_DUTY_STEP            (10.0f)
#define PHASE_DUTY_MIN             (10.0f)
#define PHASE_DUTY_MAX             (90.0f)

/* LED (P13.0) compound macro: provides (port, pin) */
#define LED                        &MODULE_P13, 0

/* ISR priority (matches InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM          (20)

/*
 * TOUT pin routing placeholders — replace NULL_PTR with valid pin symbols from IfxGtm_PinMap headers
 * when available (do NOT invent symbol names in this module).
 * Required mappings (user requested):
 *  - U: P02.0 (HS), P02.7 (LS)
 *  - V: P02.1 (HS), P02.4 (LS)
 *  - W: P02.2 (HS), P02.5 (LS)
 */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTy_P02_5_OUT */

/* ========================= Module State ========================= */

typedef struct
{
    IfxGtm_Pwm               pwm;                               /* PWM handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];         /* Persistent channels (driver uses this memory) */
    float32                  dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];         /* Stored dead-times (s) */
    float32                  phases[NUM_OF_CHANNELS];            /* Phase in degrees (or fraction, driver-config use) */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv = {0};

/* ========================= ISR and Callback Prototypes ========================= */

/* ISR declaration with priority macro; provider index 0 (cpu0) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/* Period-event callback stub used by high-level PWM driver */
void IfxGtm_periodEventFunction(void *data);

/* ========================= ISR and Callback Definitions ========================= */

void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle LED and return */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    /* Intentionally empty: required by high-level driver as a period-event callback */
    (void)data;
}

/* ========================= Public API Implementations ========================= */

/*
 * initGtmTom3phInv
 * Configure GTM TOM PWM for 3-phase inverter with complementary outputs.
 * Follows the authoritative IfxGtm_Pwm init pattern and union clockSource.tom setup.
 */
void initGtmTom3phInv(void)
{
    /* Local configuration objects (per SW detailed design) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* Initialize main PWM config with defaults using GTM SFR */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output Configuration: complementary pairs, push-pull, automotive pad driver */
    /* Channel 0 → Phase U */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;  /* High-side */
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;  /* Low-side  */
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 1 → Phase V */
    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel 2 → Phase W */
    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM (dead-time) configuration: rising and falling */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME;

    /* Base channel interrupt configuration (assigned to channel 0 only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction; /* distinct from ISR */
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations (logical indices 0..2) */
    /* CH0 → Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* only base channel gets interrupt config */

    /* CH1 → Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* CH2 → Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                     /* Use Cluster 1 as requested */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;             /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;          /* Center-aligned */
    config.syncStart            = TRUE;                                  /* Synchronous start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                        /* 20 kHz */
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;           /* Set TOM clock source (union field) */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;   /* DTM clock source */
    config.syncUpdateEnabled    = TRUE;                                  /* Synchronous updates */

    /* GTM enable guard + CMU configuration (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* Initialize PWM with persistent channels storage */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* Store initial state for runtime updates */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    g_gtmTom3phInv.phases[0]     = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]     = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]     = channelConfig[2].phase;

    /* Configure LED GPIO for ISR toggling (P13.0) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * updateGtmTom3phInvDuty
 * Step each phase duty by fixed increment with wrap at limits, then apply immediately.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Phase U */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= PHASE_DUTY_MAX)
    {
        g_gtmTom3phInv.dutyCycles[0] = PHASE_DUTY_MIN;
    }
    else
    {
        g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    }

    /* Phase V */
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= PHASE_DUTY_MAX)
    {
        g_gtmTom3phInv.dutyCycles[1] = PHASE_DUTY_MIN;
    }
    else
    {
        g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    }

    /* Phase W */
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= PHASE_DUTY_MAX)
    {
        g_gtmTom3phInv.dutyCycles[2] = PHASE_DUTY_MIN;
    }
    else
    {
        g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;
    }

    /* Apply changes immediately and synchronously */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
