/*
 * gtm_tom_3_phase_inverter_pwm.c
 *
 * Production driver: GTM TOM 3-Phase Inverter PWM using unified IfxGtm_Pwm
 * - Center-aligned complementary PWM @ 20 kHz
 * - 1 us rising/falling dead time
 * - Synchronous start and synchronous update enabled
 * - Interrupt routed via IfxGtm_Pwm InterruptConfig (CPU0, priority 20)
 * - LED toggled in ISR only
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"

/* ============================= Macros ============================= */
#define NUM_OF_CHANNELS           (3u)
#define PWM_FREQUENCY_HZ          (20000.0f)
#define ISR_PRIORITY_ATOM         (20)

/* User-requested pins (no validated TOUT symbols available here). Replace NULL_PTR with valid pin maps during integration. */
#define PHASE_U_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_0_TOUTx_P02_0_OUT */
#define PHASE_U_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_1_TOUTy_P02_7_OUT */
#define PHASE_V_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_2_TOUTx_P02_1_OUT */
#define PHASE_V_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_3_TOUTy_P02_4_OUT */
#define PHASE_W_HS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_4_TOUTx_P02_2_OUT */
#define PHASE_W_LS                (NULL_PTR) /* Replace with &IfxGtm_TOM1_5_TOUTy_P02_5_OUT */

#define PHASE_U_DUTY_INIT         (25.0f)
#define PHASE_V_DUTY_INIT         (50.0f)
#define PHASE_W_DUTY_INIT         (75.0f)
#define PHASE_DUTY_STEP           (10.0f)

/* LED: P13.0 */
#define LED                       &MODULE_P13, 0

/* ======================== Forward declarations ===================== */
/* ISR declaration (provider CPU0, priority ISR_PRIORITY_ATOM). The SDK supplies IFX_INTERRUPT. */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void);
void IfxGtm_periodEventFunction(void *data);

/* ========================= Module state ============================ */
typedef struct
{
    IfxGtm_Pwm              pwm;                              /* PWM handle */
    IfxGtm_Pwm_Channel      channels[NUM_OF_CHANNELS];        /* persistent channel state (driver references beyond init) */
    float32                 dutyCycles[NUM_OF_CHANNELS];       /* percent (0..100) */
    float32                 phases[NUM_OF_CHANNELS];           /* degrees or percent phase (driver expects float) */
    IfxGtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* stored configured dead-times */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_gtmTom3phInv;

/* ========================= ISR and callback ======================== */
void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================== Public API: init ========================= */
void initGtmTom3phInv(void)
{
    /* 1) Declare all configuration objects as locals */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_InterruptConfig  interruptConfig;
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 3) Load safe defaults */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 4) Output configuration (complementary, HS active-high, LS active-low) */
    output[0].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) DTM configuration: 1 us rising/falling */
    dtmConfig[0].deadTime.rising = 1e-6f; dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f; dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f; dtmConfig[2].deadTime.falling = 1e-6f;

    /* 5) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = (IfxGtm_Pwm_callBack)IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 5) Per-channel configuration: use logical indices 0..2, duty=0 at init */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = 0.0f;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel */

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = 0.0f;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = 0.0f;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main config fields */
    config.cluster              = IfxGtm_Cluster_1;                 /* select Cluster_1 as requested */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment            = IfxGtm_Pwm_Alignment_center;      /* center-aligned */
    config.syncStart            = TRUE;                              /* synchronous start */
    config.syncUpdateEnabled    = TRUE;                              /* synchronous update */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;
    config.clockSource.tom      = (uint32)IfxGtm_Cmu_Fxclk_0;       /* FXCLK0 for TOM */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_systemClock;

    /* 2 & 8) GTM enable and clock configuration inside enable guard */
    if (!IfxGtm_isEnabled(&MODULE_GTM)) {
        IfxGtm_enable(&MODULE_GTM);
        float32 freq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, freq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, freq);
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, (uint32)(IFXGTM_CMU_CLKEN_FXCLK | IFXGTM_CMU_CLKEN_CLK0));
    }

    /* 6 & 9) Initialize PWM with persistent handle and channels array */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 10) Store initial state: duty (percent), phases, and dead-times */
    g_gtmTom3phInv.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3phInv.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3phInv.phases[0]     = channelConfig[0].phase;
    g_gtmTom3phInv.phases[1]     = channelConfig[1].phase;
    g_gtmTom3phInv.phases[2]     = channelConfig[2].phase;

    g_gtmTom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_gtmTom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_gtmTom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 8/11) Configure LED GPIO as push-pull output (for ISR toggle) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ====================== Public API: update duty ==================== */
void updateGtmTom3phInvDuty(void)
{
    /* Wrap rule per requirement: check then unconditional add, three separate blocks */
    if ((g_gtmTom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3phInv.dutyCycles[2] = 0.0f; }

    g_gtmTom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply immediate update (driver manages sync shadow-transfer semantics) */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3phInv.pwm, (float32*)g_gtmTom3phInv.dutyCycles);
}
