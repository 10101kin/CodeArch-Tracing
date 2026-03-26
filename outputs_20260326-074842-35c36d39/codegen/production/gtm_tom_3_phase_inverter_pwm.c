#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm_Pwm.h"      /* Unified high-level GTM PWM driver */
#include "IfxGtm.h"          /* GTM enable/isEnabled */
#include "IfxGtm_Cmu.h"      /* CMU clock config */
#include "IfxGtm_PinMap.h"   /* TOM pin map symbols */
#include "IfxPort.h"         /* Port driver (pad driver/output mode enums) */

/* ========================= Local definitions ========================= */
#define NUM_OF_CHANNELS                            (3u)

/* Duty ramp behavior (percent representation) */
#define DUTY_STEP_PERCENT                          (10.0f)
#define DUTY_MIN_PERCENT                           (10.0f)
#define DUTY_MAX_PERCENT                           (90.0f)

/* ========================= Driver state ========================= */
typedef struct {
    IfxGtm_Pwm          pwm;                                        /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];                  /* Channel runtime handles */
    float32             dutyCycles[NUM_OF_CHANNELS];                 /* Duty requests in percent */
    float32             phases[NUM_OF_CHANNELS];                     /* Phase offsets (deg or percent if used) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];                  /* Dead time (rising/falling) per channel */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;
static boolean      s_initialized = FALSE;

/* ========================= Public functions ========================= */
void GTM_TOM_3PhaseInverterPWM_init(void)
{
    /* 1) Enable GTM and configure CMU clocks per requirements */
    IfxGtm_enable(&MODULE_GTM);

    /* Route CLK0 from GCLK and set frequencies; enable FXCLK domain */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, GTM_GCLK_FREQ_HZ);
    IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);   /* useGlobal = TRUE selects GCLK as source */
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, GTM_GCLK_FREQ_HZ);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);     /* enable all Fx clocks */

    /* 2) Unified PWM configuration */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outputCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (pin routing, polarity, pad) */
    /* Phase U */
    outputCfg[0].pin                        = (IfxGtm_Pwm_ToutMap*)&PHASE_U_HS;
    outputCfg[0].complementaryPin           = (IfxGtm_Pwm_ToutMap*)&PHASE_U_LS;
    outputCfg[0].polarity                   = Ifx_ActiveState_high;
    outputCfg[0].complementaryPolarity      = Ifx_ActiveState_low;
    outputCfg[0].outputMode                 = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    outputCfg[1].pin                        = (IfxGtm_Pwm_ToutMap*)&PHASE_V_HS;
    outputCfg[1].complementaryPin           = (IfxGtm_Pwm_ToutMap*)&PHASE_V_LS;
    outputCfg[1].polarity                   = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity      = Ifx_ActiveState_low;
    outputCfg[1].outputMode                 = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    outputCfg[2].pin                        = (IfxGtm_Pwm_ToutMap*)&PHASE_W_HS;
    outputCfg[2].complementaryPin           = (IfxGtm_Pwm_ToutMap*)&PHASE_W_LS;
    outputCfg[2].polarity                   = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity      = Ifx_ActiveState_low;
    outputCfg[2].outputMode                 = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration (same for all phases per requirements) */
    dtmCfg[0].deadTime.rising  = PWM_DEADTIME_S;
    dtmCfg[0].deadTime.falling = PWM_DEADTIME_S;
    dtmCfg[1].deadTime.rising  = PWM_DEADTIME_S;
    dtmCfg[1].deadTime.falling = PWM_DEADTIME_S;
    dtmCfg[2].deadTime.rising  = PWM_DEADTIME_S;
    dtmCfg[2].deadTime.falling = PWM_DEADTIME_S;

    /* 5) Channel configuration (master timebase: TOM1 CH0; center-aligned) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].phase     = 0.0f;                    /* no inter-phase offset at init */
        channelConfig[i].dtm       = &dtmCfg[i];
        channelConfig[i].output    = &outputCfg[i];
        channelConfig[i].mscOut    = NULL_PTR;                /* not used */
        channelConfig[i].interrupt = NULL_PTR;                /* no ISR in this module */
    }

    /* Explicit channel indices for TOM submodule */
    channelConfig[0].timerCh = IfxGtm_Pwm_SubModule_Ch_0;  /* Master timebase */
    channelConfig[1].timerCh = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[2].timerCh = IfxGtm_Pwm_SubModule_Ch_2;

    /* Initial duties from requirements (percent) */
    channelConfig[0].duty = INITIAL_DUTY_PERCENT_U;
    channelConfig[1].duty = INITIAL_DUTY_PERCENT_V;
    channelConfig[2].duty = INITIAL_DUTY_PERCENT_W;

    /* 6) Top-level PWM configuration */
    config.cluster              = IfxGtm_Cluster_0;                 /* GTM Cluster 0 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;                              /* start after init */
    config.syncUpdateEnabled    = TRUE;                              /* shadow update at period boundary */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                  /* 20 kHz */
    config.clockSource.tom      = MASTER_TIMEBASE_CLOCK;             /* Fxclk0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;  /* DTM clock from CMU clock0 */

    /* 7) Initialize PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 8) Cache runtime state */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    g_gtmTom3phInv.phases[0]     = 0.0f;
    g_gtmTom3phInv.phases[1]     = 0.0f;
    g_gtmTom3phInv.phases[2]     = 0.0f;

    /* 9) Stage initial duties and enable outputs (shadow-update, latch at next period) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
    IfxGtm_Pwm_startChannelOutputs(&g_gtmTom3phInv.pwm);

    s_initialized = TRUE;
}

void GTM_TOM_3PhaseInverterPWM_updateDuties(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init did not complete */
    }

    /* Compute ramp: add step and wrap between [MIN, MAX] percent for each phase */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += DUTY_STEP_PERCENT;
        if (g_gtmTom3phInv.dutyCycles[i] >= DUTY_MAX_PERCENT)
        {
            g_gtmTom3phInv.dutyCycles[i] = DUTY_MIN_PERCENT;
        }
    }

    /* Apply synchronously: driver uses shadow registers and latches at next PWM period */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
