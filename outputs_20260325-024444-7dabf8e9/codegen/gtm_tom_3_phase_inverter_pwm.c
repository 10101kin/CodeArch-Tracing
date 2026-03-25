/*
 * gtm_tom_3_phase_inverter_pwm.c
 * 3-phase inverter PWM using GTM TOM with unified IfxGtm_Pwm driver and TOM timebase
 */
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/*
 * Pin mapping: choose six outputs all within TOM0 TGC0 (channels 0..7) for atomic updates
 * Phase U: CH0 high and CH0N low
 * Phase V: CH1 high and CH1N low
 * Phase W: CH2 high and CH2N low
 *
 * NOTE: These pins are selected from validated TC3xx PinMap symbols. Verify against schematic.
 */
#define PHASE_U_HS   (&IfxGtm_TOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxGtm_TOM0_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM0_1_TOUT54_P21_3_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM0_1N_TOUT52_P21_1_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM0_2_TOUT88_P14_8_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM0_2N_TOUT93_P13_2_OUT)

/* Internal driver state */
typedef struct
{
    IfxGtm_Pwm          pwm;                           /* unified PWM driver */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];     /* populated by init */
    IfxGtm_Tom_Timer    timebase;                      /* TOM timebase for TGC update gating */
    float32             dutyCycles[NUM_OF_CHANNELS];   /* HS duty in percent [0..100] for U,V,W */
    float32             dutyStep;                      /* step percent */
} GtmTom3phInv_Driver;

static GtmTom3phInv_Driver s_drv;
static boolean s_initialized = FALSE;

/* Private helpers */
static void configurePwmPins(void)
{
    /* Route six PWM outputs using generic PinMap API */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* Initialize the GTM for a 3-phase inverter using TOM with shared timebase and synchronous update gating */
void initGtmTom3phInv(void)
{
    /* 1) Enable GTM and required CMU clocks */
    IfxGtm_enable(&MODULE_GTM);

    /* Clock setup per requirements: GCLK=300 MHz, CLK0=100 MHz; enable FXCLK and CLK0; select CLK0 from GCLK */
    IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, CLOCK_GTM_GCLK_HZ);
    IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);
    IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, CLOCK_CMU_CLK0_HZ);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));

    /* 2) Configure TOM timebase channel for 10 kHz center-aligned (shared period/alignment) */
    {
        IfxGtm_Tom_Timer_Config timerCfg;
        IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
        timerCfg.base.frequency = PWM_FREQUENCY_HZ;
        timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        timerCfg.tom            = IfxGtm_Tom_0;               /* TOM0 per requirements */
        timerCfg.timerChannel   = IfxGtm_Tom_Ch_7;            /* Dedicated timebase channel in TGC0 */
        /* Initialize and check result */
        if (IfxGtm_Tom_Timer_init(&s_drv.timebase, &timerCfg) == FALSE)
        {
            return; /* early exit on failure */
        }
    }

    /* 3) Route six PWM outputs (3 complementary pairs) to pins */
    configurePwmPins();

    /* 4) Build unified PWM configuration with three synchronized channels (complementary pins via OutputConfig) */
    {
        IfxGtm_Pwm_Config          cfg;
        IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig    outCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Output configuration with complementary pins and polarity */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outCfg[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outCfg[0].polarity              = Ifx_ActiveState_high;
        outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outCfg[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outCfg[1].polarity              = Ifx_ActiveState_high;
        outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outCfg[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outCfg[2].polarity              = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Dead-time configuration: 500 ns rising/falling per phase */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            dtmCfg[i].deadTime.rising  = DEAD_TIME_SEC;
            dtmCfg[i].deadTime.falling = DEAD_TIME_SEC;
        }

        /* Per-channel configuration (timer channel, duty, phase, dtm, output) */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            chCfg[i].phase     = 0.0f;
            chCfg[i].dtm       = &dtmCfg[i];
            chCfg[i].output    = &outCfg[i];
            chCfg[i].mscOut    = NULL_PTR;
            chCfg[i].interrupt = NULL_PTR; /* no ISR required */
        }
        chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_0; chCfg[0].duty = DUTY_25_PERCENT; /* U */
        chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_1; chCfg[1].duty = DUTY_50_PERCENT; /* V */
        chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_2; chCfg[2].duty = DUTY_75_PERCENT; /* W */

        /* Global PWM configuration */
        cfg.cluster              = IfxGtm_Cluster_0;
        cfg.subModule            = IfxGtm_Pwm_SubModule_tom;
        cfg.alignment            = IfxGtm_Pwm_Alignment_center;
        cfg.syncStart            = TRUE;
        cfg.syncUpdateEnabled    = TRUE;                    /* shadow update at period */
        cfg.numChannels          = NUM_OF_CHANNELS;
        cfg.channels             = chCfg;
        cfg.frequency            = PWM_FREQUENCY_HZ;
        cfg.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;      /* TOM uses Fxclk */
        cfg.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

        /* Initialize PWM driver (no return value) */
        IfxGtm_Pwm_init(&s_drv.pwm, s_drv.channels, &cfg);

        /* 6) Program dead-time via driver (explicit immediate set for clarity) */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_updateChannelDeadTimeImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)i, dtmCfg[i].deadTime);
        }
    }

    /* 7) Gate TGC updates: disable, write initial duties, apply atomically */
    s_drv.dutyCycles[0] = DUTY_25_PERCENT;  /* U_HS */
    s_drv.dutyCycles[1] = DUTY_50_PERCENT;  /* V_HS */
    s_drv.dutyCycles[2] = DUTY_75_PERCENT;  /* W_HS */
    s_drv.dutyStep      = DUTY_STEP_PERCENT;

    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timebase);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0, s_drv.dutyCycles[0]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1, s_drv.dutyCycles[1]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2, s_drv.dutyCycles[2]);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timebase);

    /* 8) Start synchronized PWM outputs */
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

    s_initialized = TRUE;
}

/* Runtime duty-step behavior: +10% each call, wrap 0..100, complementary implied by driver */
void updateGtmTom3phInvDuty(void)
{
    if (s_initialized == FALSE)
    {
        return;
    }

    /* Step high-side duties; compute complementary (implicit in driver by complementaryPolarity) */
    float32 hs[NUM_OF_CHANNELS];
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        float32 d = s_drv.dutyCycles[i] + s_drv.dutyStep;
        if (d >= 100.0f)
        {
            d -= 100.0f; /* wrap into 0..100 */
        }
        s_drv.dutyCycles[i] = d;
        hs[i] = d;
        /* low-side duty would be (100.0f - d); complementary handled by hardware DTM */
    }

    /* Gate and apply atomically for all channels in TGC */
    IfxGtm_Tom_Timer_disableUpdate(&s_drv.timebase);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)0, hs[0]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)1, hs[1]);
    IfxGtm_Pwm_updateChannelDutyImmediate(&s_drv.pwm, (IfxGtm_Pwm_SyncChannelIndex)2, hs[2]);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timebase);
}
