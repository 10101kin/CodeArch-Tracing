/*
 * gtm_tom_3_phase_inverter_pwm_2.c
 *
 * Production driver for GTM TOM 3-Phase Inverter PWM (Instance 2)
 * - TC3xx family, TOM submodule, Cluster_1
 * - Center-aligned, 20 kHz, complementary pairs with dead-time
 * - Single period ISR toggles diagnostic LED (P13.0)
 */

#include "gtm_tom_3_phase_inverter_pwm_2.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================================================
 * Configuration macros (migration-confirmed values)
 * ========================================================= */
#define TOM_NUM_CHANNELS              (3u)
#define PWM_SWITCHING_FREQ_HZ         (20000.0f)
#define ISR_PRIORITY_ATOM             (20)

#define PHASE_U_DUTY_INIT             (25.0f)  /* % */
#define PHASE_V_DUTY_INIT             (50.0f)  /* % */
#define PHASE_W_DUTY_INIT             (75.0f)  /* % */
#define PHASE_DUTY_STEP               (10.0f)  /* % */

/* Dead-time per user requirement: 1 us (overrides migration 0.5 us) */
#define PWM_DEAD_TIME_SECONDS         (1.0e-06f)

/* LED: compound macro (port, pin) for ISR toggle */
#define LED                           &MODULE_P13, 0

/*
 * Pin routing placeholders (TOM1, Cluster_1 pairs 0/1, 2/3, 4/5):
 * User-requested pins:
 *  - Phase U: P02.0 (HS) / P02.7 (LS)
 *  - Phase V: P02.1 (HS) / P02.4 (LS)
 *  - Phase W: P02.2 (HS) / P02.5 (LS)
 *
 * No validated TOUT symbols were provided in the context. Replace NULL_PTR with
 * the correct &IfxGtm_TOM1_x_TOUTy_Pxx_y_OUT symbols from the device pin map
 * (IfxGtm_PinMap_TC38x_516) during integration while preserving 0/1, 2/3, 4/5 pairing.
 */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUT<P02.0>_P02_0_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUT<P02.7>_P02_7_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUT<P02.1>_P02_1_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUT<P02.4>_P02_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUT<P02.2>_P02_2_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUT<P02.5>_P02_5_OUT */

/* =========================================================
 * Module state
 * ========================================================= */

typedef struct
{
    IfxGtm_Pwm             pwm;                               /* driver handle */
    IfxGtm_Pwm_Channel     channels[TOM_NUM_CHANNELS];        /* persistent channels storage */
    float32                dutyCycles[TOM_NUM_CHANNELS];      /* % */
    float32                phases[TOM_NUM_CHANNELS];          /* deg or % phase, here 0.0f */
    IfxGtm_Pwm_DeadTime    deadTimes[TOM_NUM_CHANNELS];       /* s */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv2;

/* =========================================================
 * Period callback (assigned to InterruptConfig.periodEvent)
 * MUST be non-static and empty by design.
 * ========================================================= */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================================
 * ISR (priority-bound vector). The high-level driver configures SRC
 * with priority and provider; this ISR toggles the diagnostic LED.
 * ========================================================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* =========================================================
 * initGtmTom3phInv
 * ========================================================= */
void initGtmTom3phInv(void)
{
    /* 1) Declare all local config structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[TOM_NUM_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;       /* HS: active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;        /* LS: active LOW  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration */
    dtmConfig[0].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[0].deadTime.falling = PWM_DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[1].deadTime.falling = PWM_DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.rising = PWM_DEAD_TIME_SECONDS;
    dtmConfig[2].deadTime.falling = PWM_DEAD_TIME_SECONDS;

    /* 5) Interrupt configuration (period/base event on channel 0) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;   /* period notification */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration: logical indices 0..2 */
    /* Channel 0 -> Phase U */
    channelConfig[0].timerCh   = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;  /* base ISR on ch0 */

    /* Channel 1 -> Phase V */
    channelConfig[1].timerCh   = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Channel 2 -> Phase W */
    channelConfig[2].timerCh   = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;
    config.syncStart            = TRUE;
    config.syncUpdateEnabled    = TRUE;
    config.numChannels          = (uint8)TOM_NUM_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_SWITCHING_FREQ_HZ;
    config.clockSource.tom      = IfxGtm_Cmu_Fxclk_0;              /* TOM uses FXCLK */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;

    /* 8) Enable guard and CMU clocks (inside guard ONLY) */
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

    /* 9) Initialize PWM driver (persistent channels buffer in module state) */
    IfxGtm_Pwm_init(&g_gtmTom3phInv2.pwm, &g_gtmTom3phInv2.channels[0], &config);

    /* 10) Persist initial duty cycles, phases, and dead-times into state */
    g_gtmTom3phInv2.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInv2.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInv2.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phInv2.phases[0] = 0.0f;
    g_gtmTom3phInv2.phases[1] = 0.0f;
    g_gtmTom3phInv2.phases[2] = 0.0f;

    g_gtmTom3phInv2.deadTimes[0].rising = PWM_DEAD_TIME_SECONDS;
    g_gtmTom3phInv2.deadTimes[0].falling = PWM_DEAD_TIME_SECONDS;
    g_gtmTom3phInv2.deadTimes[1].rising = PWM_DEAD_TIME_SECONDS;
    g_gtmTom3phInv2.deadTimes[1].falling = PWM_DEAD_TIME_SECONDS;
    g_gtmTom3phInv2.deadTimes[2].rising = PWM_DEAD_TIME_SECONDS;
    g_gtmTom3phInv2.deadTimes[2].falling = PWM_DEAD_TIME_SECONDS;

    /* 11) Configure diagnostic LED pin as push-pull output (no level change) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
