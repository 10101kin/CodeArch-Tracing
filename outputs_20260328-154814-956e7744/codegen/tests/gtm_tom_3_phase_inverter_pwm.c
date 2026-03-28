/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production-ready 3-phase inverter PWM driver (TC3xx) using IfxGtm_Pwm
 *
 * Behavior:
 *  - Initializes TOM1, Cluster_1 for 3 complementary center-aligned PWM channels
 *  - 20 kHz PWM, 1 us dead-time (rising and falling)
 *  - Sync start and sync update enabled
 *  - Phase duties in percent: U=25, V=50, W=75 (maintained in module state)
 *  - updateGtmTom3phInvDuty() applies 10% step with wrap rule and updates immediately
 *  - Provides empty period-event callback and an ISR that toggles LED P13.0
 *
 * Notes:
 *  - No watchdog handling is present (must be in CpuX_Main.c only)
 *  - No STM timing here; higher-level scheduling handles dwell timing
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =====================================================================================
 * Configuration macros (numeric constants from requirements)
 * ===================================================================================== */
#define NUM_OF_CHANNELS            (3u)            /* 3 complementary pairs = 3 channels */
#define PWM_FREQUENCY_HZ           (20000.0f)      /* 20 kHz */
#define DEAD_TIME_SEC              (1e-6f)         /* 1 us dead-time */
#define ISR_PRIORITY_ATOM          (20)            /* Period ISR priority */

#define PHASE_U_INIT_DUTY          (25.0f)         /* percent */
#define PHASE_V_INIT_DUTY          (50.0f)         /* percent */
#define PHASE_W_INIT_DUTY          (75.0f)         /* percent */
#define PHASE_DUTY_STEP            (10.0f)         /* percent step */

/* LED: P13.0 (compound macro: port, pin) */
#define LED                        &MODULE_P13, 0

/* =====================================================================================
 * Pin routing placeholders (validated TOUT symbols were not provided in this context)
 * Replace NULL_PTR with proper IfxGtm_TOM1/TGC1 TOUT pin symbols during integration.
 * ===================================================================================== */
#define PHASE_U_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_0_OUT */
#define PHASE_U_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_7_OUT */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_1_OUT */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_4_OUT */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_2_OUT */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxGtm_TOM1_x_TOUTxx_P02_5_OUT */

/* =====================================================================================
 * Module state (persistent)
 * ===================================================================================== */
typedef struct
{
    IfxGtm_Pwm              pwm;                               /* driver handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];         /* persistent channels array */
    float32                 dutyCycles[3];                      /* U,V,W duties in percent */
    float32                 phases[3];                          /* per-phase shift in degrees (if used) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* stored dead-times */
} GtmTom3ph_State;

IFX_STATIC GtmTom3ph_State g_gtmTom3phState;                   /* IFX_STATIC per requirements */

/* =====================================================================================
 * Forward declarations: ISR and period-event callback (must appear before init)
 * ===================================================================================== */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void IfxGtm_periodEventFunction(void *data);

/* =====================================================================================
 * ISR: toggles LED P13.0; do not call any driver handler here
 * ===================================================================================== */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* =====================================================================================
 * Period-event callback: empty body as specified
 * ===================================================================================== */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =====================================================================================
 * initGtmTom3phInv
 * ===================================================================================== */
void initGtmTom3phInv(void)
{
    /* 1) Declare configuration objects as locals */
    IfxGtm_Pwm_Config            config;
    IfxGtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig   interruptConfig;

    /* 2) Populate main config with defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration (three channels with complementary pins) */
    /* Complementary polarity convention: HS active HIGH, LS active LOW */
    output[0].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                = Ifx_ActiveState_high;
    output[0].complementaryPolarity   = Ifx_ActiveState_low;
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                     = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin        = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1 us rising/falling dead time */
    dtmConfig[0].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SEC;

    /* 4) Interrupt configuration: base channel routes unified period-event callback */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6 channels (3 pairs) per original requirement collapse to 3 IfxGtm_Pwm channels
       because each channel carries complementaryPin; we map logical CH0..CH2 */

    /* Channel 0: Phase U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_INIT_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;     /* unified period-event on base channel */

    /* Channel 1: Phase V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_INIT_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;             /* only channel 0 has the interrupt */

    /* Channel 2: Phase W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_INIT_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 5) GTM enable guard + CMU clock configuration (inside guard only) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        {
            float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
            /* Per iLLD pattern: set GCLK and CLK0 to module frequency, enable FXCLK + CLK0 */
            IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
            IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
            IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
        }
    }

    /* 7/3) Main PWM configuration */
    config.gtmSFR              = &MODULE_GTM;
    config.cluster             = IfxGtm_Cluster_1;                 /* TOM1 Cluster_1 (TGC1) */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment           = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart           = TRUE;                              /* synchronous start */
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY_HZ;
    config.clockSource.atom    = (uint32)IfxGtm_Cmu_Fxclk_0;       /* use FXCLK0 */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock from CMU CLK0 */
    config.syncUpdateEnabled   = TRUE;                              /* synchronized updates */

    /* 6) Initialize PWM driver (applies channel/output settings and routes pins) */
    IfxGtm_Pwm_init(&g_gtmTom3phState.pwm, &g_gtmTom3phState.channels[0], &config);

    /* 7) Store initial duties, phases, and dead-times in module state */
    g_gtmTom3phState.dutyCycles[0] = PHASE_U_INIT_DUTY;
    g_gtmTom3phState.dutyCycles[1] = PHASE_V_INIT_DUTY;
    g_gtmTom3phState.dutyCycles[2] = PHASE_W_INIT_DUTY;

    g_gtmTom3phState.phases[0] = 0.0f;
    g_gtmTom3phState.phases[1] = 0.0f;
    g_gtmTom3phState.phases[2] = 0.0f;

    g_gtmTom3phState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3phState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3phState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO and drive LOW */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(LED);
}

/* =====================================================================================
 * updateGtmTom3phInvDuty
 * ===================================================================================== */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per requirement: if (duty + step) >= 100 -> duty = 0; then add step */
    if ((g_gtmTom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phState.dutyCycles[2] = 0.0f; }

    g_gtmTom3phState.dutyCycles[0] += PHASE_DUTY_STEP; /* U */
    g_gtmTom3phState.dutyCycles[1] += PHASE_DUTY_STEP; /* V */
    g_gtmTom3phState.dutyCycles[2] += PHASE_DUTY_STEP; /* W */

    /* Apply immediately to all configured channels (3 complementary pairs) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phState.pwm, (float32*)g_gtmTom3phState.dutyCycles);
}
