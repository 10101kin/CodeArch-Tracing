#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"

/* ========================================================================== */
/* Internal state                                                              */
/* ========================================================================== */
static IfxGtm_Pwm          s_pwm;
static IfxGtm_Pwm_Channel  s_channels[NUM_PWM_CHANNELS];
static IfxGtm_Tom_Timer    s_timer;

static boolean             s_initialized = FALSE;

/* Stored phase duties (percent) */
static float32             s_phaseDutyU = INITIAL_DUTY_PERCENT_U;
static float32             s_phaseDutyV = INITIAL_DUTY_PERCENT_V;
static float32             s_phaseDutyW = INITIAL_DUTY_PERCENT_W;

/* Scratch array for 6-output duty updates */
static float32             s_dutyArray6[NUM_PWM_CHANNELS];

/* ========================================================================== */
/* Private helpers                                                             */
/* ========================================================================== */
/**
 * Build a 6-element duty array mapping each phase's duty to its complementary
 * pair outputs: [U_HS, U_LS, V_HS, V_LS, W_HS, W_LS].
 */
static void GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(float32 *outDutyPercent6)
{
    outDutyPercent6[0] = s_phaseDutyU; /* U_HS */
    outDutyPercent6[1] = s_phaseDutyU; /* U_LS */
    outDutyPercent6[2] = s_phaseDutyV; /* V_HS */
    outDutyPercent6[3] = s_phaseDutyV; /* V_LS */
    outDutyPercent6[4] = s_phaseDutyW; /* W_HS */
    outDutyPercent6[5] = s_phaseDutyW; /* W_LS */
}

/* ========================================================================== */
/* Public functions                                                            */
/* ========================================================================== */
void GTM_TOM_3_Phase_Inverter_PWM_init(void)
{
    /* Enable GTM and FXCLK for TOM operation */
    IfxGtm_enable(&MODULE_GTM);
    IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);

    /* Configure TOM-based time base for 20 kHz center-aligned PWM */
    IfxGtm_Tom_Timer_Config timerCfg;
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    timerCfg.base.frequency = TIMING_PWM_FREQUENCY_HZ;           /* 20 kHz */
    timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;    /* Use FXCLK0 */
    timerCfg.tom            = GTM_TOM_MASTER;                    /* TOM1 */
    timerCfg.timerChannel   = GTM_TOM_MASTER_TIMER_CH;           /* CH0 as time base */

    if (IfxGtm_Tom_Timer_init(&s_timer, &timerCfg) == FALSE)
    {
        /* Hardware init failed -> do not proceed */
        return;
    }

    /* Add the six PWM channels (TOM1 CH1..CH6) to the timer's channel mask */
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_1);
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_2);
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_3);
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_4);
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_5);
    IfxGtm_Tom_Timer_addToChannelMask(&s_timer, IfxGtm_Tom_Ch_6);

    /* Apply initial timer configuration */
    IfxGtm_Tom_Timer_applyUpdate(&s_timer);

    /* Mux all six TOM outputs to pins using generic PinMap API */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS_PIN, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* Configure unified multi-channel PWM driver */
    IfxGtm_Pwm_Config          pwmCfg;
    IfxGtm_Pwm_ChannelConfig   chCfg[NUM_PWM_CHANNELS];

    IfxGtm_Pwm_initConfig(&pwmCfg, &MODULE_GTM);

    /* Initialize per-channel config defaults (6 outputs) */
    for (uint8 i = 0U; i < NUM_PWM_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
    }

    /* High-level PWM setup: TOM submodule, center-aligned, synced updates */
    pwmCfg.cluster               = IfxGtm_Cluster_0;
    pwmCfg.subModule             = IfxGtm_Pwm_SubModule_tom;
    pwmCfg.alignment             = IfxGtm_Pwm_Alignment_center;
    pwmCfg.syncStart             = TRUE;            /* Synchronous start */
    pwmCfg.syncUpdateEnabled     = TRUE;            /* Shadow-to-compare sync updates */
    pwmCfg.numChannels           = NUM_PWM_CHANNELS;/* Six TOM outputs */
    pwmCfg.channels              = chCfg;           /* Link channel configs */
    pwmCfg.frequency             = TIMING_PWM_FREQUENCY_HZ;
    pwmCfg.clockSource.tom       = IfxGtm_Cmu_Fxclk_0; /* TOM uses Fxclk enum */

    /* Initialize PWM driver (channels array provided separately) */
    IfxGtm_Pwm_init(&s_pwm, &s_channels[0], &pwmCfg);

    /* Build and apply initial 6-output duties atomically */
    GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(s_dutyArray6);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, s_dutyArray6);

    /* Start synchronized PWM outputs (explicit per SW design) */
    IfxGtm_Pwm_startSyncedChannels(&s_pwm);

    s_initialized = TRUE;
}

void GTM_TOM_3_Phase_Inverter_PWM_stepDuty(void)
{
    if (s_initialized == FALSE)
    {
        /* Early exit on uninitialized state (safety + unit-test expectation) */
        return;
    }

    /* Increment and wrap duties per behavior description */
    s_phaseDutyU += DUTY_STEP_PERCENT;
    s_phaseDutyV += DUTY_STEP_PERCENT;
    s_phaseDutyW += DUTY_STEP_PERCENT;

    if (s_phaseDutyU > 100.0f) { s_phaseDutyU = 0.0f; }
    if (s_phaseDutyV > 100.0f) { s_phaseDutyV = 0.0f; }
    if (s_phaseDutyW > 100.0f) { s_phaseDutyW = 0.0f; }

    /* Map to six outputs (complementary pairs use same duty) and apply */
    GTM_TOM_3_Phase_Inverter_PWM_buildDutyArray(s_dutyArray6);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&s_pwm, s_dutyArray6);
}
