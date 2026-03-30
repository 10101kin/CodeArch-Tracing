/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * GTM TOM 3-Phase complementary PWM driver for TC3xx.
 * - TOM1, Cluster_1
 * - Center-aligned 20 kHz, synchronized updates, 1 us dead-time
 * - Six outputs on P02.{0,7} (U), P02.{1,4} (V), P02.{2,5} (W)
 * - ISR toggles P13.0 (priority 20) as diagnostic
 *
 * Initialization follows iLLD patterns and uses only validated APIs.
 */
#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* Numeric configuration constants (from requirements) */
#define NUM_OF_CHANNELS                 (3)
#define PWM_BASE_FREQUENCY_HZ           (20000.0f)      /* 20 kHz */
#define PWM_DEADTIME_S                  (1.0e-6f)       /* 1 us */
#define PHASE_U_DUTY_INIT_PERCENT       (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT       (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT       (75.0f)
#define DUTY_STEP_PERCENT               (10.0f)

/* ISR priority macro */
#define ISR_PRIORITY_ATOM               (20)

/* LED diagnostic pin (compound macro: port, pin) */
#define LED                             &MODULE_P13, 0

/* Validated TOUT pin symbols (user-mandated mapping) */
#define PHASE_U_HS                      (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                      (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                      (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                      (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                      (&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)
#define PHASE_W_LS                      (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* Internal module state */
typedef struct
{
    IfxGtm_Tom_Timer     timer;                     /* TOM timer driver handle */
    IfxGtm_Tom_PwmHl     pwmhl;                     /* PWM HL driver handle   */

    const IfxGtm_Tom_ToutMap *hs[NUM_OF_CHANNELS];  /* High-side pins (persistent) */
    const IfxGtm_Tom_ToutMap *ls[NUM_OF_CHANNELS];  /* Low-side pins  (persistent) */

    float32              dutyCycles[NUM_OF_CHANNELS];
    float32              deadtime_s;
} GtmTom3phInv_State;

/* IFX_STATIC per architectural rule */
IFX_STATIC GtmTom3phInv_State g_tom3ph = { 0 };

/* ISR: minimal body — toggle diagnostic LED (CPU0, priority = ISR_PRIORITY_ATOM) */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (empty as specified) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* Helper: configure all TOM TOUT pads (push-pull, automotive pad driver) */
static void gtmTom_configureToutPads(void)
{
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/*
 * initGtmTom3phInv
 * Initialize GTM TOM 3-phase complementary PWM (center-aligned, sync update, dead-time).
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare local config structures */
    IfxGtm_Tom_Timer_Config   timerCfg;
    IfxGtm_Tom_PwmHl_Config   pwmhlCfg;

    /* Store persistent pin descriptors in module state (one complementary pair per phase) */
    g_tom3ph.hs[0] = PHASE_U_HS; g_tom3ph.ls[0] = PHASE_U_LS; /* Phase U */
    g_tom3ph.hs[1] = PHASE_V_HS; g_tom3ph.ls[1] = PHASE_V_LS; /* Phase V */
    g_tom3ph.hs[2] = PHASE_W_HS; g_tom3ph.ls[2] = PHASE_W_LS; /* Phase W */

    /* 2) Initialize the TOM timer configuration with defaults, set 20 kHz and FXCLK0 */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    /* Timer selection: TOM1, FXCLK0 as clock source, center-aligned through PWMHL mode */
    timerCfg.tom                      = IfxGtm_Tom_1;
    timerCfg.clock                    = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0; /* FXCLK0 */
    timerCfg.base.frequency           = PWM_BASE_FREQUENCY_HZ;          /* 20 kHz */
    timerCfg.base.trigger.enabled     = FALSE;                          /* no external trigger */
    timerCfg.base.countDir            = IfxStdIf_Timer_CountDir_upDown; /* for center alignment */
    timerCfg.base.isrPriority         = 0u;                              /* no timer ISR here */

    /* 3) Initialize PWMHL configuration: 3 channels, push-pull, automotive pad, active states */
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);
    pwmhlCfg.timer                    = &g_tom3ph.timer;
    pwmhlCfg.base.channelCount        = NUM_OF_CHANNELS;
    pwmhlCfg.base.outputMode          = IfxPort_OutputMode_pushPull;
    pwmhlCfg.base.padDriver           = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    pwmhlCfg.ccxActiveState           = Ifx_ActiveState_high;            /* high-side active HIGH */
    pwmhlCfg.coutxActiveState         = Ifx_ActiveState_low;             /* low-side  active LOW  */

    /* Assign complementary channel pin pairs */
    pwmhlCfg.ch[0].out                = (IfxGtm_Tom_ToutMap *)g_tom3ph.hs[0];
    pwmhlCfg.ch[0].outNeg             = (IfxGtm_Tom_ToutMap *)g_tom3ph.ls[0];
    pwmhlCfg.ch[1].out                = (IfxGtm_Tom_ToutMap *)g_tom3ph.hs[1];
    pwmhlCfg.ch[1].outNeg             = (IfxGtm_Tom_ToutMap *)g_tom3ph.ls[1];
    pwmhlCfg.ch[2].out                = (IfxGtm_Tom_ToutMap *)g_tom3ph.hs[2];
    pwmhlCfg.ch[2].outNeg             = (IfxGtm_Tom_ToutMap *)g_tom3ph.ls[2];

    /* 4) Configure pin routing for each assigned TOM output */
    gtmTom_configureToutPads();

    /* 5) GTM enable guard: enable GTM and clocks if not already enabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        float32 freq = 0.0f;
        IfxGtm_enable(&MODULE_GTM);
        freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the TOM timer */
    if (IfxGtm_Tom_Timer_init(&g_tom3ph.timer, &timerCfg) == FALSE)
    {
        /* Failed to init timer: leave without proceeding */
        return;
    }

    /* 7) Initialize the PWMHL driver and set center-aligned mode */
    if (IfxGtm_Tom_PwmHl_init(&g_tom3ph.pwmhl, &pwmhlCfg) == FALSE)
    {
        /* Failed to init PWMHL */
        return;
    }

    (void)IfxGtm_Tom_PwmHl_setMode(&g_tom3ph.pwmhl, Ifx_Pwm_Mode_centerAligned);

    /* Programmed dead-time */
    g_tom3ph.deadtime_s = PWM_DEADTIME_S;
    (void)IfxGtm_Tom_PwmHl_setDeadtime(&g_tom3ph.pwmhl, g_tom3ph.deadtime_s);

    /* 8) Compute initial ON-times from timer period and initial duties */
    g_tom3ph.dutyCycles[0] = PHASE_U_DUTY_INIT_PERCENT;
    g_tom3ph.dutyCycles[1] = PHASE_V_DUTY_INIT_PERCENT;
    g_tom3ph.dutyCycles[2] = PHASE_W_DUTY_INIT_PERCENT;

    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_tom3ph.timer);
        Ifx_TimerValue tOn[NUM_OF_CHANNELS];
        tOn[0] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[0]) / 100.0f);
        tOn[1] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[1]) / 100.0f);
        tOn[2] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[2]) / 100.0f);

        /* 9) Coherent update */
        IfxGtm_Tom_Timer_disableUpdate(&g_tom3ph.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_tom3ph.pwmhl, tOn);
        IfxGtm_Tom_Timer_applyUpdate(&g_tom3ph.timer);
    }

    /* 11) Configure diagnostic/toggle GPIO pin as push-pull output (initial LOW) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * updateGtmTom3phInvDuty
 * Update three-phase duties using step and wrap, apply coherently, toggle diagnostic GPIO.
 */
void updateGtmTom3phInvDuty(void)
{
    /* 1) Apply wrap rule (separate ifs, unconditional add) */
    if ((g_tom3ph.dutyCycles[0] + DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[0] = 0.0f; }
    if ((g_tom3ph.dutyCycles[1] + DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[1] = 0.0f; }
    if ((g_tom3ph.dutyCycles[2] + DUTY_STEP_PERCENT) >= 100.0f) { g_tom3ph.dutyCycles[2] = 0.0f; }
    g_tom3ph.dutyCycles[0] += DUTY_STEP_PERCENT;
    g_tom3ph.dutyCycles[1] += DUTY_STEP_PERCENT;
    g_tom3ph.dutyCycles[2] += DUTY_STEP_PERCENT;

    /* 2) Convert duties to ON-times using current period */
    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_tom3ph.timer);
        Ifx_TimerValue tOn[NUM_OF_CHANNELS];
        tOn[0] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[0]) / 100.0f);
        tOn[1] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[1]) / 100.0f);
        tOn[2] = (Ifx_TimerValue)((period * g_tom3ph.dutyCycles[2]) / 100.0f);

        /* 3) Coherent update */
        IfxGtm_Tom_Timer_disableUpdate(&g_tom3ph.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_tom3ph.pwmhl, tOn);
        IfxGtm_Tom_Timer_applyUpdate(&g_tom3ph.timer);
    }

    /* 4) Toggle diagnostic GPIO */
    IfxPort_togglePin(LED);

    /* 5) Duties already stored in module state (g_tom3ph.dutyCycles[]) */
}
