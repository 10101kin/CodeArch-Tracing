#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* =====================================================================
 * Requirements-driven configuration (HIGH PRIORITY)
 * ===================================================================== */
#define MASTER_TIMEBASE_TOM_MODULE                 (1u)            /* TOM1 */
#define MASTER_TIMEBASE_CHANNEL                    (0u)            /* CH0 as master time base */
#define MASTER_TIMEBASE_CLOCK                      (IfxGtm_Cmu_Fxclk_0)  /* cmuFxclk0 */
#define MASTER_TIMEBASE_CLOCK_SOURCE_IS_GCLK       (TRUE)
#define MASTER_TIMEBASE_CENTER_ALIGNED             (TRUE)

#define INITIAL_DUTY_PERCENT_U                     (25.0f)
#define INITIAL_DUTY_PERCENT_V                     (50.0f)
#define INITIAL_DUTY_PERCENT_W                     (75.0f)

#define SYNCHRONIZED_UPDATE_USE_DISABLE_APPLY_UPDATE   (TRUE)
/* Note: unified IfxGtm_Pwm uses shadow updates when syncUpdateEnabled=TRUE. */

#define TIMING_PWM_FREQUENCY_HZ                    (20000.0f)      /* 20 kHz */
#define TIMING_DEADTIME_US                         (0.5f)          /* microseconds */
#define TIMING_MIN_PULSE_US                        (1.0f)          /* microseconds */
#define CLOCK_REQUIRES_XTAL                        (TRUE)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ             (300u)
#define CLOCK_EXPECTED_GTM_GCLK_MHZ                (100u)

/* Runtime ramp parameters (percent domain as per unified driver expectations) */
#define DUTY_STEP                                  (10.0f)         /* step in % each call */
#define DUTY_MIN                                   (10.0f)         /* lower threshold in % */
#define DUTY_MAX                                   (90.0f)         /* upper threshold in % */

/* =====================================================================
 * Channel/pin topology
 * - 3 complementary pairs (U, V, W)
 * - Use validated TOM1 routes on TC3xx (reference-proven P00 pins)
 *   Note: Port 02 TOM1 routes are not available in validated list; use TOM1 P00.*
 * ===================================================================== */
#define NUM_OF_CHANNELS                            (3u)

/* TOM1 complementary pairs on P00.x (validated reference mappings for TC3xx) */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ISR diagnostic LED (TC3xx generic) */
#define LED_PORT     (&MODULE_P13)
#define LED_PIN      (0u)

/* ISR priority macro name must match reference pattern */
#define ISR_PRIORITY_TOM  (20)

/* =====================================================================
 * Driver state structure per unified IfxGtm_Pwm pattern (TC3xx, TOM)
 * ===================================================================== */
typedef struct {
    IfxGtm_Pwm          pwm;                                      /* PWM Driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];                /* Channel state (post-init) */
    float32             dutyCycles[NUM_OF_CHANNELS];              /* Duty cycle values in percent */
    float32             phases[NUM_OF_CHANNELS];                  /* Phase shift values in percent of period */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];               /* Dead-time values (seconds) */
} GtmTom3phInv;

static GtmTom3phInv g_gtmTom3phInv;  /* internal driver instance */
static boolean      s_initialized = FALSE;

/* =====================================================================
 * Interrupt service routine (diagnostic)
 * ===================================================================== */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED_PORT, (uint8)LED_PIN);
}

/* Period event callback prototype (unified driver interrupt config) */
void IfxGtm_periodEventFunction(void *data)
{
    /* Called on PWM period event (optional diagnostic hook) */
    (void)data;
}

/* =====================================================================
 * Initialization per SW Detailed Design (unified IfxGtm_Pwm)
 * ===================================================================== */
void GTM_TOM_3PhaseInverterPWM_init(void)
{
    /* 1) Enable GTM and configure CMU clocks derived from GCLK */
    IfxGtm_enable(&MODULE_GTM);

    /* Program GCLK to module frequency and route CLK0 from GCLK */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM); /* Typically ~100 MHz cluster */
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, MASTER_TIMEBASE_CLOCK_SOURCE_IS_GCLK);
        /* Enable FXCLK and CLK0 distribution */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Build unified PWM configuration */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outputCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 2a) Interrupt configuration (period event on base channel) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_TOM;           /* exact macro name */
    irqCfg.periodEvent = IfxGtm_periodEventFunction; /* callback */
    irqCfg.dutyEvent   = NULL_PTR;

    /* 2b) Output configuration for complementary pairs (pins routed via unified driver) */
    outputCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    outputCfg[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    outputCfg[0].polarity              = Ifx_ActiveState_high;          /* active high */
    outputCfg[0].complementaryPolarity = Ifx_ActiveState_low;           /* complementary low */
    outputCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outputCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    outputCfg[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    outputCfg[1].polarity              = Ifx_ActiveState_high;
    outputCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outputCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    outputCfg[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    outputCfg[2].polarity              = Ifx_ActiveState_high;
    outputCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outputCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outputCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 2c) DTM configuration (dead-time per channel). Units: seconds */
    {
        const float32 deadTime_s = (TIMING_DEADTIME_US * 1.0e-6f);  /* 0.5 us */
        dtmCfg[0].deadTime = deadTime_s;
        dtmCfg[1].deadTime = deadTime_s;
        dtmCfg[2].deadTime = deadTime_s;
    }

    /* 2d) Channel configuration */
    for (uint8 i = 0u; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].timerCh   = (IfxGtm_Pwm_SubModule_Ch)((uint8)MASTER_TIMEBASE_CHANNEL + i); /* CH0..CH2 */
        channelConfig[i].phase     = 0.0f;                         /* no phase shift */
        channelConfig[i].dtm       = &dtmCfg[i];                   /* link DTM */
        channelConfig[i].output    = &outputCfg[i];                /* link outputs */
        channelConfig[i].mscOut    = NULL_PTR;                     /* not used */
        channelConfig[i].interrupt = (i == 0u) ? &irqCfg : NULL_PTR; /* period ISR on base channel */
    }

    /* Initial duties in percent (U,V,W) */
    channelConfig[0].duty = INITIAL_DUTY_PERCENT_U;
    channelConfig[1].duty = INITIAL_DUTY_PERCENT_V;
    channelConfig[2].duty = INITIAL_DUTY_PERCENT_W;

    /* 3) Top-level PWM configuration fields */
    config.cluster           = IfxGtm_Cluster_0;                     /* default cluster */
    config.subModule         = IfxGtm_Pwm_SubModule_tom;             /* TOM */
    config.alignment         = IfxGtm_Pwm_Alignment_center;          /* center-aligned */
    config.syncStart         = TRUE;                                  /* auto-start after init */
    config.syncUpdateEnabled = TRUE;                                  /* shadow updates at period boundary */
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = (float32)TIMING_PWM_FREQUENCY_HZ;      /* 20 kHz */
    config.clockSource.tom   = MASTER_TIMEBASE_CLOCK;                 /* Fxclk0 (derived from GCLK/CLK0) */
    config.dtmClockSource    = IfxGtm_Dtm_ClockSource_cmuClock0;      /* DTM clock from CMU CLK0 */

    /* 4) Initialize the PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 5) Store runtime mirrors */
    g_gtmTom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;
    g_gtmTom3phInv.phases[0]     = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]     = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]     = channelConfig[2].phase;

    /* 6) Stage initial duties into shadow registers (syncUpdateEnabled ensures synchronous latch) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);

    /* 7) Enable PWM outputs */
    IfxGtm_Pwm_startChannelOutputs(&g_gtmTom3phInv.pwm);

    /* 8) Configure LED pin for ISR diagnostic */
    IfxPort_setPinModeOutput(LED_PORT, (uint8)LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

/* =====================================================================
 * Runtime duty update (ramp between DUTY_MIN and DUTY_MAX with wrap)
 * ===================================================================== */
void GTM_TOM_3PhaseInverterPWM_updateDuties(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init not completed */
    }

    for (uint8 i = 0u; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += (float32)DUTY_STEP;
        if (g_gtmTom3phInv.dutyCycles[i] >= (float32)DUTY_MAX)
        {
            g_gtmTom3phInv.dutyCycles[i] = (float32)DUTY_MIN;
        }
    }

    /* Apply synchronously at next PWM period boundary (shadow update) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.dutyCycles[0]);
}
