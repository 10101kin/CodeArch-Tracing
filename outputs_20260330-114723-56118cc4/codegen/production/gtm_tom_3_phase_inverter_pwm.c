/**
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * GTM TOM 3-Phase Inverter PWM driver (TC3xx migration).
 * Implements a center-aligned 3-channel complementary PWM cluster using
 * the IfxGtm_Pwm high-level driver. The code follows iLLD initialization
 * and runtime update patterns mandated by the project rules.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include <Ifx_Types.h>
#include <Gtm/Pwm/IfxGtm_Pwm.h>
#include <IfxPort.h>
#include <IfxPort_Pinmap.h>

/* Configuration macros (from requirements) */
#define NUM_OF_CHANNELS          3U
#define PWM_FREQUENCY            (20000.0f)        /* 20 kHz */
#define PWM_DEAD_TIME            (5.0e-07f)        /* 0.5 us */
#define PWM_MIN_PULSE_TIME       (1.0e-06f)        /* 1.0 us */

/* Initial duty percentages (percent units 0..100) */
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* ISR priority macro required by structural rules */
#define ISR_PRIORITY_ATOM        3

/* LED macro must be in compound form: expand to (&MODULE_P13, 0)
 * The IFX port macros (MODULE_P13) are provided by platform pinmap headers.
 * When used as IfxPort_togglePin(LED) it expands to IfxPort_togglePin(&MODULE_P13, 0);
 */
#define LED &MODULE_P13, 0

/* Pin macros: per pin-validation rules we must not invent symbols that are
 * not present in the validated pin list. The user requested specific TOM pins
 * (P00.2/P00.3/P00.4) which are not present in the validated list available
 * to this generator. Therefore we provide NULL_PTR placeholders and a
 * comment indicating the intended TOUT mapping for integration replacement.
 * Replace these NULL_PTRs with the proper &IfxGtm_TOMx_y_TOUTz_P00_x_OUT
 * symbols from your pinmap when integrating into the project if available.
 */
#define PHASE_U_HS  (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUT11_P00_2_OUT */
#define PHASE_U_LS  (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUT11_P00_2_OUT (complementary) */
#define PHASE_V_HS  (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_V_LS  (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT13_P00_4_OUT (complementary) */
#define PHASE_W_HS  (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT13_P00_4_OUT */
#define PHASE_W_LS  (NULL_PTR) /* Replace with complementary pin symbol for W */

/* Module-state (persistent) - MUST be declared with IFX_STATIC per rules */
typedef struct
{
    IfxGtm_Pwm                   pwm;                             /* PWM driver handle/state */
    IfxGtm_Pwm_Channel           channels[NUM_OF_CHANNELS];       /* persistent channel SFR descriptors */
    float32                      dutyCycles[NUM_OF_CHANNELS];    /* percent values 0..100 */
    float32                      phases[NUM_OF_CHANNELS];        /* phase offsets (radians/deg - user placeholder) */
    IfxGtm_Pwm_DeadTime          deadTimes[NUM_OF_CHANNELS];     /* dead time per channel */
    boolean                      initialized;                    /* init guard for configure function */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phState =
{
    .initialized = FALSE
};

/* Forward declarations for internal functions */
static void configurePwmChannels(uint8_t channelCount);

/* ------------------------------------------------------------------------- */
/* ISR and callbacks (must be defined BEFORE init function per rules)      */
/* ------------------------------------------------------------------------- */

/* ISR declaration (structural rule): declare using IFX_INTERRUPT then provide
 * the function implementation. The ISR body MUST be exactly: IfxPort_togglePin(LED);
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    /* Toggle a debug LED only - minimal ISR processing */
    IfxPort_togglePin(LED);
}

/* Period-event callback assigned to InterruptConfig.periodEvent.
 * MUST be visible (not static) and MUST have an empty body except for (void)data;
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ------------------------------------------------------------------------- */
/* configurePwmChannels: dual-mode function
 * - On first invocation it performs full PWM initialization per SW detailed design
 * - On subsequent invocations it performs a duty-step runtime update as described
 *
 * Signature and visibility follow the requirements exactly.
 */
static void configurePwmChannels(uint8_t channelCount)
{
    /* Validate channelCount quickly (defensive) */
    if (channelCount != NUM_OF_CHANNELS)
    {
        /* Invalid parameter -> do nothing (could also assert/log). Keep minimal. */
        return;
    }

    /* If not initialized -> perform full initialization sequence */
    if (!g_gtmTom3phState.initialized)
    {
        /* 1) Declare all local configuration structures */
        IfxGtm_Pwm_Config             config;
        IfxGtm_Pwm_ChannelConfig      channelConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig       output[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig          dtmConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_InterruptConfig   irqCfg;

        /* 2) Populate defaults */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* 3) Set config fields per design requirements */
        /* Clock source: TOM submodule -> set tom field only in the union */
        config.clockSource.tom = IfxGtm_Cmu_Fxclk_0; /* TOM uses FXCLK */

        config.cluster            = (IfxGtm_Cluster)IFXGTM_CLUSTER; /* platform cluster macro */
        config.subModule          = IfxGtm_Pwm_SubModule_tom;
        config.alignment          = IfxGtm_Pwm_Alignment_center; /* center-aligned */
        config.syncStart          = TRUE;
        config.syncUpdateEnabled  = TRUE;
        config.numChannels        = NUM_OF_CHANNELS;
        config.channels           = channelConfig;
        config.frequency          = PWM_FREQUENCY;
        config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

        /* 4) Output and dead-time configuration (per-channel) */
        /* Fill outputs and dtm arrays and channelConfig entries directly */

        /* Channel 0 -> Phase U (logical mapping: 0=U) */
        output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        output[0].polarity              = Ifx_ActiveState_high;
        output[0].complementaryPolarity = Ifx_ActiveState_low;
        output[0].outputMode            = IfxPort_OutputMode_pushPull;
        output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        dtmConfig[0].deadTime.rising    = PWM_DEAD_TIME;
        dtmConfig[0].deadTime.falling   = PWM_DEAD_TIME;

        /* Channel 1 -> Phase V */
        output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        output[1].polarity              = Ifx_ActiveState_high;
        output[1].complementaryPolarity = Ifx_ActiveState_low;
        output[1].outputMode            = IfxPort_OutputMode_pushPull;
        output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        dtmConfig[1].deadTime.rising    = PWM_DEAD_TIME;
        dtmConfig[1].deadTime.falling   = PWM_DEAD_TIME;

        /* Channel 2 -> Phase W */
        output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        output[2].polarity              = Ifx_ActiveState_high;
        output[2].complementaryPolarity = Ifx_ActiveState_low;
        output[2].outputMode            = IfxPort_OutputMode_pushPull;
        output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        dtmConfig[2].deadTime.rising    = PWM_DEAD_TIME;
        dtmConfig[2].deadTime.falling   = PWM_DEAD_TIME;

        /* 5) Initialize per-channel configuration directly (no initChannelConfig call) */
        /* Channel indices: Ch_0..Ch_2 per structural rule */
        channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        channelConfig[0].phase     = 0.0f;
        channelConfig[0].duty      = PHASE_U_DUTY;
        channelConfig[0].dtm       = &dtmConfig[0];
        channelConfig[0].output    = &output[0];
        channelConfig[0].mscOut    = NULL_PTR;
        /* Interrupt: base channel gets the interrupt */

        channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        channelConfig[1].phase     = 0.0f;
        channelConfig[1].duty      = PHASE_V_DUTY;
        channelConfig[1].dtm       = &dtmConfig[1];
        channelConfig[1].output    = &output[1];
        channelConfig[1].mscOut    = NULL_PTR;

        channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        channelConfig[2].phase     = 0.0f;
        channelConfig[2].duty      = PHASE_W_DUTY;
        channelConfig[2].dtm       = &dtmConfig[2];
        channelConfig[2].output    = &output[2];
        channelConfig[2].mscOut    = NULL_PTR;

        /* 6) Interrupt configuration (fully populated) */
        irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
        irqCfg.isrProvider = IfxSrc_Tos_cpu0;
        irqCfg.priority    = ISR_PRIORITY_ATOM;
        irqCfg.periodEvent = IfxGtm_periodEventFunction;
        irqCfg.dutyEvent   = NULL_PTR;

        /* Assign interrupt only to the base channel (channel 0) */
        channelConfig[0].interrupt = &irqCfg;
        channelConfig[1].interrupt = NULL_PTR;
        channelConfig[2].interrupt = NULL_PTR;

        /* 7) Enable guard (all CMU calls must be inside) */
        if (!IfxGtm_isEnabled(&MODULE_GTM))
        {
            IfxGtm_enable(&MODULE_GTM);

            /* Read dynamic module frequency and configure clocks */
            float32 moduleFrequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFrequency);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFrequency);

            /* Enable FXCLK and CLK0 as required */
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }

        /* 8) Initialize the PWM driver. Provide the persistent channels array from module-state */
        IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

        /* 9) After initialization, apply initial duty values stored in persistent state
         * Prepare the persistent duty array and deadTimes in state before applying.
         */
        g_gtmTom3phState.dutyCycles[0] = PHASE_U_DUTY;
        g_gtmTom3phState.dutyCycles[1] = PHASE_V_DUTY;
        g_gtmTom3phState.dutyCycles[2] = PHASE_W_DUTY;

        g_gtmTom3phState.phases[0] = channelConfig[0].phase;
        g_gtmTom3phState.phases[1] = channelConfig[1].phase;
        g_gtmTom3phState.phases[2] = channelConfig[2].phase;

        g_gtmTom3phState.deadTimes[0] = dtmConfig[0].deadTime;
        g_gtmTom3phState.deadTimes[1] = dtmConfig[1].deadTime;
        g_gtmTom3phState.deadTimes[2] = dtmConfig[2].deadTime;

        /* 10) Apply initial duty values immediately (per behavior_description) */
        IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);

        /* 11) Configure status/LED GPIO AFTER PWM init sequence */
        IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

        /* Mark as initialized so subsequent calls perform runtime updates */
        g_gtmTom3phState.initialized = TRUE;

        return;
    }

    /* --------------------------------------------------------------------- */
    /* Runtime update behavior: perform duty-step update for 3 channels only   */
    /* The update modifies only the persistent dutyCycles[] and calls the
     * immediate update API so changes take effect without blocking.
     * The duty wrap rule must follow the exact two-line per phase sequence.
     */
    /* Defensive: ensure initialized before updating */
    if (!g_gtmTom3phState.initialized)
    {
        return; /* nothing to update */
    }

    /* Apply DUTY WRAP RULE exactly as mandated (three separate if-blocks) */
    if ((g_gtmTom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[2] = 0.0f; }

    /* Unconditional increments (three lines) */
    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Invoke immediate update API with state's duty array directly (percent values) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32 *)g_gtmTom3phState.dutyCycles);
}

/* End of file */
