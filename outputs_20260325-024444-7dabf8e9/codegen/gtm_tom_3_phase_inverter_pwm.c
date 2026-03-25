#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxCpu.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxPort.h"

/* Internal driver/state structures */
typedef struct {
    IfxGtm_Pwm          pwm;                            /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];      /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];    /* High-side duty in percent */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];     /* Dead-time values */
} GtmTom3phInv;

static GtmTom3phInv     g_gtmTom3phInv;
static IfxGtm_Tom_Timer g_tomTimer;                     /* Shared timebase for TGC atomic gating */
static boolean          s_initialized = FALSE;

/* TOM ISR: use reference macro name ISR_PRIORITY_TOM */
IFX_INTERRUPT(interruptGtmTom, 0, ISR_PRIORITY_TOM);
void interruptGtmTom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback (linked via interrupt config if needed) */
void IfxGtm_periodEventFunction(void *data)
{
    /* User period event hook (not used) */
    (void)data;
}

/* Private: explicit pin routing using generic PinMap API (as per SW Detailed Design) */
static void configurePwmPins(void)
{
    /* Route six TOM outputs: three complementary pairs */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

void initGtmTom3phInv(void)
{
    /*
     * Behavior steps per SW Detailed Design:
     * 1) Enable GTM + CMU clocks; select CLK0 from GCLK; set GCLK and CLK0 frequencies; enable FXCLK/CLK0
     */
    IfxGtm_enable(&MODULE_GTM);

    /* Select CLK0 to use GCLK */
    IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);

    /* Program GTM clocks per requirements */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, (float32)(CLOCK_GTM_GCLK_MHZ * 1.0e6f));
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, (float32)(CLOCK_CMU_CLK0_MHZ * 1.0e6f));

    /* Enable FXCLK and CLK0 */
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));

    /* 2) Configure dedicated TOM timebase for 10 kHz center-aligned shared period */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        timerCfg.base.frequency = PWM_FREQUENCY_HZ;
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        timerCfg.tom            = TOM_SELECTION_MODULE;   /* TOM0 */
        timerCfg.timerChannel   = TOM_MASTER_TIMER_CH;    /* e.g., TOM0 CH7 within TGC0 */

        /* Init timebase; check return for robust error handling */
        if (IfxGtm_Tom_Timer_init(&g_tomTimer, &timerCfg) == FALSE)
        {
            return; /* Early exit on failure; do not set s_initialized */
        }
    }

    /* 3) Route six PWM outputs (three complementary pairs) via generic PinMap API */
    configurePwmPins();

    /* 4) Build unified PWM configuration with 3 channels (each with complementary pin) on TOM0 */
    IfxGtm_Pwm_Config          cfg;
    IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig irqCfg;

    IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

    /* Interrupt on base channel (optional callback) */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_TOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Output + DTM configuration per phase */
    /* Phase U (channel 0) */
    outCfg[0].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    outCfg[0].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    outCfg[0].polarity                 = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity    = Ifx_ActiveState_low;
    outCfg[0].outputMode               = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V (channel 1) */
    outCfg[1].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    outCfg[1].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    outCfg[1].polarity                 = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity    = Ifx_ActiveState_low;
    outCfg[1].outputMode               = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W (channel 2) */
    outCfg[2].pin                      = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    outCfg[2].complementaryPin         = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    outCfg[2].polarity                 = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity    = Ifx_ActiveState_low;
    outCfg[2].outputMode               = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 6) Program dead-time of 500 ns (hardware DTM path) */
    dtmCfg[0].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmCfg[0].deadTime.falling = DEAD_TIME_SECONDS;
    dtmCfg[1].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmCfg[1].deadTime.falling = DEAD_TIME_SECONDS;
    dtmCfg[2].deadTime.rising  = DEAD_TIME_SECONDS;
    dtmCfg[2].deadTime.falling = DEAD_TIME_SECONDS;

    /* Channel config (explicit per-channel) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
        chCfg[i].phase     = 0.0f;
        chCfg[i].dtm       = &dtmCfg[i];
        chCfg[i].output    = &outCfg[i];
        chCfg[i].mscOut    = NULL_PTR;
        chCfg[i].interrupt = (i == 0u) ? &irqCfg : NULL_PTR; /* Interrupt on base channel */
    }

    /* Map logical channels to TOM0 channels 0,1,2 and assign initial duty */
    chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_0; chCfg[0].duty = INITIAL_DUTY_PERCENT_U; /* U */
    chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_1; chCfg[1].duty = INITIAL_DUTY_PERCENT_V; /* V */
    chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_2; chCfg[2].duty = INITIAL_DUTY_PERCENT_W; /* W */

    /* Top-level PWM config */
    cfg.cluster              = IfxGtm_Cluster_0;
    cfg.subModule            = IfxGtm_Pwm_SubModule_tom;
    cfg.alignment            = IfxGtm_Pwm_Alignment_center;
    cfg.syncStart            = TRUE;      /* Start channels after init */
    cfg.syncUpdateEnabled    = TRUE;      /* Shadow update at period event */
    cfg.numChannels          = NUM_OF_CHANNELS;
    cfg.channels             = &chCfg[0];
    cfg.frequency            = PWM_FREQUENCY_HZ;
    cfg.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;      /* TOM uses Fxclk */
    cfg.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 5) Initialize PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &cfg);

    /* Optionally force dead-time via runtime API for robustness */
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0u, dtmCfg[0].deadTime);
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1u, dtmCfg[1].deadTime);
    IfxGtm_Pwm_updateChannelDeadTimeImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2u, dtmCfg[2].deadTime);

    /* Store initial runtime values */
    g_gtmTom3phInv.dutyCycles[0] = chCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = chCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = chCfg[2].duty;
    g_gtmTom3phInv.deadTimes[0]  = dtmCfg[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmCfg[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmCfg[2].deadTime;

    /* 7) Gate updates: program initial duties atomically (DTM computes complementary) */
    IfxGtm_Tom_Timer_disableUpdate(&g_tomTimer);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0u, INITIAL_DUTY_PERCENT_U);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1u, INITIAL_DUTY_PERCENT_V);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2u, INITIAL_DUTY_PERCENT_W);
    IfxGtm_Tom_Timer_applyUpdate(&g_tomTimer);

    /* 8) Start synchronized PWM outputs and run timebase */
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);
    IfxGtm_Tom_Timer_run(&g_tomTimer);

    /* Configure LED pin for ISR toggling */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    s_initialized = TRUE;
}

void updateGtmTom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early exit if init failed/not done */
    }

    /* 1) Increase each phase's high-side duty by +10% */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = g_gtmTom3phInv.dutyCycles[i] + DUTY_STEP_PERCENT;
        /* 2) Wrap 0..100% */
        if (d >= 100.0f)
        {
            d -= 100.0f;
        }
        g_gtmTom3phInv.dutyCycles[i] = d;
    }

    /* 3) Complementary low-side is 100% - high-side (handled by DTM in hardware) */
    /* 4) Gate TGC updates, 5) Write new duties immediately for all three phase channels */
    IfxGtm_Tom_Timer_disableUpdate(&g_tomTimer);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0u, g_gtmTom3phInv.dutyCycles[0]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1u, g_gtmTom3phInv.dutyCycles[1]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&g_gtmTom3phInv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2u, g_gtmTom3phInv.dutyCycles[2]);
    /* 6) Atomically apply the update */
    IfxGtm_Tom_Timer_applyUpdate(&g_tomTimer);
}
