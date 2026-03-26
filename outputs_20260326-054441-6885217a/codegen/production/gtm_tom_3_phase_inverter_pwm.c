/***************************************************************************
 * @file    gtm_tom_3_phase_inverter_pwm.c
 * @brief   GTM TOM 3-Phase Inverter PWM driver implementation (TC3xx)
 *
 * Fulfills SW Detailed Design using real iLLD drivers with exact signatures:
 * - Unified PWM driver: IfxGtm_Pwm (init, update, start)
 * - Pin routing: IfxGtm_PinMap_setTomTout
 * - CMU clocks: IfxGtm_Cmu_setGclkFrequency / setClkFrequency / enableClocks
 * - GTM enable: IfxGtm_enable
 *
 * Behavior (init):
 *   - Enable GTM and functional clocks (FXCLK/CLK0)
 *   - Configure center-aligned, complementary outputs on TOM1 with P00 pins
 *   - Program 20 kHz frequency, 0.5us dead-time, 1.0us min-pulse
 *   - Apply initial duty vector [25, 50, 75] % synchronously
 *   - Start synced channels and enable outputs
 *
 * Behavior (update):
 *   - Increment each phase duty by DUTY_STEP_PERCENT
 *   - Wrap to DUTY_MIN_PERCENT when reaching/exceeding DUTY_MAX_PERCENT
 *   - Apply single synchronous duty update
 *
 * Watchdog note: No watchdog API calls here (must be in CpuN_Main.c only)
 ***************************************************************************/
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ========================= Validated pin mappings for KIT_A2G_TC387_5V_TFT ========================= */
/* High-side (CCX) */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT)
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT)
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT)
/* Low-side (COUTX) */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT)
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT)
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT)

/* ========================= Driver state ========================= */
typedef struct
{
    IfxGtm_Pwm          pwm;                               /* Unified PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel data populated by init */
    float32             dutyPercent[NUM_OF_CHANNELS];       /* Duty in percent [0..100] */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];         /* Dead-time per channel */
} GtmTom3phInv_Driver;

static GtmTom3phInv_Driver s_drv;
static boolean             s_initialized = FALSE;

/* ========================= Private helpers ========================= */
static void GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal(void)
{
    uint8 i;
    for (i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        s_drv.dutyPercent[i] += DUTY_STEP_PERCENT;
        if (s_drv.dutyPercent[i] >= DUTY_MAX_PERCENT)
        {
            s_drv.dutyPercent[i] = DUTY_MIN_PERCENT;
        }
    }
}

/* ========================= Public API ========================= */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Enable GTM and CMU clocks required by TOM */
    IfxGtm_enable(&MODULE_GTM);
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        /* Enable FXCLK (and keep CLK0 enabled by setClkFrequency) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Route pins for complementary pairs using PinMap API (as per SW design) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_U_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_U_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_V_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_V_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_W_HS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap*)PHASE_W_LS, OUTPUT_MODE, OUTPUT_PAD_DRIVER);

    /* 3) Build unified PWM configuration and per-channel setup */
    {
        IfxGtm_Pwm_Config           cfg;
        IfxGtm_Pwm_ChannelConfig    chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig        dtmCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Complementary output configs for U, V, W (active-high for both CCX and COUTX) */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
        outCfg[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
        outCfg[0].polarity              = OUTPUT_ACTIVE_STATE_CCX;
        outCfg[0].complementaryPolarity = OUTPUT_ACTIVE_STATE_COUTX;
        outCfg[0].outputMode            = OUTPUT_MODE;
        outCfg[0].padDriver             = OUTPUT_PAD_DRIVER;

        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
        outCfg[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
        outCfg[1].polarity              = OUTPUT_ACTIVE_STATE_CCX;
        outCfg[1].complementaryPolarity = OUTPUT_ACTIVE_STATE_COUTX;
        outCfg[1].outputMode            = OUTPUT_MODE;
        outCfg[1].padDriver             = OUTPUT_PAD_DRIVER;

        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
        outCfg[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
        outCfg[2].polarity              = OUTPUT_ACTIVE_STATE_CCX;
        outCfg[2].complementaryPolarity = OUTPUT_ACTIVE_STATE_COUTX;
        outCfg[2].outputMode            = OUTPUT_MODE;
        outCfg[2].padDriver             = OUTPUT_PAD_DRIVER;

        /* Dead-time (uniform across all pairs) */
        dtmCfg[0].deadTime = PWM_DEAD_TIME_S;
        dtmCfg[1].deadTime = PWM_DEAD_TIME_S;
        dtmCfg[2].deadTime = PWM_DEAD_TIME_S;

        /* Channel configs (TOM submodule, TOM1 base time, center-aligned, sync start/update) */
        /* Base channel 0 (maps to TOM1_CH0 as time base by configuration) */
        IfxGtm_Pwm_initChannelConfig(&chCfg[0]);
        chCfg[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
        chCfg[0].phase     = 0.0f;
        chCfg[0].duty      = DUTY_U_INIT_PERCENT;  /* U */
        chCfg[0].dtm       = &dtmCfg[0];
        chCfg[0].output    = &outCfg[0];
        chCfg[0].mscOut    = NULL_PTR;
        chCfg[0].interrupt = NULL_PTR;            /* No ISR required per SW design */

        /* Channel 1 (V) */
        IfxGtm_Pwm_initChannelConfig(&chCfg[1]);
        chCfg[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
        chCfg[1].phase     = 0.0f;
        chCfg[1].duty      = DUTY_V_INIT_PERCENT;  /* V */
        chCfg[1].dtm       = &dtmCfg[1];
        chCfg[1].output    = &outCfg[1];
        chCfg[1].mscOut    = NULL_PTR;
        chCfg[1].interrupt = NULL_PTR;

        /* Channel 2 (W) */
        IfxGtm_Pwm_initChannelConfig(&chCfg[2]);
        chCfg[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
        chCfg[2].phase     = 0.0f;
        chCfg[2].duty      = DUTY_W_INIT_PERCENT;  /* W */
        chCfg[2].dtm       = &dtmCfg[2];
        chCfg[2].output    = &outCfg[2];
        chCfg[2].mscOut    = NULL_PTR;
        chCfg[2].interrupt = NULL_PTR;

        cfg.cluster              = IfxGtm_Cluster_1;                 /* TOM1 cluster */
        cfg.subModule            = IfxGtm_Pwm_SubModule_tom;         /* Use TOM */
        cfg.alignment            = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
        cfg.syncStart            = TRUE;                             /* Auto-start after init */
        cfg.syncUpdateEnabled    = TRUE;                             /* Shadow update */
        cfg.numChannels          = NUM_OF_CHANNELS;                  /* 3 phases */
        cfg.channels             = &chCfg[0];                        /* Channel array */
        cfg.frequency            = PWM_FREQUENCY_HZ;                 /* 20 kHz */
        cfg.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;               /* Fxclk0 */
        cfg.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock */

        /* 4) Initialize unified PWM driver with channels array */
        IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &cfg);

        /* 5) Program frequency immediately (explicit per SW design) */
        IfxGtm_Pwm_updateFrequencyImmediate(&s_drv.pwm, PWM_FREQUENCY_HZ);

        /* 6) Program dead-time immediately (uniform) */
        s_drv.deadTimes[0] = dtmCfg[0].deadTime;
        s_drv.deadTimes[1] = dtmCfg[1].deadTime;
        s_drv.deadTimes[2] = dtmCfg[2].deadTime;
        IfxGtm_Pwm_updateChannelsDeadTimeImmediate(&s_drv.pwm, &s_drv.deadTimes[0]);

        /* 7) Prepare and apply initial duty vector synchronously */
        s_drv.dutyPercent[0] = DUTY_U_INIT_PERCENT;
        s_drv.dutyPercent[1] = DUTY_V_INIT_PERCENT;
        s_drv.dutyPercent[2] = DUTY_W_INIT_PERCENT;
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &s_drv.dutyPercent[0]);

        /* 8) Finally, start synced channels and enable outputs */
        IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);
        IfxGtm_Pwm_startChannelOutputs(&s_drv.pwm);

        s_initialized = TRUE;
    }
}

void GTM_TOM_3_Phase_Inverter_PWM_updateUVW(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init did not complete */
    }

    /* Update runtime duty array per SW design: increment, bound, wrap */
    GTM_TOM_3_Phase_Inverter_PWM_updateDutyArrayInternal();

    /* Single synchronous update for all 3 phases */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, &s_drv.dutyPercent[0]);
}
