#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"   /* Generic PinMap header */
#include "IfxPort.h"
#include "IfxCpu.h"

/* ========================= Configuration Macros (from requirements) ========================= */
#define NUM_OF_CHANNELS                 (6u)
#define TIMEBASE_FREQUENCY_HZ           (20000.0f)         /* 20 kHz */
#define TIMEBASE_CLOCK_SOURCE           (IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0)
#define TIMEBASE_ALIGNMENT_CENTER       (1)                /* informational, configured via timer setup */
#define INITIAL_DUTY_PERCENT_U          (25.0f)
#define INITIAL_DUTY_PERCENT_V          (50.0f)
#define INITIAL_DUTY_PERCENT_W          (75.0f)
#define DUTY_UPDATE_POLICY_STEP_PERCENT (10.0f)
#define TIMING_SOFTWARE_DEAD_TIME_US    (0.5f)

/* ========================= Board Pin Mapping (retain existing mapping) ===================== */
/* KIT_A2G_TC387_5V_TFT mapping request: TOM1 CH1..CH6 as listed */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Module-local State ============================================== */
typedef struct
{
    IfxGtm_Pwm          pwm;                              /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channels returned by init */
    float32             dutyCycles[NUM_OF_CHANNELS];      /* Current duties for 6 channels (percent) */
} GtmTom3phInv_t;

static IfxGtm_Tom_Timer s_timebase;       /* Common TOM timebase */
static GtmTom3phInv_t   s_drv;            /* Driver state */
static boolean          s_initialized = FALSE;

/* ========================= Utilities ======================================================== */
/** Clamp value to [minVal, maxVal] */
static float32 clampFloat32(float32 value, float32 minVal, float32 maxVal)
{
    if (value < minVal) { return minVal; }
    if (value > maxVal) { return maxVal; }
    return value;
}

/* ========================= Public API: Initialization ====================================== */
void initGtmTomPwm(void)
{
    /* Enable GTM and FXCLK domain required by TOM */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* --- Configure TOM timer as common 20 kHz center-aligned timebase on Fxclk0 --- */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);

        timerCfg.base.frequency = TIMEBASE_FREQUENCY_HZ;           /* 20 kHz */
        timerCfg.clock          = TIMEBASE_CLOCK_SOURCE;           /* Fxclk0 */
        timerCfg.tom            = IfxGtm_Tom_1;                    /* TIMEBASE_MODULE = GTM.TOM1 */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;                 /* Use CH0 as master timebase */

        if (IfxGtm_Tom_Timer_init(&s_timebase, &timerCfg) == FALSE)
        {
            return; /* Early exit on failure - do not set s_initialized */
        }

        IfxGtm_Tom_Timer_updateInputFrequency(&s_timebase);
    }

    /* --- Map six PWM outputs (explicit PinMap as per SW Detailed Design) --- */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* --- Configure unified multi-channel PWM driver (6 independent TOM1 channels) --- */
    {
        IfxGtm_Pwm_Config        cfg;
        IfxGtm_Pwm_ChannelConfig chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig  outCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Alignment and clock source (TOM on Fxclk0), sync updates enabled */
        cfg.cluster                 = IfxGtm_Cluster_0;                   /* TOM1 belongs to Cluster_0 on TC3xx */
        cfg.subModule               = IfxGtm_Pwm_SubModule_tom;
        cfg.alignment               = IfxGtm_Pwm_Alignment_center;       /* Up-down */
        cfg.syncUpdateEnabled       = TRUE;                               /* Shadow update */
        cfg.frequency               = TIMEBASE_FREQUENCY_HZ;              /* Mirror the timebase frequency */
        cfg.clockSource.tom         = IfxGtm_Cmu_Fxclk_0;                 /* TOM uses Fxclk enum */

        /* Output config: six independent outputs, no hardware complementary pairing (software dead-time policy) */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS; outCfg[0].complementaryPin = NULL_PTR; outCfg[0].polarity = Ifx_ActiveState_high; outCfg[0].outputMode = IfxPort_OutputMode_pushPull; outCfg[0].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS; outCfg[1].complementaryPin = NULL_PTR; outCfg[1].polarity = Ifx_ActiveState_high; outCfg[1].outputMode = IfxPort_OutputMode_pushPull; outCfg[1].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS; outCfg[2].complementaryPin = NULL_PTR; outCfg[2].polarity = Ifx_ActiveState_high; outCfg[2].outputMode = IfxPort_OutputMode_pushPull; outCfg[2].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        outCfg[3].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS; outCfg[3].complementaryPin = NULL_PTR; outCfg[3].polarity = Ifx_ActiveState_high; outCfg[3].outputMode = IfxPort_OutputMode_pushPull; outCfg[3].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        outCfg[4].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS; outCfg[4].complementaryPin = NULL_PTR; outCfg[4].polarity = Ifx_ActiveState_high; outCfg[4].outputMode = IfxPort_OutputMode_pushPull; outCfg[4].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        outCfg[5].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS; outCfg[5].complementaryPin = NULL_PTR; outCfg[5].polarity = Ifx_ActiveState_high; outCfg[5].outputMode = IfxPort_OutputMode_pushPull; outCfg[5].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            chCfg[i].output = &outCfg[i];
            /* Explicit TOM1 channel assignment: U_HS=CH2, U_LS=CH1, V_HS=CH4, V_LS=CH3, W_HS=CH6, W_LS=CH5 */
            switch (i)
            {
                case 0u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_2; break; /* U HS -> TOM1 CH2 */
                case 1u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_1; break; /* U LS -> TOM1 CH1 */
                case 2u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_4; break; /* V HS -> TOM1 CH4 */
                case 3u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_3; break; /* V LS -> TOM1 CH3 */
                case 4u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_6; break; /* W HS -> TOM1 CH6 */
                case 5u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_5; break; /* W LS -> TOM1 CH5 */
                default: break;
            }
        }

        cfg.numChannels = NUM_OF_CHANNELS;
        cfg.channels    = &chCfg[0];

        /* Initialize the PWM driver with channel array */
        IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &cfg);
    }

    /* --- Start timebase and PWM channels --- */
    IfxGtm_Tom_Timer_run(&s_timebase);
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

    /* --- Compute and apply initial duties atomically (HS setpoints + LS complement minus SW DT) --- */
    {
        /* Derive dead-time fraction of period using Fxclk0 and period ticks */
        float32 fxclkHz      = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);
        Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&s_timebase);
        float32 deadTime_s   = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
        float32 deadFrac     = (periodTicks > 0u) ? ((fxclkHz * deadTime_s) / ((float32)periodTicks)) : 0.0f;
        float32 deadPct      = deadFrac * 100.0f;

        float32 duties[NUM_OF_CHANNELS];
        /* High-side initial duties */
        duties[0] = INITIAL_DUTY_PERCENT_U;  /* U HS */
        duties[2] = INITIAL_DUTY_PERCENT_V;  /* V HS */
        duties[4] = INITIAL_DUTY_PERCENT_W;  /* W HS */
        /* Low-side = 100 - HS - deadPct, clamped to [0,100] */
        duties[1] = clampFloat32(100.0f - duties[0] - deadPct, 0.0f, 100.0f); /* U LS */
        duties[3] = clampFloat32(100.0f - duties[2] - deadPct, 0.0f, 100.0f); /* V LS */
        duties[5] = clampFloat32(100.0f - duties[4] - deadPct, 0.0f, 100.0f); /* W LS */

        /* Store to driver state */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
        {
            s_drv.dutyCycles[i] = duties[i];
        }

        /* Atomic application using timer update gating */
        (void)IfxCpu_disableInterrupts();
        IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &duties[0]);
        IfxGtm_Tom_Timer_applyUpdate(&s_timebase);
        IfxCpu_enableInterrupts();
    }

    s_initialized = TRUE;
}

/* ========================= Public API: Periodic Update ===================================== */
void updateGtmTomPwmDutyCycles(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if not initialized */
    }

    /* 1) Read active period and Fxclk0 to derive dead-time fraction */
    Ifx_TimerValue periodTicks = IfxGtm_Tom_Timer_getPeriod(&s_timebase);
    float32 fxclkHz            = IfxGtm_Cmu_getFxClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Fxclk_0, TRUE);

    /* 2) Increment high-side duties by +10% with wrap in 0..100% */
    float32 hsU = s_drv.dutyCycles[0] + DUTY_UPDATE_POLICY_STEP_PERCENT; if (hsU >= 100.0f) { hsU -= 100.0f; }
    float32 hsV = s_drv.dutyCycles[2] + DUTY_UPDATE_POLICY_STEP_PERCENT; if (hsV >= 100.0f) { hsV -= 100.0f; }
    float32 hsW = s_drv.dutyCycles[4] + DUTY_UPDATE_POLICY_STEP_PERCENT; if (hsW >= 100.0f) { hsW -= 100.0f; }

    /* 3) Convert 0.5us dead-time to duty fraction of period */
    float32 deadTime_s = (TIMING_SOFTWARE_DEAD_TIME_US * 1.0e-6f);
    float32 deadFrac   = (periodTicks > 0u) ? ((fxclkHz * deadTime_s) / ((float32)periodTicks)) : 0.0f;
    float32 deadPct    = deadFrac * 100.0f;

    /* 4) Low-side = 100 - HS - deadPct, clamped to [0,100] */
    float32 lsU = clampFloat32(100.0f - hsU - deadPct, 0.0f, 100.0f);
    float32 lsV = clampFloat32(100.0f - hsV - deadPct, 0.0f, 100.0f);
    float32 lsW = clampFloat32(100.0f - hsW - deadPct, 0.0f, 100.0f);

    /* 5) Build duty array ordered to match configured channels: [U_HS, U_LS, V_HS, V_LS, W_HS, W_LS] */
    float32 duties[NUM_OF_CHANNELS];
    duties[0] = hsU; duties[1] = lsU; duties[2] = hsV; duties[3] = lsV; duties[4] = hsW; duties[5] = lsW;

    /* Update internal state */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; ++i)
    {
        s_drv.dutyCycles[i] = duties[i];
    }

    /* 6) Apply atomically using timer gating */
    (void)IfxCpu_disableInterrupts();
    IfxGtm_Tom_Timer_disableUpdate(&s_timebase);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &duties[0]);
    IfxGtm_Tom_Timer_applyUpdate(&s_timebase);
    IfxCpu_enableInterrupts();
}
