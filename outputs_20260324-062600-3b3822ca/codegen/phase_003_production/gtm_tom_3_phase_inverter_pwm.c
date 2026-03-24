/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production module: GTM_TOM_3_Phase_Inverter_PWM
 * Target: AURIX TC3xx (TC387)
 * Peripheral: GTM TOM PWM (unified IfxGtm_Pwm driver, single-output)
 */
#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic, not family-suffixed) */
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_PinMap.h"
#include "IfxGtm.h"
#include "IfxPort.h"

/* ========================= Pin assignments (validated for TC3xx) ========================= */
/*
 * User requirement: TOM1 CH2/CH4/CH6 routed to P02.0/P02.1/P02.2 respectively
 * Using unified driver single-output (ignore low-side pins)
 */
#define PHASE_U_PIN   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_V_PIN   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_4N_TOUT1_P02_1_OUT)
#define PHASE_W_PIN   ((IfxGtm_Pwm_ToutMap *)&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)

/* Optional LED for diagnostics (no requirement to use ISR here) */
#define LED_PORT      (&MODULE_P13)
#define LED_PIN       (0U)

/* ========================= Driver state ========================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                                 /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];           /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];         /* Duty in percent */
    float32             phases[NUM_OF_CHANNELS];             /* Phase shift in percent or degrees (unused, 0.0f) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];          /* Dead-time values (unused for single-output) */
} GtmTom3phInv;

static GtmTom3phInv s_pwm;
static boolean       s_initialized = FALSE;

/* ========================= Internal helpers ========================= */
static void prv_enableGtmClocks(void)
{
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
    }

    /* Set GCLK to module frequency, set CLK0 = GCLK, enable FXCLK for TOM */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }
}

/* ========================= Public functions ========================= */
void initGtmTom3PhaseInverterPwm(void)
{
    IfxGtm_Pwm_Config        config;
    IfxGtm_Pwm_ChannelConfig chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig  outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig     dtmCfg[NUM_OF_CHANNELS];

    /* 1) Enable GTM and functional clocks */
    prv_enableGtmClocks();

    /* 2) Initialize main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration for single-ended outputs (ignore complementary) */
    /* U: TOM1 CH2 -> P02.0 */
    outCfg[0].pin                   = PHASE_U_PIN;
    outCfg[0].complementaryPin      = NULL_PTR; /* single-output */
    outCfg[0].polarity              = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity = Ifx_ActiveState_low; /* unused */
    outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V: TOM1 CH4 -> P02.1 */
    outCfg[1].pin                   = PHASE_V_PIN;
    outCfg[1].complementaryPin      = NULL_PTR;
    outCfg[1].polarity              = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W: TOM1 CH6 -> P02.2 */
    outCfg[2].pin                   = PHASE_W_PIN;
    outCfg[2].complementaryPin      = NULL_PTR;
    outCfg[2].polarity              = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time config (not used for single-output, set to zero but keep structure per pattern) */
    dtmCfg[0].deadTime.rising  = 0.0f;
    dtmCfg[0].deadTime.falling = 0.0f;
    dtmCfg[1].deadTime.rising  = 0.0f;
    dtmCfg[1].deadTime.falling = 0.0f;
    dtmCfg[2].deadTime.rising  = 0.0f;
    dtmCfg[2].deadTime.falling = 0.0f;

    /* 5) Channel configuration: TOM1 channels 2, 4, 6; 20 kHz center-aligned, Fxclk0 */
    chCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;  /* TOM1 CH2 */
    chCfg[0].phase     = 0.0f;
    chCfg[0].duty      = DUTY_U_INIT_PERCENT;        /* U=25% */
    chCfg[0].dtm       = &dtmCfg[0];
    chCfg[0].output    = &outCfg[0];
    chCfg[0].mscOut    = NULL_PTR;
    chCfg[0].interrupt = NULL_PTR;

    chCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_4;  /* TOM1 CH4 */
    chCfg[1].phase     = 0.0f;
    chCfg[1].duty      = DUTY_V_INIT_PERCENT;        /* V=50% */
    chCfg[1].dtm       = &dtmCfg[1];
    chCfg[1].output    = &outCfg[1];
    chCfg[1].mscOut    = NULL_PTR;
    chCfg[1].interrupt = NULL_PTR;

    chCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_6;  /* TOM1 CH6 */
    chCfg[2].phase     = 0.0f;
    chCfg[2].duty      = DUTY_W_INIT_PERCENT;        /* W=75% */
    chCfg[2].dtm       = &dtmCfg[2];
    chCfg[2].output    = &outCfg[2];
    chCfg[2].mscOut    = NULL_PTR;
    chCfg[2].interrupt = NULL_PTR;

    /* 6) Main config fields */
    config.cluster           = IfxGtm_Cluster_0;                 /* GTM Cluster 0 */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;         /* TOM */
    config.alignment         = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart         = TRUE;                              /* start after init */
    config.syncUpdateEnabled = TRUE;                              /* shadow updates */
    config.numChannels       = NUM_OF_CHANNELS;
    config.channels          = chCfg;
    config.frequency         = TIMING_PWM_FREQUENCY_HZ;           /* 20 kHz */
    config.clockSource.tom   = IfxGtm_Cmu_Fxclk_0;                /* Fxclk0 */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;  /* keep default clock source */

    /* 7) Initialize PWM driver */
    IfxGtm_Pwm_init(&s_pwm.pwm, &s_pwm.channels[0], &config);

    /* 8) Start the synchronized PWM channels explicitly (per SW Detailed Design) */
    IfxGtm_Pwm_startSyncedChannels(&s_pwm.pwm);

    /* 9) Initialize runtime arrays and apply initial duties immediately */
    s_pwm.dutyCycles[0] = DUTY_U_INIT_PERCENT;
    s_pwm.dutyCycles[1] = DUTY_V_INIT_PERCENT;
    s_pwm.dutyCycles[2] = DUTY_W_INIT_PERCENT;

    s_pwm.phases[0] = 0.0f; s_pwm.phases[1] = 0.0f; s_pwm.phases[2] = 0.0f;
    s_pwm.deadTimes[0] = dtmCfg[0].deadTime; s_pwm.deadTimes[1] = dtmCfg[1].deadTime; s_pwm.deadTimes[2] = dtmCfg[2].deadTime;

    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm.pwm, (float32 *)s_pwm.dutyCycles);

    /* Configure optional LED pin */
    IfxPort_setPinModeOutput(LED_PORT, LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

void updateGtmTom3PhaseDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init not completed */
    }

    /* Increment each duty by +10%, wrap at 100% to 0% */
    for (uint8 i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        float32 next = s_pwm.dutyCycles[i] + TIMING_DUTY_STEP_PERCENT;
        if (next >= 100.0f)
        {
            next = 0.0f;
        }
        s_pwm.dutyCycles[i] = next;
    }

    /* Apply immediately to active outputs */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm.pwm, (float32 *)s_pwm.dutyCycles);
}
