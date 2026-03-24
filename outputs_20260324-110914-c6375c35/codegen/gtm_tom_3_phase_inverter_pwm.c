/**********************************************************************************************************************
 * Module: GTM_TOM_3_Phase_Inverter_PWM
 * File:   gtm_tom_3_phase_inverter_pwm.c
 * Brief:  TC3xx GTM/TOM 3-phase inverter PWM using unified IfxGtm_Pwm with TOM time base
 *
 * Implements EXACT SW Detailed Design API contract and behavior:
 *  - GTM_TOM_3_Phase_Inverter_PWM_init():
 *      Enable GTM + CMU clocks, configure TOM time base for 20 kHz center-aligned PWM,
 *      mux six TOM outputs via IfxGtm_PinMap_setTomTout, initialize IfxGtm_Pwm with 3
 *      complementary pairs, synchronous updates, apply initial six-element duty array,
 *      and start synchronized PWM outputs.
 *  - GTM_TOM_3_Phase_Inverter_PWM_stepDuty():
 *      Maintain 3 stored duty percentages (U,V,W). On each call, add +10%; if >100%, wrap to 0.
 *      Build a six-element duty array (pairs share same duty) and apply atomically via
 *      IfxGtm_Pwm_updateChannelsDutyImmediate().
 *
 * Notes:
 *  - Uses GENERIC headers only (no family-specific pinmap headers).
 *  - No watchdog handling here; watchdog configuration belongs in CpuN_Main.c files.
 *********************************************************************************************************************/
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"

/* ========================= Local state ========================= */
static boolean                 s_initialized = FALSE;
static IfxGtm_Tom_Timer        s_timeBase;          /* TOM time base (master) */
static IfxGtm_Pwm              s_pwm;               /* Unified PWM driver */
static IfxGtm_Pwm_Channel      s_channels[GTM_TOM_PWM_NUM_PHASES];

/* Stored phase duties in percent (U, V, W) */
static float32 s_dutyU = INITIAL_DUTY_PERCENT_U;
static float32 s_dutyV = INITIAL_DUTY_PERCENT_V;
static float32 s_dutyW = INITIAL_DUTY_PERCENT_W;

/* ========================= Local helpers ========================= */
static void GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(float32 *outDutyPercent6)
{
    /* Map per-phase duty to complementary output pairs: [U_HS, U_LS, V_HS, V_LS, W_HS, W_LS] */
    outDutyPercent6[0] = s_dutyU; /* U_HS */
    outDutyPercent6[1] = s_dutyU; /* U_LS */
    outDutyPercent6[2] = s_dutyV; /* V_HS */
    outDutyPercent6[3] = s_dutyV; /* V_LS */
    outDutyPercent6[4] = s_dutyW; /* W_HS */
    outDutyPercent6[5] = s_dutyW; /* W_LS */
}

/* ========================= Public functions ========================= */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* 1) Enable GTM and configure CMU clocks for TOM operation */
    IfxGtm_enable(&MODULE_GTM);

    /* Set GCLK = module frequency, CLK0 = GCLK, enable FXCLK (and CLK0 if required) */
    {
        float32 modFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, modFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Configure TOM time base @ 20 kHz center-aligned (retain TOM time base) */
    {
        IfxGtm_Tom_Timer_Config tmrCfg;
        IfxGtm_Tom_Timer_initConfig(&tmrCfg, &MODULE_GTM);

        tmrCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;
        tmrCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        tmrCfg.tom            = IfxGtm_Tom_1;               /* Use TOM1 per requirement */
        tmrCfg.timerChannel   = IfxGtm_Tom_Ch_0;            /* Master timer channel */

        /* Initialize time base and check result */
        if (IfxGtm_Tom_Timer_init(&s_timeBase, &tmrCfg) == FALSE)
        {
            /* Early exit on failure (tests expect no further calls) */
            return;
        }

        /* Add the six working channels (1..6) to the time base channel mask */
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_1);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_2);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_3);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_4);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_5);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_6);

        /* Apply time base configuration */
        IfxGtm_Tom_Timer_applyUpdate(&s_timeBase);
    }

    /* 3) Mux six TOM outputs using GENERIC pin map API (P00.2..P00.7) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, GTM_TOM_PWM_OUTPUT_MODE, GTM_TOM_PWM_PAD_DRIVER);

    /* 4) Configure unified multi-channel PWM (3 complementary pairs) */
    {
        IfxGtm_Pwm_Config         cfg;
        IfxGtm_Pwm_ChannelConfig  chCfg[GTM_TOM_PWM_NUM_PHASES];
        IfxGtm_Pwm_OutputConfig   outCfg[GTM_TOM_PWM_NUM_PHASES];
        IfxGtm_Pwm_DtmConfig      dtmCfg[GTM_TOM_PWM_NUM_PHASES];
        uint8 i;

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Complementary outputs per phase (U, V, W) */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outCfg[0].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outCfg[0].polarity              = Ifx_ActiveState_high;
        outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[0].outputMode            = GTM_TOM_PWM_OUTPUT_MODE;
        outCfg[0].padDriver             = GTM_TOM_PWM_PAD_DRIVER;

        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outCfg[1].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outCfg[1].polarity              = Ifx_ActiveState_high;
        outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[1].outputMode            = GTM_TOM_PWM_OUTPUT_MODE;
        outCfg[1].padDriver             = GTM_TOM_PWM_PAD_DRIVER;

        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outCfg[2].complementaryPin      = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outCfg[2].polarity              = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[2].outputMode            = GTM_TOM_PWM_OUTPUT_MODE;
        outCfg[2].padDriver             = GTM_TOM_PWM_PAD_DRIVER;

        /* DTM dead-time per channel (example 1us both edges) */
        for (i = 0U; i < GTM_TOM_PWM_NUM_PHASES; i++)
        {
            dtmCfg[i].deadTime.rising  = 1.0e-6f;
            dtmCfg[i].deadTime.falling = 1.0e-6f;
        }

        /* Channel configs */
        for (i = 0U; i < GTM_TOM_PWM_NUM_PHASES; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            chCfg[i].dtm    = &dtmCfg[i];
            chCfg[i].output = &outCfg[i];
            chCfg[i].phase  = 0.0f; /* no phase shift */
        }
        /* Initial duties per phase (percent) */
        chCfg[0].duty = INITIAL_DUTY_PERCENT_U;
        chCfg[1].duty = INITIAL_DUTY_PERCENT_V;
        chCfg[2].duty = INITIAL_DUTY_PERCENT_W;

        /* Global PWM configuration */
        cfg.cluster            = IfxGtm_Cluster_0;
        cfg.subModule          = IfxGtm_Pwm_SubModule_tom;        /* TOM */
        cfg.alignment          = IfxGtm_Pwm_Alignment_center;     /* center-aligned */
        cfg.syncStart          = FALSE;                           /* we will call startSyncedChannels() explicitly */
        cfg.syncUpdateEnabled  = TRUE;                            /* atomic shadow updates */
        cfg.numChannels        = GTM_TOM_PWM_NUM_PHASES;          /* 3 complementary pairs */
        cfg.channels           = &chCfg[0];
        cfg.frequency          = TIMING_PWM_FREQUENCY_HZ;
        cfg.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;              /* TOM uses Fxclk enum */
        cfg.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;

        /* Initialize unified PWM driver (no return value) */
        IfxGtm_Pwm_init(&s_pwm, &s_channels[0], &cfg);
    }

    /* 5) Apply initial six-output duty array atomically and start synced channels */
    {
        float32 duty6[GTM_TOM_PWM_NUM_OUTPUTS];
        GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(duty6);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, duty6);
        IfxGtm_Pwm_startSyncedChannels(&s_pwm);
    }

    s_initialized = TRUE;
}

void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        /* Early exit if init failed or not performed yet (tests expect no driver calls) */
        return;
    }

    /* Increment and wrap per requirements */
    s_dutyU += GTM_TOM_PWM_DUTY_STEP; if (s_dutyU > 100.0f) { s_dutyU = 0.0f; }
    s_dutyV += GTM_TOM_PWM_DUTY_STEP; if (s_dutyV > 100.0f) { s_dutyV = 0.0f; }
    s_dutyW += GTM_TOM_PWM_DUTY_STEP; if (s_dutyW > 100.0f) { s_dutyW = 0.0f; }

    /* Build six-element duty array and apply atomically */
    {
        float32 duty6[GTM_TOM_PWM_NUM_OUTPUTS];
        GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(duty6);
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, duty6);
    }
}
