/**
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production-ready GTM TOM 3-phase inverter PWM driver (TC3xx migration)
 * Implements initialization and runtime duty update logic in a single
 * static function configurePwmChannels(uint8_t channelCount).
 *
 * Conforms to iLLD initialization patterns and project structural rules.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include <Ifx_Types.h>
#include <Gtm/Pwm/IfxGtm_Pwm.h>
#include <IfxPort.h>
#include <IfxPort_Pinmap.h>

/* Configuration macros (use values from requirements) */
#define NUM_OF_CHANNELS         3u
#define PWM_FREQUENCY           (20000.0f)        /* 20 kHz */
#define PWM_DEAD_TIME           (5e-07f)          /* 0.5e-6 s */
#define PWM_MIN_PULSE_TIME      (1e-06f)          /* 1.0e-6 s */

/* ISR priority macro (used by IFX_INTERRUPT and InterruptConfig.priority) */
#define ISR_PRIORITY_ATOM      3

/* Phase pin mappings (preserve reference TOM mappings / user requests)
 * NOTE: These macros refer to TOUT pinmap symbols. Replace these if your
 * integration requires different TOUT symbols. The symbols used here are
 * from the project reference mapping.
 */
#define PHASE_U_HS    &IfxGtm_TOM1_2_TOUT12_P00_3_OUT
#define PHASE_U_LS    &IfxGtm_TOM1_1_TOUT11_P00_2_OUT
#define PHASE_V_HS    &IfxGtm_TOM1_4_TOUT14_P00_5_OUT
#define PHASE_V_LS    &IfxGtm_TOM1_3_TOUT13_P00_4_OUT
#define PHASE_W_HS    &IfxGtm_TOM1_6_TOUT16_P00_7_OUT
#define PHASE_W_LS    &IfxGtm_TOM1_5_TOUT15_P00_6_OUT

/* Initial duties (percent) */
#define PHASE_U_DUTY  (25.0f)
#define PHASE_V_DUTY  (50.0f)
#define PHASE_W_DUTY  (75.0f)

/* Duty step (percent) */
#define PHASE_DUTY_STEP (10.0f)

/* LED macro: compound form <port-addr>, <pin> per structural rules */
#define LED (&MODULE_P13), 0

/* Module persistent state (IFX_STATIC per structural rules) */
typedef struct
{
    IfxGtm_Pwm                 pwm;                            /* PWM driver handle/state */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];      /* persistent channel SFR wrappers */
    float32                    dutyCycles[NUM_OF_CHANNELS];    /* percent values 0..100 */
    float32                    phases[NUM_OF_CHANNELS];        /* phase offsets in percent or degrees (reserved) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];     /* stored dead-time per channel */
    boolean                    initialized;                    /* simple guard */
} GtmTom3phPwmState;

IFX_STATIC GtmTom3phPwmState g_state =
{
    .initialized = FALSE
};

/*
 * Period-event callback required by InterruptConfig. Must be non-static and
 * have exactly the required signature. Body MUST be empty aside from a cast
 * to void for the parameter (per structural rules).
 */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/*
 * ISR for GTM ATOM/TOM notifications. Name taken from DRIVER-SPECIFIC KNOWLEDGE.
 * Body MUST be exactly: IfxPort_togglePin(LED);
 */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/**
 * configurePwmChannels
 *
 * This function performs two behaviors depending on internal module state:
 *  - First call: perform full PWM driver initialization for a 3-channel
 *    center-aligned TOM cluster and store persistent runtime state.
 *  - Subsequent calls: perform runtime duty step updates (+10% per update,
 *    wrapping to 0 when exceeding 100%) and apply them immediately.
 *
 * The function signature and behavior precisely follow the SW Detailed Design
 * and the mandatory iLLD initialization patterns.
 *
 * NOTE: This function is declared static (translation-unit scope) per the
 * module design. It should be invoked by the application (Cpu0_Main.c)
 * during startup and periodically from the main loop to apply duty updates.
 *
 * @param channelCount  Number of logical channels requested (ignored after init)
 */
static void configurePwmChannels(uint8_t channelCount)
{
    /* If not initialized, perform the complete initialization sequence */
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

        /* 3) Output configuration and complementary polarity convention */
        /* Channel 0 -> Phase U */
        output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        output[0].polarity               = Ifx_ActiveState_high;
        output[0].complementaryPolarity  = Ifx_ActiveState_low;
        output[0].outputMode             = IfxPort_OutputMode_pushPull;
        output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel 1 -> Phase V */
        output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        output[1].polarity               = Ifx_ActiveState_high;
        output[1].complementaryPolarity  = Ifx_ActiveState_low;
        output[1].outputMode             = IfxPort_OutputMode_pushPull;
        output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel 2 -> Phase W */
        output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        output[2].polarity               = Ifx_ActiveState_high;
        output[2].complementaryPolarity  = Ifx_ActiveState_low;
        output[2].outputMode             = IfxPort_OutputMode_pushPull;
        output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* 4) Dead-time configuration (rising & falling) */
        dtmConfig[0].deadTime.rising  = PWM_DEAD_TIME;
        dtmConfig[0].deadTime.falling = PWM_DEAD_TIME;

        dtmConfig[1].deadTime.rising  = PWM_DEAD_TIME;
        dtmConfig[1].deadTime.falling = PWM_DEAD_TIME;

        dtmConfig[2].deadTime.rising  = PWM_DEAD_TIME;
        dtmConfig[2].deadTime.falling = PWM_DEAD_TIME;

        /* 5) Interrupt configuration - fully populated per rules */
        irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
        irqCfg.isrProvider = IfxSrc_Tos_cpu0;
        irqCfg.priority    = ISR_PRIORITY_ATOM;
        irqCfg.periodEvent = IfxGtm_periodEventFunction;
        irqCfg.dutyEvent   = NULL_PTR;

        /* 6) Channel configuration - initialize directly, DO NOT call initChannelConfig */
        /* Channel indices must start at Ch_0 through Ch_2 */
        channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        channelConfig[0].phase     = 0.0f;
        channelConfig[0].duty      = PHASE_U_DUTY;
        channelConfig[0].dtm       = &dtmConfig[0];
        channelConfig[0].output    = &output[0];
        channelConfig[0].mscOut    = NULL_PTR;
        channelConfig[0].interrupt = &irqCfg;

        channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        channelConfig[1].phase     = 0.0f;
        channelConfig[1].duty      = PHASE_V_DUTY;
        channelConfig[1].dtm       = &dtmConfig[1];
        channelConfig[1].output    = &output[1];
        channelConfig[1].mscOut    = NULL_PTR;
        channelConfig[1].interrupt = &irqCfg;

        channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        channelConfig[2].phase     = 0.0f;
        channelConfig[2].duty      = PHASE_W_DUTY;
        channelConfig[2].dtm       = &dtmConfig[2];
        channelConfig[2].output    = &output[2];
        channelConfig[2].mscOut    = NULL_PTR;
        channelConfig[2].interrupt = &irqCfg;

        /* 7) Main config fields (clockSource union -> set TOM field only) */
        config.cluster              = (IfxGtm_Cluster)IFXGTM_CLUSTER; /* preserve reference cluster macro */
        config.subModule            = IfxGtm_Pwm_SubModule_tom;     /* TOM submodule */
        config.alignment            = IfxGtm_Pwm_Alignment_center;  /* center-aligned */
        config.syncStart            = TRUE;
        config.numChannels          = (uint8_t)NUM_OF_CHANNELS;
        config.channels             = &channelConfig[0];
        config.frequency            = PWM_FREQUENCY;
        config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;          /* TOM uses fxclk */
        config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
        config.syncUpdateEnabled    = TRUE;

        /* 8) Enable guard + CMU configuration (all CMU calls must be inside guard) */
        if (!IfxGtm_isEnabled(&MODULE_GTM))
        {
            IfxGtm_enable(&MODULE_GTM);
            float32 moduleFrequency = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFrequency);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFrequency);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }

        /* 9) Initialize the PWM driver and reserve internal state
         *    Channels array is persistent in g_state and passed to driver
         */
        IfxGtm_Pwm_init(&g_state.pwm, &g_state.channels[0], &config);

        /* 10) Store persistent runtime state: duties, phases and deadTimes */
        g_state.dutyCycles[0] = channelConfig[0].duty;
        g_state.dutyCycles[1] = channelConfig[1].duty;
        g_state.dutyCycles[2] = channelConfig[2].duty;

        g_state.phases[0] = channelConfig[0].phase;
        g_state.phases[1] = channelConfig[1].phase;
        g_state.phases[2] = channelConfig[2].phase;

        g_state.deadTimes[0] = dtmConfig[0].deadTime;
        g_state.deadTimes[1] = dtmConfig[1].deadTime;
        g_state.deadTimes[2] = dtmConfig[2].deadTime;

        g_state.initialized = TRUE;

        /* 11) Configure status/LED GPIO AFTER PWM init per rules */
        IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

        /* Per strict rules: DO NOT call IfxGtm_Pwm_updateChannelsDutyImmediate here.
         * The initial channelConfig[].duty values are applied by the driver during init.
         */
        (void)channelCount; /* suppress unused parameter warning */
        return;
    }

    /* Runtime update: minimal computation only. Do not time or poll here. */

    /* Per DUTY WRAP RULE: three separate if-blocks, then three unconditional adds */
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

    g_state.dutyCycles[0] += PHASE_DUTY_STEP;
    g_state.dutyCycles[1] += PHASE_DUTY_STEP;
    g_state.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply updated duties immediately. Pass the persistent state's array directly. */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_state.pwm, (float32 *)g_state.dutyCycles);
}
