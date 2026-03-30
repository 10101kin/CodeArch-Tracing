/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver implementing a 3-phase complementary PWM using GTM TOM1 Cluster_1
 * - Center-aligned mode, synchronized updates
 * - Dead-time insertion (1 us)
 * - Six outputs mapped to P02.{0,7,1,4,2,5}
 * - Duty in percent: U=25, V=50, W=75; update step=10 with wrap rule
 * - GTM enable guard with CMU clock setup (GCLK and FXCLK0)
 * - Diagnostic ISR toggles P13.0; empty period callback provided
 *
 * Note: Watchdog disable MUST NOT be here (only in CpuX_Main.c per AURIX standard)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"

/* ============================
 * Configuration macros
 * ============================ */
#define PWM_NUM_CHANNELS               (3u)
#define PWM_FREQUENCY_HZ               (20000.0f)          /* 20 kHz */
#define PWM_DEADTIME_SEC               (1.0e-6f)           /* 1 us */
#define PWM_PERIOD_TICKS_CENTER_ALG    (2500u)             /* informational (center-aligned) */

#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)
#define PHASE_DUTY_STEP                (10.0f)

/* ISR priority and LED pin */
#define ISR_PRIORITY_ATOM              (20u)
#define LED                            &MODULE_P13, 0

/* Validated TOM1 pin routing for P02.x (High/Low per phase) */
#define PHASE_U_HS                     (&IfxGtm_TOM1_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                     (&IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS                     (&IfxGtm_TOM1_1_TOUT1_P02_1_OUT)
#define PHASE_V_LS                     (&IfxGtm_TOM1_12_TOUT4_P02_4_OUT)
#define PHASE_W_HS                     (&IfxGtm_TOM1_10_TOUT2_P02_2_OUT)
#define PHASE_W_LS                     (&IfxGtm_TOM1_13_TOUT5_P02_5_OUT)

/* ============================
 * Module state
 * ============================ */
typedef struct
{
    IfxGtm_Tom_Timer   timer;               /* TOM time-base */
    IfxGtm_Tom_PwmHl   pwm;                 /* Complementary PWM driver */
    float32            dutyPercent[PWM_NUM_CHANNELS]; /* U, V, W in percent */
    float32            deadtimeSec;         /* dead-time in seconds */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gti;

/* ============================
 * ISR and callback (declared before init)
 * ============================ */
IFX_INTERRUPT(interruptGtm_TomAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtm_TomAtom(void)
{
    IfxPort_togglePin(LED);
}

/* Period event hook (empty by design) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================
 * Local helpers
 * ============================ */
static void gtmTom_routePins(void)
{
    /* Configure TOM TOUT routing for each assigned pin, push-pull + automotive pad driver */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}

/* ============================
 * Public functions
 * ============================ */

/*
 * Initialize a 3-phase complementary PWM using GTM TOM with center-aligned mode.
 * Algorithm per SW detailed design:
 *  - Local config structs and pin arrays
 *  - Timer config: 20 kHz, FXCLK0
 *  - PWMHL config: 3 channels, push-pull, active states, assign pins
 *  - Pin routing
 *  - GTM enable guard: enable module, set GCLK/CLK0, enable FXCLK/CLK0
 *  - Initialize timer and PWMHL, set mode center-aligned, set dead-time
 *  - Compute initial ON-times from period and initial duties (25/50/75)
 *  - Coherent update: disableUpdate -> setOnTime -> applyUpdate
 *  - Store persistent state
 *  - Configure diagnostic LED P13.0 as push-pull output
 */
void initGtmTom3phInv(void)
{
    /* 1) Local configuration objects and pin descriptor arrays */
    IfxGtm_Tom_Timer_Config  timerCfg;
    IfxGtm_Tom_PwmHl_Config  pwmhlCfg;

    const IfxGtm_Tom_ToutMap *highPins[PWM_NUM_CHANNELS] = { PHASE_U_HS, PHASE_V_HS, PHASE_W_HS };
    const IfxGtm_Tom_ToutMap *lowPins[PWM_NUM_CHANNELS]  = { PHASE_U_LS, PHASE_V_LS, PHASE_W_LS };

    /* 2) Initialize TOM timer configuration: 20 kHz, select FXCLK0 as clock source */
    IfxGtm_Tom_Timer_initConfig(&timerCfg, &MODULE_GTM);
    /* Set desired base frequency; clock source selection: FXCLK0 for TOM */
    /* Note: Field names depend on iLLD version; frequency and clock assignments are typical. */
    timerCfg.base.frequency = PWM_FREQUENCY_HZ;                 /* desired PWM base frequency */
    timerCfg.clock          = IfxGtm_Tom_Ch_ClkSrc_cmuFxclk0;   /* use FXCLK0 for TOM */

    /* 3) Initialize PWMHL configuration: 3 channels, push-pull, active states, assign pins */
    IfxGtm_Tom_PwmHl_initConfig(&pwmhlCfg);
    pwmhlCfg.timer                  = &g_gti.timer;
    pwmhlCfg.base.channelCount      = PWM_NUM_CHANNELS;
    pwmhlCfg.base.outputMode        = IfxPort_OutputMode_pushPull;
    pwmhlCfg.base.outputDriver      = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    /* Complementary polarity convention: HS active HIGH, LS active LOW */
    pwmhlCfg.base.polarity.top      = Ifx_ActiveState_high;  /* high-side */
    pwmhlCfg.base.polarity.bottom   = Ifx_ActiveState_low;   /* low-side  */
    pwmhlCfg.pins.top               = (IfxGtm_Tom_ToutMap const * const *)highPins;
    pwmhlCfg.pins.bottom            = (IfxGtm_Tom_ToutMap const * const *)lowPins;

    /* 4) Pin routing for each assigned TOUT */
    gtmTom_routePins();

    /* 5) GTM enable guard and CMU clock configuration (inside the guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        /* Read module frequency dynamically and configure clocks for TOM */
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);  /* CMU CLK0 */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize the TOM timer */
    {
        boolean ok = IfxGtm_Tom_Timer_init(&g_gti.timer, &timerCfg);
        if (ok == FALSE)
        {
            /* Initialization failed: leave safely */
            return;
        }
    }

    /* 7) Initialize PWMHL and set mode to center-aligned */
    {
        boolean ok = IfxGtm_Tom_PwmHl_init(&g_gti.pwm, &pwmhlCfg);
        if (ok == FALSE)
        {
            return;
        }
        ok = IfxGtm_Tom_PwmHl_setMode(&g_gti.pwm, Ifx_Pwm_Mode_centerAligned);
        if (ok == FALSE)
        {
            return;
        }
    }

    /* Programmed dead-time */
    {
        boolean ok = IfxGtm_Tom_PwmHl_setDeadtime(&g_gti.pwm, PWM_DEADTIME_SEC);
        if (ok == FALSE)
        {
            return;
        }
        g_gti.deadtimeSec = PWM_DEADTIME_SEC;
    }

    /* 8) Compute initial ON-times from period and initial duties */
    g_gti.dutyPercent[0] = PHASE_U_DUTY_INIT; /* U */
    g_gti.dutyPercent[1] = PHASE_V_DUTY_INIT; /* V */
    g_gti.dutyPercent[2] = PHASE_W_DUTY_INIT; /* W */

    {
        Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gti.timer);
        Ifx_TimerValue tOn[PWM_NUM_CHANNELS];
        tOn[0] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[0] / 100.0f));
        tOn[1] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[1] / 100.0f));
        tOn[2] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[2] / 100.0f));

        /* 9) Coherent update */
        IfxGtm_Tom_Timer_disableUpdate(&g_gti.timer);
        IfxGtm_Tom_PwmHl_setOnTime(&g_gti.pwm, &tOn[0]);
        IfxGtm_Tom_Timer_applyUpdate(&g_gti.timer);
    }

    /* 11) Diagnostic GPIO (LED) as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Update three-phase duties with fixed step and wrap rule; apply coherently.
 * - Wrap rule: if (duty + step) >= 100 then reset to 0, then add step (always add)
 * - Compute new ON-times from current timer period and apply as coherent update
 * - Toggle diagnostic GPIO (P13.0)
 */
void updateGtmTom3phInvDuty(void)
{
    /* 1) Apply wrap rule to duties (no loop: explicit per-channel updates) */
    if ((g_gti.dutyPercent[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyPercent[0] = 0.0f; }
    if ((g_gti.dutyPercent[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyPercent[1] = 0.0f; }
    if ((g_gti.dutyPercent[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gti.dutyPercent[2] = 0.0f; }
    g_gti.dutyPercent[0] += PHASE_DUTY_STEP;
    g_gti.dutyPercent[1] += PHASE_DUTY_STEP;
    g_gti.dutyPercent[2] += PHASE_DUTY_STEP;

    /* 2) Convert each duty to ON-time using current period */
    Ifx_TimerValue period = IfxGtm_Tom_Timer_getPeriod(&g_gti.timer);
    Ifx_TimerValue tOn[PWM_NUM_CHANNELS];
    tOn[0] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[0] / 100.0f));
    tOn[1] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[1] / 100.0f));
    tOn[2] = (Ifx_TimerValue)((float32)period * (g_gti.dutyPercent[2] / 100.0f));

    /* 3) Coherent update */
    IfxGtm_Tom_Timer_disableUpdate(&g_gti.timer);
    IfxGtm_Tom_PwmHl_setOnTime(&g_gti.pwm, &tOn[0]);
    IfxGtm_Tom_Timer_applyUpdate(&g_gti.timer);

    /* 4) Toggle diagnostic GPIO */
    IfxPort_togglePin(LED);
}
