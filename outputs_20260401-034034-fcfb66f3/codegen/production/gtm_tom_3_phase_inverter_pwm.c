/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver for GTM TOM 3-phase complementary PWM using IfxGtm_Pwm
 * Target: KIT_A2G_TC387_5V_TFT (TC3xx)
 * Notes:
 *  - Follows unified IfxGtm_Pwm initialization with synchronized start/update
 *  - Uses TOM submodule on Cluster_1, center-aligned at 20 kHz
 *  - Dead-time configured via DTM (rising/falling) using migration overrides
 *  - Update commit controlled via IfxGtm_Tom_Timer_disableUpdate/applyUpdate
 *  - No watchdog modifications here (must be in CpuX_Main.c only)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* Numeric configuration (migration-confirmed values) */
#define GTM_TOM_NUM_CHANNELS         (3u)
#define GTM_TOM_PWM_FREQ_HZ          (20000.0f)
#define PHASE_U_DUTY_INIT            (25.0f)
#define PHASE_V_DUTY_INIT            (50.0f)
#define PHASE_W_DUTY_INIT            (75.0f)
#define PHASE_DUTY_STEP              (10.0f)
#define PHASE_DUTY_MIN               (10.0f)
#define PHASE_DUTY_MAX               (90.0f)
#define PWM_DEADTIME_SEC             (5e-07f)   /* 0.5 us (migration override) */
#define PWM_MIN_PULSE_SEC            (1e-06f)

/* Interrupt priority for GTM periodic indication (CPU0) */
#define ISR_PRIORITY_ATOM            (20)

/* LED pin used by ISR to indicate activity (compound macro: port, pin) */
#define LED                          &MODULE_P13, 0

/*
 * PWM output pin assignment macros
 * No validated TOM1/Cluster_1 TOUT symbols were provided for the requested pins.
 * Keep placeholders as NULL_PTR; replace with actual IfxGtm_TOM1_x_TOUTy_P02_z_OUT symbols during board integration.
 */
#define PHASE_U_HS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTxx_P02_0_OUT */
#define PHASE_U_LS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTxx_P02_7_OUT */
#define PHASE_V_HS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTxx_P02_1_OUT */
#define PHASE_V_LS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTxx_P02_4_OUT */
#define PHASE_W_HS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTxx_P02_2_OUT */
#define PHASE_W_LS                   (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTxx_P02_5_OUT */

/* Persistent module state (must be IFX_STATIC per structural rule) */
typedef struct
{
    IfxGtm_Pwm               pwm;                                      /* driver handle */
    IfxGtm_Pwm_Channel       channels[GTM_TOM_NUM_CHANNELS];           /* persistent channel handles */
    IfxGtm_Tom_Timer         timer;                                    /* shared TOM timer for update gating */
    float32                  dutyCycles[GTM_TOM_NUM_CHANNELS];         /* duty in percent [0..100] */
    float32                  phases[GTM_TOM_NUM_CHANNELS];             /* phase in degrees or percent-of-period (as configured) */
    IfxGtm_Pwm_DeadTime      deadTimes[GTM_TOM_NUM_CHANNELS];          /* per-channel dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv = {0};

/* ISR declaration: toggles LED only (no driver calls) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Empty period-event callback used by IfxGtm_Pwm interrupt configuration */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration structures as locals */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[GTM_TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;
    IfxGtm_Tom_Timer_Config      timerConfig;

    /* 2) Load default configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Configure outputs: HS (active high) with complementary LS (active low) */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;            /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;             /* LS active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: dead-time from migration values; select DTM CMU clock 0 */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_SEC;

    /* 5) Channel configuration: logical indices 0..2, initial phase 0, duties 25/50/75 */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = NULL_PTR; /* assigned below after interruptConfig is populated */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 6) Prepare interrupt configuration for period event on base channel only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel handles periodic notification */
    channelConfig[1].interrupt  = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Populate main PWM config (TOM/Cluster_1, center, sync start/updates, 20kHz) */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)GTM_TOM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = GTM_TOM_PWM_FREQ_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0; /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) GTM enable guard: enable + CMU clocks only if GTM is disabled */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 9) Initialize PWM driver; store persistent state */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phInv.phases[0] = 0.0f;
    g_gtmTom3phInv.phases[1] = 0.0f;
    g_gtmTom3phInv.phases[2] = 0.0f;

    g_gtmTom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 10) Initialize shared TOM timer for update gating (defaults are used; non-colliding channel selection TBD) */
    IfxGtm_Tom_Timer_initConfig(&timerConfig, &MODULE_GTM);
    {
        boolean timerOk = IfxGtm_Tom_Timer_init(&g_gtmTom3phInv.timer, &timerConfig);
        (void)timerOk; /* Error handling hook: application may assert/log if FALSE */
    }

    /* 11) Configure LED GPIO (push-pull output) for ISR toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule: if (duty + step) >= 100 reset to 0 then add step (three separate checks) */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Clamp to [MIN, MAX] after wrap handling */
    if (g_gtmTom3phInv.dutyCycles[0] < PHASE_DUTY_MIN) { g_gtmTom3phInv.dutyCycles[0] = PHASE_DUTY_MIN; }
    if (g_gtmTom3phInv.dutyCycles[1] < PHASE_DUTY_MIN) { g_gtmTom3phInv.dutyCycles[1] = PHASE_DUTY_MIN; }
    if (g_gtmTom3phInv.dutyCycles[2] < PHASE_DUTY_MIN) { g_gtmTom3phInv.dutyCycles[2] = PHASE_DUTY_MIN; }

    if (g_gtmTom3phInv.dutyCycles[0] > PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[0] = PHASE_DUTY_MAX; }
    if (g_gtmTom3phInv.dutyCycles[1] > PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[1] = PHASE_DUTY_MAX; }
    if (g_gtmTom3phInv.dutyCycles[2] > PHASE_DUTY_MAX) { g_gtmTom3phInv.dutyCycles[2] = PHASE_DUTY_MAX; }

    /* Issue synchronized update via TOM timer gating */
    IfxGtm_Tom_Timer_disableUpdate(&g_gtmTom3phInv.timer);
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
    IfxGtm_Tom_Timer_applyUpdate(&g_gtmTom3phInv.timer);
}
