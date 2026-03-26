/*
 * gtm_tom_3_phase_inverter_pwm.c
 * Production driver: GTM TOM 3-Phase Complementary PWM using IfxGtm_Pwm.
 *
 * Notes:
 * - Follows authoritative iLLD initialization patterns for IfxGtm_Pwm.
 * - No watchdog disable here (Cpu0_Main.c only, per project architecture).
 * - No STM timing/scheduling in this driver.
 */

#include "gtm_tom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxGtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS            (3)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (20)

/* Phase initial duties in percent */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)
#define PHASE_DUTY_STEP            (10.0f)

/* LED pin (compound macro: port, pin) */
#define LED                        &MODULE_P13, 0

/*
 * Pin routing macros for complementary TOM outputs.
 * NOTE: Pin symbols must come from the device's PinMap headers.
 * In migration mode, keep placeholders if mapping is pending.
 * Assign actual symbols (e.g., &IfxGtm_TOM1_0_TOUT0_P02_0_OUT, &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT)
 * once validated for the target board/package.
 */
#define PHASE_U_HS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_0_TOUT0_P02_0_OUT */
#define PHASE_U_LS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_0N_TOUT7_P02_7_OUT */
#define PHASE_V_HS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_1_TOUT1_P02_1_OUT */
#define PHASE_V_LS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_1N_TOUT4_P02_4_OUT */
#define PHASE_W_HS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_2_TOUT2_P02_2_OUT */
#define PHASE_W_LS                 (NULL_PTR)  /* e.g., &IfxGtm_TOM1_2N_TOUT5_P02_5_OUT */

/* =============================
 * Module State
 * ============================= */
typedef struct
{
    IfxGtm_Pwm           pwm;                         /* driver handle */
    IfxGtm_Pwm_Channel   channels[NUM_OF_CHANNELS];   /* persistent channel array */
    float32              dutyCycles[NUM_OF_CHANNELS]; /* in percent */
    float32              phases[NUM_OF_CHANNELS];     /* in degrees (unused -> 0.0f) */
    IfxGtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];  /* stored for reference */
} GtmTom3phInv_State;

IFX_STATIC GtmTom3phInv_State g_tom3phInv = {0};

/* =============================
 * ISR and Callback (file-scope, before init)
 * ============================= */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Minimal ISR: toggle LED for debug/heartbeat */
    IfxPort_togglePin(LED);
}

void IfxGtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty per design */
}

/* =============================
 * Public API Implementations
 * ============================= */

void initGtmTom3phInv(void)
{
    /* 1) Declare local configuration structures */
    IfxGtm_Pwm_Config           config;
    IfxGtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxGtm_Pwm_InterruptConfig  irqCfg;
    IfxGtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults for the unified GTM PWM driver */
    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Output configuration for three complementary pairs (HS/LS) */
    /* Phase U */
    output[0].pin                    = (IfxGtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxGtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */
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

    /* Dead-time configuration: 1 us both edges for all three pairs */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;

    /* 3) Interrupt configuration: pulse-notify, CPU0, priority 20, period callback */
    irqCfg.mode        = IfxGtm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxGtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 4) Per-channel configuration: TOM1 cluster_1 channels 8,10,12 as base channels */
    /* Base channel carries the period event interrupt configuration */
    channelConfig[0].timerCh    = IfxGtm_Pwm_SubModule_Ch_8;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &irqCfg;

    channelConfig[1].timerCh    = IfxGtm_Pwm_SubModule_Ch_10;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    channelConfig[2].timerCh    = IfxGtm_Pwm_SubModule_Ch_12;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* Complete main IfxGtm_Pwm configuration */
    config.cluster              = IfxGtm_Cluster_1;                  /* TOM1 Cluster_1 */
    config.subModule            = IfxGtm_Pwm_SubModule_tom;          /* TOM submodule  */
    config.alignment            = IfxGtm_Pwm_Alignment_center;       /* center-aligned */
    config.syncStart            = TRUE;                               /* sync start     */
    config.syncUpdateEnabled    = TRUE;                               /* shadow update  */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY_HZ;                   /* 20 kHz         */
    config.clockSource.atom     = IfxGtm_Cmu_Fxclk_0;                 /* FXCLK0 source  */
    config.dtmClockSource       = IfxGtm_Dtm_ClockSource_cmuClock0;   /* DTM from CLK0  */

    /* 5) GTM enable guard + CMU clocks (ALL inside guard) */
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

    /* 6) Initialize PWM with persistent handle and channels[] */
    IfxGtm_Pwm_init(&g_tom3phInv.pwm, &g_tom3phInv.channels[0], &config);

    /* 7) Persist configured state for runtime updates */
    g_tom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_tom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_tom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_tom3phInv.phases[0] = 0.0f;
    g_tom3phInv.phases[1] = 0.0f;
    g_tom3phInv.phases[2] = 0.0f;

    g_tom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_tom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_tom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure LED GPIO and drive low initially per requirements.
     * Per structural rule, only configure mode here; initial state handling is done by application/ISR.
     */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    /* Intentionally no IfxPort_setPinLow() here (driver keeps ISR-exclusive LED control). */
}

void updateGtmTom3phInvDuty(void)
{
    /* Percent-based +10% step with wrap rule (no loop: three explicit sequences) */
    if ((g_tom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[0] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[1] = 0.0f; }
    if ((g_tom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_tom3phInv.dutyCycles[2] = 0.0f; }

    g_tom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_tom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties immediately and synchronously across the group */
    IfxGtm_Pwm_updateChannelsDutyImmediate(&g_tom3phInv.pwm, (float32 *)g_tom3phInv.dutyCycles);
}
