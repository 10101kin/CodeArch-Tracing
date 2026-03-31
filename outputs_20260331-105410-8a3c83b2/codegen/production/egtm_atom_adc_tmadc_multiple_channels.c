/*====================================================================================================================
 *  File: egtm_atom_adc_tmadc_multiple_channels.c
 *  Brief: EGTM ATOM unified PWM driver for 3-phase complementary inverter and TMADC trigger (TC4xx)
 *  Note : Production-ready per iLLD patterns; uses EGTM ATOM with unified IfxEgtm_Pwm driver
 *====================================================================================================================*/

#include "Ifx_Types.h"
#include "egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/*====================================================================================================================
 *  Configuration Macros (numeric values follow user-confirmed migration values)
 *====================================================================================================================*/
/* Channel counts */
#define EGTM_INV_NUM_CHANNELS                (3U)
#define EGTM_ADC_TRIG_NUM_CHANNELS           (1U)

/* PWM frequency (Hz) */
#define PWM_FREQUENCY_HZ                     (20000.0f)

/* Initial duty cycles (%) */
#define PHASE_U_DUTY                         (25.0f)
#define PHASE_V_DUTY                         (50.0f)
#define PHASE_W_DUTY                         (75.0f)

/* Duty step (%) for external ramps (not used here; provided for integration completeness) */
#define PHASE_DUTY_STEP                      (0.01f)

/* ISR priority macro (used by IFX_INTERRUPT) */
#define ISR_PRIORITY_ATOM                    (25)

/* LED pin (compound macro: port, pin) — user requirement P03.9 */
#define LED                                  &MODULE_P03, 9U

/*====================================================================================================================
 *  Pin Routing Macros (EGTM ATOM → TOUT to package pins)
 *  User-requested mappings (KIT_A3G_TC4D7_LITE):
 *    - Phase U: P20.8 (HS), P20.9  (LS)
 *    - Phase V: P20.10(HS), P20.11 (LS)
 *    - Phase W: P20.12(HS), P20.13 (LS)
 *    - ADC Trig: P33.0 (ATOM CH3)
 *  Note: These symbols come from IfxEgtm_PinMap headers in the iLLD.
 *====================================================================================================================*/
#define PHASE_U_HS                           (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                           (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                           (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                           (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                           (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                           (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)
#define ADC_TRIGGER_PIN                      (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* Complementary polarity convention (MANDATORY):
 *   - High-side     active HIGH
 *   - Low-side (N)  active LOW
 */
#define EGTM_POLARITY_HS                     (Ifx_ActiveState_high)
#define EGTM_POLARITY_LS                     (Ifx_ActiveState_low)

/*====================================================================================================================
 *  Module State
 *====================================================================================================================*/

typedef struct
{
    IfxEgtm_Pwm              pwm;                                           /* Driver handle (persistent) */
    IfxEgtm_Pwm_Channel      channels[EGTM_INV_NUM_CHANNELS];               /* Persistent channels array */
    float32                  dutyCycles[EGTM_INV_NUM_CHANNELS];             /* Duty (%) */
    float32                  phases[EGTM_INV_NUM_CHANNELS];                 /* Phase (deg or %) */
    IfxEgtm_Pwm_DeadTime     deadTimes[EGTM_INV_NUM_CHANNELS];              /* Dead-times (s) */
} EgtmAtom3phInv_State;

typedef struct
{
    IfxEgtm_Pwm              pwm;                                           /* Driver handle (persistent) */
    IfxEgtm_Pwm_Channel      channels[EGTM_ADC_TRIG_NUM_CHANNELS];          /* Persistent channels array */
    float32                  dutyCycles[EGTM_ADC_TRIG_NUM_CHANNELS];        /* Duty (%) */
} EgtmAtomAdcTrig_State;

/* IFX_STATIC is required by the coding rules (Compilers.h provides it) */
IFX_STATIC EgtmAtom3phInv_State   g_egtmInv;
IFX_STATIC EgtmAtomAdcTrig_State  g_egtmAdcTrig;

/*====================================================================================================================
 *  ISR and Callback (declared BEFORE init per structural rules)
 *====================================================================================================================*/
/* Period-event callback (assigned via InterruptConfig). Empty body by design. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR: keep body minimal — toggle LED only. Priority uses ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*====================================================================================================================
 *  Local Helpers
 *====================================================================================================================*/
/* Configure EGTM clocks if module is not yet enabled. MANDATORY enable guard pattern. */
static void egtm_enableIfNeeded(void)
{
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }
}

/*====================================================================================================================
 *  Public API Implementations
 *====================================================================================================================*/
/**
 * Initialize a 3-channel complementary, center-aligned PWM inverter on EGTM Cluster 0 ATOM channels 0..2 at 20 kHz
 * with 1 µs dead-time on both edges using the unified high-level PWM driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local configuration structures */
    IfxEgtm_Pwm_Config           config;                                      /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_INV_NUM_CHANNELS];        /* Per-channel config */
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_INV_NUM_CHANNELS];            /* Per-channel dead-time */
    IfxEgtm_Pwm_OutputConfig     output[EGTM_INV_NUM_CHANNELS];               /* Per-channel output */

    /* Optional: Interrupt configuration — assigned to base channel (index 0) */
    IfxEgtm_Pwm_InterruptConfig  irqCfg;
    irqCfg.mode        = (IfxEgtm_IrqMode)0;               /* Implementation-defined (pulse notify) */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output configuration: complementary pairs with required polarities */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = EGTM_POLARITY_HS;
    output[0].complementaryPolarity = EGTM_POLARITY_LS;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = EGTM_POLARITY_HS;
    output[1].complementaryPolarity = EGTM_POLARITY_LS;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = EGTM_POLARITY_HS;
    output[2].complementaryPolarity = EGTM_POLARITY_LS;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) Dead-time configuration: 1 µs on both edges for all channels */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 4) Channel configuration (logical indices 0..2 map to ATOM Ch0..Ch2) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;                   /* Base channel gets interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 5) Main PWM configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;                       /* Start all channels in sync */
    config.syncUpdateEnabled  = TRUE;                       /* Buffered updates */
    config.numChannels        = (uint8)EGTM_INV_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;  /* Union field for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0; /* DTM clock source */

    /* 6) Enable-guard and CMU clock setup inside the guard */
    egtm_enableIfNeeded();

    /* 7) Initialize PWM driver (stores pointer to persistent channels array) */
    IfxEgtm_Pwm_init(&g_egtmInv.pwm, &g_egtmInv.channels[0], &config);

    /* 8) Copy initial duty, phase, and dead-time values into persistent state */
    g_egtmInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmInv.phases[0] = channelConfig[0].phase;
    g_egtmInv.phases[1] = channelConfig[1].phase;
    g_egtmInv.phases[2] = channelConfig[2].phase;

    g_egtmInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 9) Configure status LED as output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Runtime update for the 3-phase inverter: copy three duty values from requestDuty[0..2] into the module's
 * persistent dutyCycles[0..2] without scaling, then apply them immediately using the driver's multi-channel
 * immediate duty update API.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy incoming duties (percent) into persistent state */
    g_egtmInv.dutyCycles[0] = requestDuty[0];
    g_egtmInv.dutyCycles[1] = requestDuty[1];
    g_egtmInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmInv.pwm, (float32 *)g_egtmInv.dutyCycles);
}

/**
 * Configure a separate single-channel PWM on EGTM Cluster 0 ATOM channel 3 to generate an edge-aligned 50% duty
 * time-base used as a trigger source for TMADC. TMADC trigger consumption is configured elsewhere.
 */
void initEgtmAtomAdcTrigger(void)
{
    /* 1) Declare local configuration structures */
    IfxEgtm_Pwm_Config           config;                                 /* Main PWM config */
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_ADC_TRIG_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_ADC_TRIG_NUM_CHANNELS];

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output routing for ATOM Ch3 → P33.0 */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)ADC_TRIGGER_PIN;
    output[0].complementaryPin      = NULL_PTR;                          /* Not used */
    output[0].polarity              = EGTM_POLARITY_HS;                  /* Active high */
    output[0].complementaryPolarity = EGTM_POLARITY_LS;                  /* Don't care */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Channel configuration: ATOM logical Ch3, 50% duty */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = 50.0f;
    channelConfig[0].dtm       = NULL_PTR;                               /* No DTM for trigger */
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR;                               /* No interrupt needed */

    /* Main PWM configuration fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_edge;              /* Edge-aligned */
    config.syncStart          = FALSE;
    config.syncUpdateEnabled  = FALSE;
    config.numChannels        = (uint8)EGTM_ADC_TRIG_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* Union field for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* Not used but set */

    /* 3) Enable-guard and CMU clock setup inside the guard */
    egtm_enableIfNeeded();

    /* 4) Initialize PWM driver (stores pointer to persistent channels array) */
    IfxEgtm_Pwm_init(&g_egtmAdcTrig.pwm, &g_egtmAdcTrig.channels[0], &config);

    /* Store initial duty into persistent state */
    g_egtmAdcTrig.dutyCycles[0] = channelConfig[0].duty;
}
