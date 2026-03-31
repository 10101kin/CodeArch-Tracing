/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production-ready driver implementing a 3-channel center-aligned GTM/TOM PWM
 * cluster for a 3-phase inverter on TC3xx family. The implementation follows
 * the authoritative iLLD initialization pattern and the supplied SW detailed
 * design. The module stores persistent runtime state so duty updates can be
 * applied with the immediate update API.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include <Ifx_Types.h>
#include <IfxGtm_Pwm.h>
#include <IfxPort.h>
#include <IfxPort_PinMap.h>

/*
 * Configuration macros (values derived from the provided configuration values)
 */
#define NUM_OF_CHANNELS        3u
#define PWM_FREQUENCY          (20000.0f)        /* 20 kHz */
#define PWM_DEAD_TIME          (0.5e-6f)         /* 0.5 microsecond */
#define PWM_MIN_PULSE_TIME     (1.0e-6f)         /* 1.0 microsecond */

/* Initial duty percentages (0..100) */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* Per-update duty step (percent) */
#define PHASE_DUTY_STEP        (10.0f)

/* ISR priority macro (project may override) */
#ifndef ISR_PRIORITY_ATOM
#define ISR_PRIORITY_ATOM      6u
#endif

/* LED macro as required by the structural rules (compound form) */
#define LED &MODULE_P13, 0

/*
 * Pin mapping macros
 * NOTE: The design requires preserving TOM/timer & pin mapping. Use the
 * requested TOM pin symbols. If integration headers differ, these macros
 * must be adjusted in integration stage. These macros reference iLLD
 * TOUT pinmap symbols (assumed present in the integration environment).
 */
#define PHASE_U_HS   &IfxGtm_TOM1_5_TOUT11_P00_2_OUT
#define PHASE_U_LS   NULL_PTR
#define PHASE_V_HS   &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_V_LS   NULL_PTR
#define PHASE_W_HS   &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_LS   NULL_PTR

/* Module-level persistent state (must be IFX_STATIC per structural rules) */
typedef struct
{
    IfxGtm_Pwm                 pwm;                               /* pwm handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];         /* persistent channel SFR refs */
    float32                    dutyCycles[NUM_OF_CHANNELS];       /* percent values (0..100) */
    float32                    phases[NUM_OF_CHANNELS];           /* phases (radians or degrees as needed) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];       /* dead times stored */
    boolean                    initialized;
} GtmTom3phPwm_State;

IFX_STATIC GtmTom3phPwm_State g_state =
{
    .initialized = FALSE
};

/* Forward declarations for period callback (non-static per structural rules) */
void IfxGtm_periodEventFunction(void *data);

/* ISR required by structural rules. It toggles the LED only. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* ISR body must be minimal: toggle LED */
    IfxPort_togglePin(LED);
}

/**
 * @brief Period-event callback assigned to the base channel interrupt config
 *
 * This callback is intentionally empty. It is referenced by the
 * IfxGtm_Pwm_InterruptConfig::periodEvent pointer during initialization.
 *
 * @param data  generic pointer (unused)
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/**
 * @brief Initialize or update the 3-phase GTM TOM PWM channels
 *
 * Behavior (combined init + runtime update):
 * - On first call (g_state.initialized == FALSE) this function performs the
 *   full initialization sequence per the authoritative iLLD pattern and the
 *   SW detailed design. It prepares configuration structs, enables GTM and
 *   CMU clocks (inside the enable guard), initialises the PWM driver, stores
 *   persistent state (duties, dead-times, channels), applies initial duties
 *   via the immediate-update API and configures the LED GPIO.
 * - On subsequent calls this function performs a minimal runtime duty update:
 *   each of the three duty values is incremented by PHASE_DUTY_STEP; if the
 *   (duty + step) >= 100 the duty is wrapped to 0 before adding the step.
 *   The updated duty array (percent values) is passed directly to the
 *   IfxGtm_Pwm_updateChannelsDutyImmediate API.
 *
 * @param channelCount  expected logical channel count (must be >= 3 for init)
 */
static void configurePwmChannels(uint8_t channelCount)
{
    /* Basic guard: ensure reasonable input */
    if (channelCount < NUM_OF_CHANNELS)
    {
        return;
    }

    /* Initialization path */
    if (!g_state.initialized)
    {
        /* 1) Declare local configuration structures */
        IfxGtm_Pwm_Config           config;
        IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outputs[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
        IfxGtm_Pwm_InterruptConfig  interruptConfig;

        /* 2) Populate defaults */
        IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

        /* 3) Output configuration (high-side pins, complementary left NULL_PTR)
         *    and polarity/pad driver setup. Complementary pins are left
         *    NULL_PTR here to preserve flexibility; complementary polarity is
         *    still set according to convention.
         */
        outputs[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
        outputs[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
        outputs[0].polarity              = Ifx_ActiveState_high;
        outputs[0].complementaryPolarity = Ifx_ActiveState_low;
        outputs[0].outputMode            = IfxPort_OutputMode_pushPull;
        outputs[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputs[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
        outputs[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
        outputs[1].polarity              = Ifx_ActiveState_high;
        outputs[1].complementaryPolarity = Ifx_ActiveState_low;
        outputs[1].outputMode            = IfxPort_OutputMode_pushPull;
        outputs[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outputs[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
        outputs[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
        outputs[2].polarity              = Ifx_ActiveState_high;
        outputs[2].complementaryPolarity = Ifx_ActiveState_low;
        outputs[2].outputMode            = IfxPort_OutputMode_pushPull;
        outputs[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* 4) Dead-time configuration per channel */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
        {
            dtmConfig[i].deadTime.rising  = PWM_DEAD_TIME;
            dtmConfig[i].deadTime.falling = PWM_DEAD_TIME;
        }

        /* 5) Interrupt/base-channel config - fully populate as required */
        interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
        interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
        interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
        interruptConfig.periodEvent = IfxGtm_periodEventFunction;
        interruptConfig.dutyEvent   = NULL_PTR;

        /* 6) Channel configuration: initialize each element directly (no initChannelConfig call) */
        /* Channel index ordering: Ch_0, Ch_1, Ch_2 (logical channels) */

        /* Channel 0 */
        channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        channelConfig[0].phase     = 0.0f;
        channelConfig[0].duty      = PHASE_U_DUTY;
        channelConfig[0].dtm       = &dtmConfig[0];
        channelConfig[0].output    = &outputs[0];
        channelConfig[0].mscOut    = NULL_PTR;
        /* base channel gets the interrupt reference */
        channelConfig[0].interrupt = &interruptConfig;

        /* Channel 1 */
        channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        channelConfig[1].phase     = 0.0f;
        channelConfig[1].duty      = PHASE_V_DUTY;
        channelConfig[1].dtm       = &dtmConfig[1];
        channelConfig[1].output    = &outputs[1];
        channelConfig[1].mscOut    = NULL_PTR;
        channelConfig[1].interrupt = NULL_PTR;

        /* Channel 2 */
        channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        channelConfig[2].phase     = 0.0f;
        channelConfig[2].duty      = PHASE_W_DUTY;
        channelConfig[2].dtm       = &dtmConfig[2];
        channelConfig[2].output    = &outputs[2];
        channelConfig[2].mscOut    = NULL_PTR;
        channelConfig[2].interrupt = NULL_PTR;

        /* 7) Main config fields (clockSource TOM union field set per submodule type) */
        config.cluster             = (IfxGtm_Cluster)IFXGTM_CLUSTER;
        config.subModule           = IfxGtm_Pwm_SubModule_tom;
        config.alignment           = IfxGtm_Pwm_Alignment_center;
        config.syncStart           = TRUE;
        config.numChannels         = (uint8)NUM_OF_CHANNELS;
        config.channels            = &channelConfig[0];
        config.frequency           = PWM_FREQUENCY;
        /* CLOCK SOURCE UNION: set TOM field only */
        config.clockSource.tom     = IfxGtm_Cmu_Fxclk_0;
        config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0;
        config.syncUpdateEnabled   = TRUE;

        /* 8) Enable guard & CMU configuration (exact pattern required) */
        if (!IfxGtm_isEnabled(&MODULE_GTM))
        {
            IfxGtm_enable(&MODULE_GTM);

            float32 moduleFrequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFrequency);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFrequency);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }

        /* 9) Initialize the PWM driver and reserve internal state. Channels array must be persistent. */
        IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

        /* 10) After init, store persistent runtime state: duties, dead-times, phases */
        g_state.dutyCycles[0] = PHASE_U_DUTY;
        g_state.dutyCycles[1] = PHASE_V_DUTY;
        g_state.dutyCycles[2] = PHASE_W_DUTY;

        g_state.phases[0] = channelConfig[0].phase;
        g_state.phases[1] = channelConfig[1].phase;
        g_state.phases[2] = channelConfig[2].phase;

        for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
        {
            g_state.deadTimes[i].rising  = dtmConfig[i].deadTime.rising;
            g_state.deadTimes[i].falling = dtmConfig[i].deadTime.falling;
        }

        /* 11) Apply the initial duty values immediately (percent values as required)
         * NOTE: The SW detailed design requested this immediate update after init.
         */
        IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);

        /* 12) LED pin configuration MUST be performed AFTER PWM init per rules */
        IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

        /* Mark module as initialized */
        g_state.initialized = TRUE;

        return;
    }

    /* Runtime update path: update duty cycles by step and wrap per DUTY WRAP RULE */

    /* The structural rules require three separate if-blocks (no loop) */
    if ((g_state.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[0] = 0.0f;
    }

    if ((g_state.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[1] = 0.0f;
    }

    if ((g_state.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_state.dutyCycles[2] = 0.0f;
    }

    /* Unconditionally add the step to each channel */
    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Pass the state's duty array directly to the immediate update API (percent values) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32*)g_state.dutyCycles);
}

/* Note: configurePwmChannels is a static function per the API specification.
 * Callers (e.g. Cpu0_Main.c) must call configurePwmChannels(NUM_OF_CHANNELS)
 * once to perform initialization and then call it periodically (e.g. every
 * 500 ms) to perform the duty-step updates.
 */
