/**********************************************************************************************************************
 * 
 *  File: gtm_tom_3_phase_inverter_pwm.c
 *  Brief: GTM TOM 3-Phase Inverter PWM - Production code using IfxGtm_Pwm unified driver
 *  Target: TC3xx (TC387)
 *
 *  Requirements implemented:
 *   - 20 kHz center-aligned PWM, synchronous shadow updates
 *   - Use IfxGtm_Pwm (unified) with TOM submodule; retain IfxGtm_Tom_Timer as time base
 *   - Keep TOM1 channels 1..6 on P00.2..P00.7
 *   - Initialize duties: U=25%%, V=50%%, W=75%%
 *   - Step each channel by +10%% every 500 ms (timing handled by caller)
 *
 *  Notes:
 *   - Watchdogs are NOT managed here (handled only in CpuN_Main.c per architecture rules)
 *   - Generic headers used (no family-specific pinmap headers)
 *
 *********************************************************************************************************************/

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD includes (generic) */
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ===================================================================================================================
 * Configuration macros from requirements
 * =================================================================================================================== */
#define NUM_OF_CHANNELS                 (6u)
#define TIMING_PWM_FREQUENCY_HZ         (20000.0f)   /* 20 kHz from requirements */
#define TIMING_UPDATE_INTERVAL_MS       (500u)       /* Informational; caller controls timing */

#define INITIAL_DUTY_PERCENT_U          (25.0f)
#define INITIAL_DUTY_PERCENT_V          (50.0f)
#define INITIAL_DUTY_PERCENT_W          (75.0f)
#define DUTY_STEP_PERCENT               (10.0f)      /* +10% per step */

/* Optional informational macros (not directly used here) */
#define CLOCK_REQUIRES_XTAL             (1)
#define CLOCK_EXPECTED_SYSTEM_FREQ_MHZ  (300u)

/* ===================================================================================================================
 * Pin assignments (KIT_A2G_TC387_5V_TFT: Keep TOM1 ch1..6 on P00.2..P00.7)
 * Use GENERIC pin map header symbols
 * =================================================================================================================== */
#define PHASE_U_HS   (&IfxGtm_TOM1_2_TOUT12_P00_3_OUT) /* U high-side:  TOM1 ch2 -> P00.3 (TOUT12) */
#define PHASE_U_LS   (&IfxGtm_TOM1_1_TOUT11_P00_2_OUT) /* U low-side:   TOM1 ch1 -> P00.2 (TOUT11) */
#define PHASE_V_HS   (&IfxGtm_TOM1_4_TOUT14_P00_5_OUT) /* V high-side:  TOM1 ch4 -> P00.5 (TOUT14) */
#define PHASE_V_LS   (&IfxGtm_TOM1_3_TOUT13_P00_4_OUT) /* V low-side:   TOM1 ch3 -> P00.4 (TOUT13) */
#define PHASE_W_HS   (&IfxGtm_TOM1_6_TOUT16_P00_7_OUT) /* W high-side:  TOM1 ch6 -> P00.7 (TOUT16) */
#define PHASE_W_LS   (&IfxGtm_TOM1_5_TOUT15_P00_6_OUT) /* W low-side:   TOM1 ch5 -> P00.6 (TOUT15) */

/* ===================================================================================================================
 * Driver state
 * =================================================================================================================== */

typedef struct
{
    IfxGtm_Pwm          pwm;                              /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];        /* Channel data populated by init */
    float32             dutyCycles[NUM_OF_CHANNELS];      /* Last applied 6-channel duty array (percent) */
    float32             phases[NUM_OF_CHANNELS];          /* Reserved (unused) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];       /* Configured dead time per channel */
} GtmTom3phInv;

static GtmTom3phInv s_inv = {0};
static IfxGtm_Tom_Timer s_timeBase;                       /* TOM time base */
static boolean         s_initialized = FALSE;             /* Init status flag */

/* Stored per-phase duty percentages (U, V, W) */
static float32 s_dutyU = INITIAL_DUTY_PERCENT_U;
static float32 s_dutyV = INITIAL_DUTY_PERCENT_V;
static float32 s_dutyW = INITIAL_DUTY_PERCENT_W;

/* ===================================================================================================================
 * Local helpers
 * =================================================================================================================== */
static void GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(float32 *outDutyPercent6)
{
    /* Map each complementary pair to the same duty (U,V,W) */
    outDutyPercent6[0] = s_dutyU; /* U_HS */
    outDutyPercent6[1] = s_dutyU; /* U_LS */
    outDutyPercent6[2] = s_dutyV; /* V_HS */
    outDutyPercent6[3] = s_dutyV; /* V_LS */
    outDutyPercent6[4] = s_dutyW; /* W_HS */
    outDutyPercent6[5] = s_dutyW; /* W_LS */
}

/* ===================================================================================================================
 * Public API
 * =================================================================================================================== */

/* Initialize GTM TOM PWM for 3-phase inverter using unified IfxGtm_Pwm driver (6 channels as 3 complementary pairs) */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    s_initialized = FALSE; /* Reset init flag until success */

    /* 1) Enable GTM and set CMU clocks for TOM operation */
    IfxGtm_enable(&MODULE_GTM);

    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, IfxGtm_Cmu_getGclkFrequency(&MODULE_GTM));
        /* Enable FXCLK and CLK0 for TOM */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 2) Configure TOM-based time base (20 kHz center-aligned period source) */
    {
        IfxGtm_Tom_Timer_Config tcfg;
        IfxGtm_Tom_Timer_initConfig(&tcfg, &MODULE_GTM);
        tcfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;
        tcfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
        tcfg.tom            = IfxGtm_Tom_1;           /* Use TOM1 cluster */
        tcfg.timerChannel   = IfxGtm_Tom_Ch_0;        /* Master time base channel */

        if (IfxGtm_Tom_Timer_init(&s_timeBase, &tcfg) == FALSE)
        {
            /* Init failed: do not proceed */
            return;
        }

        /* Assign all used channels (1..6) to the timer's update mask for atomic updates */
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_1);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_2);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_3);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_4);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_5);
        IfxGtm_Tom_Timer_addToChannelMask(&s_timeBase, IfxGtm_Tom_Ch_6);
        IfxGtm_Tom_Timer_applyUpdate(&s_timeBase);
    }

    /* 3) Mux six PWM pins to TOM function using generic PinMap API (output push-pull, automotive speed1) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Configure unified multi-channel PWM driver (6 channels) */
    {
        IfxGtm_Pwm_Config          cfg;
        IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_OutputConfig    outCfg[NUM_OF_CHANNELS];
        IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

        IfxGtm_Pwm_initConfig(&cfg, &MODULE_GTM);

        /* Global PWM settings */
        cfg.cluster            = IfxGtm_Cluster_0;
        cfg.subModule          = IfxGtm_Pwm_SubModule_tom;
        cfg.alignment          = IfxGtm_Pwm_Alignment_center;     /* Center-aligned */
        cfg.syncStart          = FALSE;                            /* Start explicitly via API */
        cfg.syncUpdateEnabled  = TRUE;                             /* Atomic shadow updates */
        cfg.numChannels        = NUM_OF_CHANNELS;                  /* 6 physical outputs */
        cfg.frequency          = TIMING_PWM_FREQUENCY_HZ;
        cfg.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;               /* TOM uses Fxclk enum */
        cfg.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock */

        /* Output configuration per channel (no separate pin functions) */
        /* Channel order: U_HS, U_LS, V_HS, V_LS, W_HS, W_LS */
        outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
        outCfg[0].complementaryPin      = NULL_PTR;
        outCfg[0].polarity              = Ifx_ActiveState_high;
        outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
        outCfg[1].complementaryPin      = NULL_PTR;
        outCfg[1].polarity              = Ifx_ActiveState_low;     /* Low-side active low */
        outCfg[1].complementaryPolarity = Ifx_ActiveState_high;
        outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
        outCfg[2].complementaryPin      = NULL_PTR;
        outCfg[2].polarity              = Ifx_ActiveState_high;
        outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[3].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
        outCfg[3].complementaryPin      = NULL_PTR;
        outCfg[3].polarity              = Ifx_ActiveState_low;
        outCfg[3].complementaryPolarity = Ifx_ActiveState_high;
        outCfg[3].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[3].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[4].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
        outCfg[4].complementaryPin      = NULL_PTR;
        outCfg[4].polarity              = Ifx_ActiveState_high;
        outCfg[4].complementaryPolarity = Ifx_ActiveState_low;
        outCfg[4].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[4].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        outCfg[5].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
        outCfg[5].complementaryPin      = NULL_PTR;
        outCfg[5].polarity              = Ifx_ActiveState_low;
        outCfg[5].complementaryPolarity = Ifx_ActiveState_high;
        outCfg[5].outputMode            = IfxPort_OutputMode_pushPull;
        outCfg[5].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* DTM dead-time per channel (example: 1us rise/fall) */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            dtmCfg[i].deadTime.rising  = 1.0e-6f;
            dtmCfg[i].deadTime.falling = 1.0e-6f;
        }

        /* Channel configuration */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
            /* Map channel indices to TOM1 Ch1..Ch6 for outputs 0..5 */
            switch (i)
            {
                case 0u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_2; break; /* U_HS -> TOM1 Ch2 */
                case 1u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_1; break; /* U_LS -> TOM1 Ch1 */
                case 2u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_4; break; /* V_HS -> TOM1 Ch4 */
                case 3u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_3; break; /* V_LS -> TOM1 Ch3 */
                case 4u: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_6; break; /* W_HS -> TOM1 Ch6 */
                default: chCfg[i].timerCh = IfxGtm_Pwm_SubModule_Ch_5; break; /* W_LS -> TOM1 Ch5 */
            }
            chCfg[i].phase     = 0.0f;
            chCfg[i].duty      = 0.0f;                   /* Initial duty will be applied via update API */
            chCfg[i].dtm       = &dtmCfg[i];
            chCfg[i].output    = &outCfg[i];
            chCfg[i].mscOut    = NULL_PTR;
            chCfg[i].interrupt = NULL_PTR;               /* No ISR used in this module */
        }

        cfg.channels = &chCfg[0]; /* Link array */

        /* Initialize the PWM driver */
        IfxGtm_Pwm_init(&s_inv.pwm, &s_inv.channels[0], &cfg);
    }

    /* 5) Build and apply initial six-channel duty array (pairs equal) */
    {
        float32 duty6[NUM_OF_CHANNELS];
        GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(&duty6[0]);
        /* Store last applied duties */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            s_inv.dutyCycles[i] = duty6[i];
        }
        /* Apply atomically; hardware updates at the next period boundary */
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_inv.pwm, &duty6[0]);
    }

    /* 6) Start synchronized PWM outputs explicitly */
    IfxGtm_Pwm_startSyncedChannels(&s_inv.pwm);

    s_initialized = TRUE; /* Initialization successful */
}

/* Step duty: add +10% to U/V/W and wrap to 0% if exceeding 100%; apply atomically to 6 outputs */
void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        /* Early exit on uninitialized driver (per error-handling requirements) */
        return;
    }

    /* Increment and wrap per-phase duties */
    s_dutyU += DUTY_STEP_PERCENT; if (s_dutyU > 100.0f) { s_dutyU = 0.0f; }
    s_dutyV += DUTY_STEP_PERCENT; if (s_dutyV > 100.0f) { s_dutyV = 0.0f; }
    s_dutyW += DUTY_STEP_PERCENT; if (s_dutyW > 100.0f) { s_dutyW = 0.0f; }

    /* Build six-channel array (pairs equal) and apply immediate update */
    {
        float32 duty6[NUM_OF_CHANNELS];
        GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(&duty6[0]);
        /* Cache */
        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            s_inv.dutyCycles[i] = duty6[i];
        }
        IfxGtm_Pwm_updateChannelsDutyImmediate(&s_inv.pwm, &duty6[0]);
    }
}
