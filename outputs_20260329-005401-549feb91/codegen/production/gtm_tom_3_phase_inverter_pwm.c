/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: Unified IfxGtm_Pwm TOM-based 3-phase complementary PWM
 * - Center-aligned, 20 kHz timebase on TOM1 with CH0 as base
 * - Three complementary pairs (U/V/W), active-high HS, active-low LS
 * - Dead-time: 0.5 us, Min pulse: 1.0 us (stored in module state)
 * - Pin routing per requirements
 *
 * Notes:
 * - Clock enable and GCLK programming are done inside the GTM enable guard.
 * - Watchdog handling MUST NOT be placed here (only in CpuX_Main.c).
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (only the allowed ones) ========================= */
#define NUM_OF_CHANNELS             (3U)
#define PWM_FREQUENCY               (20000.0f)      /* 20 kHz */
#define ISR_PRIORITY_ATOM           (3)

/* LED (port, pin) for ISR debug toggle */
#define LED                         &MODULE_P13, 0

/* User-specified TOUT mappings (validated for TC3xx TOM1 pins) */
#define PHASE_U_HS                  &IfxGtm_TOM1_2_TOUT12_P00_3_OUT  /* U high-side: TOM1 CH2 -> P00.3 (TOUT12) */
#define PHASE_U_LS                  &IfxGtm_TOM1_1_TOUT11_P00_2_OUT  /* U low -side: TOM1 CH1 -> P00.2 (TOUT11) */
#define PHASE_V_HS                  &IfxGtm_TOM1_4_TOUT14_P00_5_OUT  /* V high-side: TOM1 CH4 -> P00.5 (TOUT14) */
#define PHASE_V_LS                  &IfxGtm_TOM1_3_TOUT13_P00_4_OUT  /* V low -side: TOM1 CH3 -> P00.4 (TOUT13) */
#define PHASE_W_HS                  &IfxGtm_TOM1_6_TOUT16_P00_7_OUT  /* W high-side: TOM1 CH6 -> P00.7 (TOUT16) */
#define PHASE_W_LS                  &IfxGtm_TOM1_5_TOUT15_P00_6_OUT  /* W low -side: TOM1 CH5 -> P00.6 (TOUT15) */

/* Initial duties in percent (0..100) */
#define PHASE_U_DUTY                (25.0f)
#define PHASE_V_DUTY                (50.0f)
#define PHASE_W_DUTY                (75.0f)

/* Optional duty step (percent) retained for future runtime updates */
#define PHASE_DUTY_STEP             (5.0f)

/* ========================= Local types and module state ========================= */
typedef struct
{
    IfxGtm_Pwm                 pwm;                                   /* PWM driver handle */
    IfxGtm_Pwm_Channel         channels[NUM_OF_CHANNELS];             /* persistent channels array */
    float32                    dutyCycles[NUM_OF_CHANNELS];           /* current duties (percent) */
    float32                    phases[NUM_OF_CHANNELS];               /* current phases (deg or percent as per driver) */
    IfxGtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];            /* stored dead-time (s) per channel */
    float32                    minPulse[NUM_OF_CHANNELS];             /* stored min pulse (s) per channel */
} GtmTom3phPwm_State;

IFX_STATIC GtmTom3phPwm_State g_gtmTom3phPwm_state;

/* ========================= ISR and callback (declared before init) ========================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback required by InterruptConfig (empty body) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API implementation ========================= */
/**
 * Initialize the GTM unified PWM for 3 complementary pairs on TOM1 with a 20 kHz center-aligned timebase.
 * Requirements mapped:
 *  - TOM1 as submodule, base channel logically SubModule_Ch_0
 *  - Clock source FXCLK0
 *  - Center alignment, syncStart and syncUpdate enabled
 *  - Dead-time 0.5 us, min pulse 1.0 us stored in state
 *  - Pins mapped to U/V/W HS/LS per user request with push-pull and CMOS Automotive Speed1
 *  - Initial duties: U=25%, V=50%, W=75%
 */
void initGtmTomPwm(void)
{
    /* 1) Persistent state exists at file-scope (g_gtmTom3phPwm_state) */

    /* 2) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Main config base fields (set later after channel/output setup):
     *    - TOM submodule
     *    - Center-aligned
     *    - FXCLK0 (clockSource.tom)
     *    - syncStart TRUE, frequency 20 kHz
     */

    /* 4) Output routing for three complementary pairs (HS pin + LS complementaryPin) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;            /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;             /* LS active low  */
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

    /* 5) Dead-time and min pulse (seconds) */
    {
        const float32 dt = 0.5e-6f;    /* 0.5 us */
        const float32 mp = 1.0e-6f;    /* 1.0 us */
        for (uint8 i = 0U; i < NUM_OF_CHANNELS; ++i)
        {
            dtmConfig[i].deadTime.rising = dt;
            dtmConfig[i].deadTime.falling = dt;
            g_gtmTom3phPwm_state.deadTimes[i].rising = dt;
            g_gtmTom3phPwm_state.deadTimes[i].falling = dt;
            g_gtmTom3phPwm_state.minPulse[i] = mp;
        }
    }

    /* Interrupt configuration for base channel events */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2). Base/timebase channel is Ch_0. */
    /* Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;

    /* Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = &irqCfg;

    /* Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = &irqCfg;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;                  /* use CLS0 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;          /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;       /* center-aligned */
    config.syncStart            = TRUE;                              /* synced start */
    config.syncUpdateEnabled    = TRUE;                              /* shadow updates at period */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                     /* 20 kHz */
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;                /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock */

    /* 8) Enable guard: enable GTM and CMU only if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize the PWM driver (applies pin mux, timebase, alignment, shadow setup) */
    IfxGtm_Pwm_init(&g_gtmTom3phPwm_state.pwm, g_gtmTom3phPwm_state.channels, &config);

    /* Store initial state for later runtime updates */
    g_gtmTom3phPwm_state.dutyCycles[0] = PHASE_U_DUTY;
    g_gtmTom3phPwm_state.dutyCycles[1] = PHASE_V_DUTY;
    g_gtmTom3phPwm_state.dutyCycles[2] = PHASE_W_DUTY;
    g_gtmTom3phPwm_state.phases[0]     = 0.0f;
    g_gtmTom3phPwm_state.phases[1]     = 0.0f;
    g_gtmTom3phPwm_state.phases[2]     = 0.0f;

    /* 10) Ensure exact base frequency and initial duty are applied immediately */
    IfxGtm_Pwm_updateFrequencyImmediate(&g_gtmTom3phPwm_state.pwm, PWM_FREQUENCY);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phPwm_state.pwm, (float32 *)g_gtmTom3phPwm_state.dutyCycles);

    /* LED configured last */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
