/*
 * GTM TOM 3-Phase Inverter PWM driver (unified IfxGtm_Pwm)
 * - TC3xx family
 * - Center-aligned complementary PWM on TOM1 (Cluster_1)
 * - 3 phases (U, V, W)
 * - Frequency: 20 kHz
 * - Dead-time: 1.0 us (rising/falling)
 * - Clock source: FXCLK0
 * - Sync start/update enabled
 * - Interrupt: minimal ISR toggling LED on P13.0
 *
 * Notes:
 * - Watchdog disable MUST NOT be placed here (only in CpuX_Main.c).
 * - Pin routing symbols for TOUT are not provided in the validated list; the
 *   output pin pointers are therefore left as NULL_PTR and must be assigned
 *   to valid IfxGtm_TOMx_y_TOUTz_Pxx_y_OUT symbols in integration, matching
 *   the requested pins (P02.0/7/1/4/2/5) for phases U/V/W respectively.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */
#define NUM_OF_CHANNELS            (3U)
#define PWM_FREQUENCY              (20000.0f)   /* 20 kHz */

#define ISR_PRIORITY_ATOM          (20)

/* Initial duties in percent */
#define PHASE_U_DUTY_INIT          (25.0f)
#define PHASE_V_DUTY_INIT          (50.0f)
#define PHASE_W_DUTY_INIT          (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED on P13.0: compound macro (port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Pin routing placeholders for TOM1 complementary pairs.
 * IMPORTANT: Replace NULL_PTR with validated IfxGtm_TOM1_*_TOUT*_P02_*_OUT symbols
 * that match these user-requested pins:
 *   U: HS=P02.0, LS=P02.7
 *   V: HS=P02.1, LS=P02.4
 *   W: HS=P02.2, LS=P02.5
 */
#define PHASE_U_HS                 (NULL_PTR)
#define PHASE_U_LS                 (NULL_PTR)
#define PHASE_V_HS                 (NULL_PTR)
#define PHASE_V_LS                 (NULL_PTR)
#define PHASE_W_HS                 (NULL_PTR)
#define PHASE_W_LS                 (NULL_PTR)

/* ========================= Module State ========================= */
typedef struct
{
    IfxGtm_Pwm               pwm;                             /* unified PWM driver handle */
    IfxGtm_Pwm_Channel       channels[NUM_OF_CHANNELS];       /* persistent channels array */
    float32                  dutyCycles[NUM_OF_CHANNELS];      /* duty in percent */
    float32                  phases[NUM_OF_CHANNELS];          /* phase in degrees (or percent) */
    IfxGtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];       /* stored dead-times */
} GtmTom3PhPwm_State;

IFX_STATIC GtmTom3PhPwm_State g_gtmTom3PhState;

/* ========================= ISR and Callback Declarations ========================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);

void interruptGtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */
void initGtmTomPwm(void)
{
    /* 1) Local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load default configuration */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* 3) Output configuration: complementary HS/LS pairs on TOM1 cluster */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxGtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin       = (IfxGtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1.0 us on rising and falling edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 4) Base-channel interrupt configuration: period event on channel 0 only */
    interruptConfig.mode        = IfxGtm_IrqMode_pulseNotify;     /* period notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                /* CPU0 */
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.periodEvent = (IfxGtm_Pwm_callBack)IfxGtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel configurations: logical indices Ch_0..Ch_2 */
    /* U */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;   /* base channel interrupt */

    /* V */
    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;           /* base channel only */

    /* W */
    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;           /* base channel only */

    /* 5) Main PWM configuration */
    config.cluster              = IfxGtm_Cluster_1;                /* TOM1 (Cluster_1) */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;
    config.alignment            = IfxGtm_Pwm_Alignment_center;    /* center-aligned */
    config.syncStart            = TRUE;                           /* sync start */
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.atom     = (uint32)IfxGtm_Cmu_Fxclk_0;     /* FXCLK0 */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled    = TRUE;                           /* sync update */

    /* 6) GTM enable guard and CMU clock setup (inside guard) */
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

    /* 7) Initialize the unified PWM driver with persistent state */
    IfxGtm_Pwm_init(&g_gtmTom3PhState.pwm, &g_gtmTom3PhState.channels[0], &config);

    /* Store initial duties, phases, and dead-times in module state */
    g_gtmTom3PhState.dutyCycles[0] = PHASE_U_DUTY_INIT;
    g_gtmTom3PhState.dutyCycles[1] = PHASE_V_DUTY_INIT;
    g_gtmTom3PhState.dutyCycles[2] = PHASE_W_DUTY_INIT;

    g_gtmTom3PhState.phases[0] = 0.0f;
    g_gtmTom3PhState.phases[1] = 0.0f;
    g_gtmTom3PhState.phases[2] = 0.0f;

    g_gtmTom3PhState.deadTimes[0] = dtmConfig[0].deadTime;
    g_gtmTom3PhState.deadTimes[1] = dtmConfig[1].deadTime;
    g_gtmTom3PhState.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Synchronous start: handled by config.syncStart = TRUE in the unified driver */

    /* 9) Configure LED GPIO (debug toggle in ISR) AFTER PWM initialization */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateGtmTomPwmDutyCycles(void)
{
    /* Wrap rule then add step (no for-loops; explicit sequence) */
    if ((g_gtmTom3PhState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[0] = 0.0f; }
    if ((g_gtmTom3PhState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[1] = 0.0f; }
    if ((g_gtmTom3PhState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_gtmTom3PhState.dutyCycles[2] = 0.0f; }

    g_gtmTom3PhState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_gtmTom3PhState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_gtmTom3PhState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Immediate, coherent update of all configured channels */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_gtmTom3PhState.pwm, (float32*)g_gtmTom3PhState.dutyCycles);
}
