/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * GTM TOM 3-phase inverter PWM driver (migration mode, TC3xx)
 *
 * Implements PWM initialization and runtime duty-step update logic
 * according to the authoritative iLLD-derived specification.
 *
 * Notes:
 * - All configuration and initialization patterns follow the mandated
 *   iLLD usage patterns and the SW detailed design.
 * - Watchdog functions are NOT present in this module (CPU file only).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* Public configuration macros (from CONFIGURATION VALUES) */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)      /* 20 kHz */
#define PWM_DEAD_TIME              (0.5e-6f)       /* 0.5 us */
#define PWM_MIN_PULSE_TIME         (1.0e-6f)       /* 1.0 us */
#define PHASE_DUTY_STEP            (10.0f)         /* percent */

/* Initial duties (percent) */
#define PHASE_U_DUTY_INIT          (25.0f)
#define PHASE_V_DUTY_INIT          (50.0f)
#define PHASE_W_DUTY_INIT          (75.0f)

/* ISR priority macro (driver-specific naming requirement) */
#define ISR_PRIORITY_ATOM         (3u)

/* LED macro: compound form (port, pin) per structural rules */
#define LED &MODULE_P13, 0

/*
 * Pin symbols
 *
 * The user's requested TOUT pin symbols (P00.2/P00.3/P00.4) were not
 * present in the validated pin symbol list provided in the prompt. Per
 * the PIN SYMBOL RESOLUTION rules we do NOT invent TOUT symbols. Use
 * NULL_PTR placeholders here and document intended mapping for integrators.
 *
 * Replace the NULL_PTR values with the correct &IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT
 * symbols from the device pin map when integrating on real hardware.
 */
#define PHASE_U_HS   (NULL_PTR) /* Intended: &IfxGtm_TOM1_5_TOUT11_P00_2_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Intended complementary pin for U (low side) */
#define PHASE_V_HS   (NULL_PTR) /* Intended: &IfxGtm_TOM1_2_TOUT12_P00_3_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Intended complementary pin for V (low side) */
#define PHASE_W_HS   (NULL_PTR) /* Intended: &IfxGtm_TOM1_3_TOUT13_P00_4_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Intended complementary pin for W (low side) */

/* Module persistent state (IFX_STATIC required by structural rules) */
typedef struct
{
    IfxGtm_Pwm                pwm;                                    /* PWM driver handle */
    IfxGtm_Pwm_Channel        channels[NUM_OF_CHANNELS];              /* persistent channel SFR handles */
    float32                   dutyCycles[NUM_OF_CHANNELS];           /* percent (0..100) */
    float32                   phases[NUM_OF_CHANNELS];                /* phase offsets in degrees (optional) */
    IfxGtm_Pwm_DeadTime       deadTimes[NUM_OF_CHANNELS];             /* stored deadtime values */
    boolean                   initialized;                            /* init guard */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_state =
{
    .pwm = {0},
    .channels = {{0}},
    .dutyCycles = {0},
    .phases = {0},
    .deadTimes = {{0}},
    .initialized = FALSE
};

/* Forward declarations for period callback and ISR (structural rules) */
void IfxGtm_periodEventFunction(void *data);
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

/*
 * Period-event callback assigned to InterruptConfig.periodEvent
 * Must be non-static and have an empty body per structural rules.
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/*
 * ISR: minimal body toggles LED only (per structural rules)
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/*
 * configurePwmChannels
 *
 * Dual-behavior function implemented per the AUTHORITATIVE SW DETAILED DESIGN.
 * - On first call it performs full initialization of a 3-channel center-aligned
 *   TOM PWM cluster and stores persistent state.
 * - On subsequent calls it performs the duty-step update algorithm (increment
 *   each channel by PHASE_DUTY_STEP percent with wrap-to-zero behavior) and
 *   issues an immediate duty update to the driver.
 *
 * The function signature is static as required by the API description.
 */
static void configurePwmChannels(uint8_t channelCount)
{
    /* Guard: only channelCount sanity-checked for expected number */
    if (channelCount < NUM_OF_CHANNELS)
    {
        /* Do not attempt init/update if caller provided too-small count */
        return;
    }

    /* If not initialized: perform full initialization sequence */
    if (!g_state.initialized)
    {
        /* 1) Declare local configuration structures */
        IfxGtm_Pwm_Config           config;
        IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_InterruptConfig  irqCfg;

        /* 2) Populate defaults */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* 3) Configure outputs (pin, polarity, output mode, pad driver)
         *    and DTM dead time. Per complementary polarity convention: HS active HIGH,
         *    LS active LOW.
         */
        /* Channel 0 -> phase U */
        output[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        output[0].polarity              = Ifx_ActiveState_high;
        output[0].complementaryPolarity = Ifx_ActiveState_low;
        output[0].outputMode            = IfxPort_OutputMode_pushPull;
        output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel 1 -> phase V */
        output[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        output[1].polarity              = Ifx_ActiveState_high;
        output[1].complementaryPolarity = Ifx_ActiveState_low;
        output[1].outputMode            = IfxPort_OutputMode_pushPull;
        output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel 2 -> phase W */
        output[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        output[2].polarity              = Ifx_ActiveState_high;
        output[2].complementaryPolarity = Ifx_ActiveState_low;
        output[2].outputMode            = IfxPort_OutputMode_pushPull;
        output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* 4) Dead-time configuration (rising and falling equal to PWM_DEAD_TIME) */
        for (uint8 i = 0; i < NUM_OF_CHANNELS; ++i)
        {
            dtmConfig[i].deadTime.rising  = PWM_DEAD_TIME;
            dtmConfig[i].deadTime.falling = PWM_DEAD_TIME;
        }

        /* 5) Interrupt configuration (base/period event assigned to channel 0)
         *    Fully populate fields per structural rule.
         */
        irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
        irqCfg.isrProvider = IfxSrc_Tos_cpu0;
        irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
        irqCfg.periodEvent = IfxGtm_periodEventFunction;
        irqCfg.dutyEvent   = NULL_PTR;

        /* 6) Channel configuration - initialize EACH channelConfig element directly
         *    (no initChannelConfig calls). Use logical timer indices starting at 0.
         */
        /* Channel 0 */
        channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        channelConfig[0].phase     = 0.0f;
        channelConfig[0].duty      = PHASE_U_DUTY_INIT;
        channelConfig[0].dtm       = &dtmConfig[0];
        channelConfig[0].output    = &output[0];
        channelConfig[0].mscOut    = NULL_PTR;
        channelConfig[0].interrupt = &irqCfg;

        /* Channel 1 */
        channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        channelConfig[1].phase     = 0.0f;
        channelConfig[1].duty      = PHASE_V_DUTY_INIT;
        channelConfig[1].dtm       = &dtmConfig[1];
        channelConfig[1].output    = &output[1];
        channelConfig[1].mscOut    = NULL_PTR;
        channelConfig[1].interrupt = NULL_PTR; /* only base channel has interrupt */

        /* Channel 2 */
        channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        channelConfig[2].phase     = 0.0f;
        channelConfig[2].duty      = PHASE_W_DUTY_INIT;
        channelConfig[2].dtm       = &dtmConfig[2];
        channelConfig[2].output    = &output[2];
        channelConfig[2].mscOut    = NULL_PTR;
        channelConfig[2].interrupt = NULL_PTR;

        /* 7) Main config struct fields (clockSource TOM field only, per SUBMODULE TYPE = TOM) */
        config.cluster              = (IfxGtm_Cluster)0; /* integrator: replace with desired cluster if needed */
        config.subModule            = IfxGtm_Pwm_SubModule_tom;
        config.alignment            = IfxGtm_Pwm_Alignment_center;
        config.syncStart            = TRUE;
        config.numChannels          = NUM_OF_CHANNELS;
        config.channels             = channelConfig;
        config.frequency            = PWM_FREQUENCY;
        config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0; /* TOM uses fxclk */
        config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
        config.syncUpdateEnabled    = TRUE;

        /* 8) Enable guard: perform GTM enable + CMU configuration only if GTM is not enabled */
        if (!IfxGtm_isEnabled(&MODULE_GTM))
        {
            IfxGtm_enable(&MODULE_GTM);

            float32 moduleFrequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFrequency);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFrequency);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }

        /* 9) Initialize the PWM driver; pass persistent channels array from module state */
        IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

        /* 10) After initialization, apply initial duties via immediate update as specified.
         *     The SW detailed design requests this explicit update call.
         */
        g_state.dutyCycles[0] = PHASE_U_DUTY_INIT;
        g_state.dutyCycles[1] = PHASE_V_DUTY_INIT;
        g_state.dutyCycles[2] = PHASE_W_DUTY_INIT;

        IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);

        /* 11) Store deadTimes from the dtmConfig into persistent state */
        for (uint8 i = 0; i < NUM_OF_CHANNELS; ++i)
        {
            g_state.deadTimes[i] = dtmConfig[i].deadTime;
            g_state.phases[i]    = channelConfig[i].phase;
        }

        /* 12) Configure LED GPIO AFTER PWM init sequence */
        IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

        /* Mark initialized */
        g_state.initialized = TRUE;

        return;
    }

    /* If already initialized: perform duty-step update algorithm (minimal work)
     * Per DUTY WRAP RULE: Do not use a for-loop. Write three separate if-blocks
     * followed by three unconditional additions. Then issue immediate update API.
     */

    /* Channel 0 (U) */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[0] = 0.0f;
    }

    /* Channel 1 (V) */
    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[1] = 0.0f;
    }

    /* Channel 2 (W) */
    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[2] = 0.0f;
    }

    /* Unconditional additions */
    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate update request to the GTM PWM driver - pass state's duty array directly */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
