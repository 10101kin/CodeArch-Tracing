/**********************************************************************************************************************
 * 
 *  File: gtm_tom_3_phase_inverter_pwm.c
 *  Brief: GTM TOM 3-Phase Inverter PWM driver implementation (TC3xx, IfxGtm_Pwm high-level driver)
 *
 **********************************************************************************************************************/
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ============================== Internal Driver State ============================== */
typedef struct
{
    IfxGtm_Tom_Timer      timer;                     /* TOM time base */
    IfxGtm_Pwm            pwm;                       /* Unified PWM driver */
    IfxGtm_Pwm_Channel    channels[NUM_OF_CHANNELS]; /* Channel state returned by init */
    float32               dutyU;                    /* Stored U phase duty (percent) */
    float32               dutyV;                    /* Stored V phase duty (percent) */
    float32               dutyW;                    /* Stored W phase duty (percent) */
    float32               duty6[NUM_OF_CHANNELS];   /* Six-channel duty buffer */
} GTM_TOM_3PhInv_Driver;

static GTM_TOM_3PhInv_Driver s_drv;
static boolean               s_initialized = FALSE;

/* ============================== Private helpers ============================== */
static void GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(float32 *outDutyPercent6)
{
    /* Mapping: [U_LS, U_HS, V_LS, V_HS, W_LS, W_HS] with same duty per pair */
    outDutyPercent6[0] = s_drv.dutyU; /* U_LS (TOM1 Ch1) */
    outDutyPercent6[1] = s_drv.dutyU; /* U_HS (TOM1 Ch2) */
    outDutyPercent6[2] = s_drv.dutyV; /* V_LS (TOM1 Ch3) */
    outDutyPercent6[3] = s_drv.dutyV; /* V_HS (TOM1 Ch4) */
    outDutyPercent6[4] = s_drv.dutyW; /* W_LS (TOM1 Ch5) */
    outDutyPercent6[5] = s_drv.dutyW; /* W_HS (TOM1 Ch6) */
}

/* ============================== Public API implementation ============================== */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    IfxGtm_Pwm_Config           pwmCfg;
    IfxGtm_Pwm_ChannelConfig    chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     outCfg[NUM_OF_CHANNELS];
    IfxGtm_Tom_Timer_Config     timerCfg;

    /* 1) Enable GTM and CMU clocks required for TOM operation */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)IFXGTM_CMU_CLKEN_FXCLK);

    /* 2) Configure TOM-based time base at 20 kHz, center-aligned period source for cluster */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    timerCfg.base.frequency = (float32)TIMING_PWM_FREQUENCY_HZ;
    timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;
    timerCfg.tom            = IfxGtm_Tom_1;              /* TOM1 is used for channels 1..6 */
    timerCfg.timerChannel   = IfxGtm_Tom_Ch_0;           /* Use Ch0 as time base (free from outputs) */

    if (IfxGtm_Tom_Timer_init(&s_drv.timer, &timerCfg) == FALSE)
    {
        /* Early exit on failure; do not mark initialized */
        return;
    }

    /* Make TOM1 channels 1..6 part of the timer's update mask (synchronous shadow transfer) */
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_1);
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_2);
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_3);
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_4);
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_5);
    IfxGtm_Tom_Timer_addToChannelMask(&s_drv.timer, IfxGtm_Tom_Ch_6);
    IfxGtm_Tom_Timer_applyUpdate(&s_drv.timer);

    /* 3) Mux six PWM output pins to TOM function (generic PinMap API) */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Initialize unified multi-channel PWM driver (6 channels, TOM submodule, center-aligned, sync updates) */
    IfxGtm_Pwm_initConfig(&pwmCfg, &MODULE_GTM);

    /* Output configuration per channel (no complementary pair object; 6 independent outputs mapped to TOM1 ch1..6) */
    outCfg[0].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;  outCfg[0].outputMode = IfxPort_OutputMode_pushPull; outCfg[0].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    outCfg[1].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;  outCfg[1].outputMode = IfxPort_OutputMode_pushPull; outCfg[1].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    outCfg[2].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;  outCfg[2].outputMode = IfxPort_OutputMode_pushPull; outCfg[2].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    outCfg[3].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;  outCfg[3].outputMode = IfxPort_OutputMode_pushPull; outCfg[3].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    outCfg[4].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;  outCfg[4].outputMode = IfxPort_OutputMode_pushPull; outCfg[4].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    outCfg[5].pin        = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;  outCfg[5].outputMode = IfxPort_OutputMode_pushPull; outCfg[5].padDriver = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel configurations (explicit per-channel) */
    for (uint8 i = 0U; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
        chCfg[i].output    = &outCfg[i];
        chCfg[i].phase     = 0.0f;
        chCfg[i].mscOut    = NULL_PTR;
        chCfg[i].interrupt = NULL_PTR; /* No ISR in this module */
    }

    /* Map timer channel indices 1..6 for TOM1; assign initial duties per pair (U,U,V,V,W,W) */
    chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_1; chCfg[0].duty = INITIAL_DUTY_PERCENT_U;
    chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_2; chCfg[1].duty = INITIAL_DUTY_PERCENT_U;
    chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_3; chCfg[2].duty = INITIAL_DUTY_PERCENT_V;
    chCfg[3].timerCh = IfxGtm_Pwm_SubModule_Ch_4; chCfg[3].duty = INITIAL_DUTY_PERCENT_V;
    chCfg[4].timerCh = IfxGtm_Pwm_SubModule_Ch_5; chCfg[4].duty = INITIAL_DUTY_PERCENT_W;
    chCfg[5].timerCh = IfxGtm_Pwm_SubModule_Ch_6; chCfg[5].duty = INITIAL_DUTY_PERCENT_W;

    pwmCfg.cluster            = IfxGtm_Cluster_0;
    pwmCfg.subModule          = IfxGtm_Pwm_SubModule_tom;
    pwmCfg.alignment          = IfxGtm_Pwm_Alignment_center;  /* Center-aligned */
    pwmCfg.syncStart          = TRUE;                          /* Synchronized start */
    pwmCfg.syncUpdateEnabled  = TRUE;                          /* Atomic shadow updates */
    pwmCfg.numChannels        = NUM_OF_CHANNELS;
    pwmCfg.channels           = &chCfg[0];
    pwmCfg.frequency          = (float32)TIMING_PWM_FREQUENCY_HZ;
    pwmCfg.clockSource.tom    = IfxGtm_Cmu_Fxclk_0;            /* TOM uses Fxclk */

    IfxGtm_Pwm_init(&s_drv.pwm, &s_drv.channels[0], &pwmCfg);

    /* 5) Initialize stored duties and apply initial six-channel array atomically, then start synced outputs */
    s_drv.dutyU = INITIAL_DUTY_PERCENT_U;
    s_drv.dutyV = INITIAL_DUTY_PERCENT_V;
    s_drv.dutyW = INITIAL_DUTY_PERCENT_W;

    GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(s_drv.duty6);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, s_drv.duty6);
    IfxGtm_Pwm_startSyncedChannels(&s_drv.pwm);

    s_initialized = TRUE;
}

void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if init failed or not executed yet */
    }

    /* Step phase duties by +10%; wrap to 0% if any exceeds 100% */
    s_drv.dutyU += DUTY_STEP_PERCENT; if (s_drv.dutyU > 100.0f) { s_drv.dutyU = 0.0f; }
    s_drv.dutyV += DUTY_STEP_PERCENT; if (s_drv.dutyV > 100.0f) { s_drv.dutyV = 0.0f; }
    s_drv.dutyW += DUTY_STEP_PERCENT; if (s_drv.dutyW > 100.0f) { s_drv.dutyW = 0.0f; }

    GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(s_drv.duty6);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_drv.pwm, s_drv.duty6);
}
