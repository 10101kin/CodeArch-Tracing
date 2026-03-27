/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Inverter PWM using IfxGtm_Pwm on TC3xx
 * - Center-aligned 20 kHz, complementary HS/LS active-high with 1 us dead-time
 * - Synchronous start/update enabled
 * - LED P13.0 toggled from GTM ATOM0 CH0 ISR (priority 20)
 *
 * Notes:
 * - Followed iLLD unified PWM init pattern (IfxGtm_Pwm) and mandatory GTM CMU enable guard
 * - No watchdog handling here (must be in CpuX main)
 * - No STM timing in this module (done in CpuX main while-loop)
 */

#include "gtm_tom_3_phase_inverter_pwm.h"

/* iLLD headers (only in .c) */
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxGtm_Cmu.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =========================================
 * Configuration Macros (from requirements)
 * ========================================= */
#define NUM_OF_CHANNELS              (3u)
#define PWM_FREQUENCY_HZ             (20000.0f)     /* 20 kHz */
#define PWM_DEADTIME_SEC             (1.0e-6f)      /* 1 us rising/falling */
#define ISR_PRIORITY_ATOM            (20u)

/* Initial duty cycles (percent) */
#define PHASE_U_DUTY_INIT            (25.0f)
#define PHASE_V_DUTY_INIT            (50.0f)
#define PHASE_W_DUTY_INIT            (75.0f)

/* Update step (percent) */
#define PHASE_DUTY_STEP              (10.0f)

/* LED P13.0 (compound macro expands to two args: port, pin) */
#define LED                          &MODULE_P13, 0u

/*
 * Pin routing macros
 *
 * User requested routing:
 *   U: HS=P02.0, LS=P02.7
 *   V: HS=P02.1, LS=P02.4
 *   W: HS=P02.2, LS=P02.5
 *
 * IMPORTANT:
 *  - Pin symbols (IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT) are device/package specific and
 *    provided in the GTM PinMap headers. As validated symbols are not available
 *    in this context, set to NULL for now. Replace NULL with the correct symbols
 *    from the appropriate IfxGtm_PinMap_TC38x_516.h when integrating on target.
 */
#define PHASE_U_HS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_U_LS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_HS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_V_LS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_HS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)
#define PHASE_W_LS_TOUT              ((IfxGtm_Pwm_ToutMap*)0)

/* =========================================
 * Module State
 * ========================================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                                  /* driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];            /* persistent channel handles */
    float32                  dutyCycles[NUM_OF_CHANNELS];          /* duty (%) */
    float32                  phases[NUM_OF_CHANNELS];              /* phase (deg or %) - reserved */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];           /* persisted dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gmtTom3phInvState = {0};

/* =========================================
 * ISR and Callback Declarations/Definitions
 * ========================================= */

/* ISR: GTM ATOM interrupt toggles LED. Registration/routing is done externally. */
IFX_INTERRUPT(interruptGtmAtom0Ch0, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom0Ch0(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback required by unified PWM driver InterruptConfig (empty body) */
void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================
 * Public API Implementations
 * ========================================= */

/**
 * Initialize a 3-phase complementary PWM using the unified GTM PWM driver on TOM1, Cluster_1.
 * Center-aligned at 20 kHz, 1 us dead-time, complementary HS/LS active-high with sync start/update.
 */
void initGtmTom3phInv(void)
{
    /* 1) Declare configuration structures (local) */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Populate defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary pairs, active-high HS, active-low LS, push-pull */
    /* U phase */
    output[0].pin                    = PHASE_U_HS_TOUT;          /* HS: P02.0 -> fill correct TOUT symbol during integration */
    output[0].complementaryPin       = PHASE_U_LS_TOUT;          /* LS: P02.7 -> fill correct TOUT symbol during integration */
    output[0].polarity               = Ifx_ActiveState_high;     /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;      /* LS active low (complementary) */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* V phase */
    output[1].pin                    = PHASE_V_HS_TOUT;          /* HS: P02.1 */
    output[1].complementaryPin       = PHASE_V_LS_TOUT;          /* LS: P02.4 */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* W phase */
    output[2].pin                    = PHASE_W_HS_TOUT;          /* HS: P02.2 */
    output[2].complementaryPin       = PHASE_W_LS_TOUT;          /* LS: P02.5 */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* DTM configuration: 1.0 us rising/falling */
    dtmConfig[0].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[0].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[1].deadTime.falling = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.rising = PWM_DEADTIME_SEC;
    dtmConfig[2].deadTime.falling = PWM_DEADTIME_SEC;

    /* Interrupt configuration: period notification, CPU0, priority set, callback linked */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* Channel configuration (logical indices 0..2) */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;    /* 25% */
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;              /* base channel provides period interrupt */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;    /* 50% */
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;             /* no ISR on sync channels */

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;    /* 75% */
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Main PWM configuration */
    config.gtmSFR             = &MODULE_GTM;
    config.cluster            = IfxGtm_Cluster_1;             /* TOM1 Cluster_1 (TGC1) */
    config.subModule          = IfxGtm_Pwm_SubModule_tom;     /* Use TOM */
    config.alignment          = IfxGtm_Pwm_Alignment_center;  /* Center-aligned */
    config.syncStart          = TRUE;                          /* Start channels synchronously */
    config.syncUpdateEnabled  = TRUE;                          /* Shadow transfer at period */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;              /* 20 kHz */
    /* Clock sources: FXCLK0 for TOM, DTM from CMU clock0 */
    config.clockSource        = IfxGtm_Pwm_ClockSource_cmuFxclk0;   /* FXCLK0 for TOM */
    config.dtmClockSource     = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM from CMU CLK0 */

    /* 5) GTM enable/clock guard (all CMU calls inside the guard) */
    if (!IfxGtm_isEnabled(&MODULE_GTM))
    {
        IfxGtm_enable(&MODULE_GTM);
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);                 /* GCLK = module freq */
        IfxGtm_Cmu_selectClkInput(&MODULE_GTM, IfxGtm_Cmu_Clk_0, TRUE);       /* CLK0 from GCLK */
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);/* CLK0 = GCLK freq */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize unified PWM driver with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gmtTom3phInvState.pwm, &g_gmtTom3phInvState.channels[0], &config);

    /* 7) Persist initial duties, phases, dead-times in module state */
    g_gmtTom3phInvState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gmtTom3phInvState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gmtTom3phInvState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gmtTom3phInvState.phases[0] = 0.0f;
    g_gmtTom3phInvState.phases[1] = 0.0f;
    g_gmtTom3phInvState.phases[2] = 0.0f;

    g_gmtTom3phInvState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gmtTom3phInvState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gmtTom3phInvState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Synchronous start handled by config.syncStart = TRUE at init */

    /* 9) LED GPIO: P13.0 as push-pull output, initial low */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED, IfxPort_State_low);
}

/**
 * Update three-phase PWM duty cycles with wrap rule and apply synchronously.
 * Rule: for each phase, if (duty + 10%) >= 100% then duty = 0; then duty += 10%.
 */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule applied separately per phase (no loop as per requirement) */
    if ((g_gmtTom3phInvState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gmtTom3phInvState.dutyCycles[0] = 0.0f; }
    if ((g_gmtTom3phInvState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gmtTom3phInvState.dutyCycles[1] = 0.0f; }
    if ((g_gmtTom3phInvState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gmtTom3phInvState.dutyCycles[2] = 0.0f; }

    g_gmtTom3phInvState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gmtTom3phInvState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gmtTom3phInvState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply the three updated duties synchronously (immediate array-based API) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gmtTom3phInvState.pwm, (float32*)g_gmtTom3phInvState.dutyCycles);
}
