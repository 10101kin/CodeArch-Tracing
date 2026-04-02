/*
 * egtmatompwm.c
 *
 * Production EGTM ATOM PWM driver for TC4xx per iLLD unified PWM API.
 * Implements 3 complementary, center-aligned phase pairs with dead-time
 * and an additional edge-aligned trigger channel for ADC sampling.
 */

#include "egtmatompwm.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"

/* ======================== Configuration Macros (from user-confirmed values) ======================== */
#define NUM_OF_CHANNELS                (3U)
#define PWM_FREQUENCY                  (20000.0f)     /* Hz */
#define PHASE_U_DUTY                   (25.0f)        /* percent */
#define PHASE_V_DUTY                   (50.0f)        /* percent */
#define PHASE_W_DUTY                   (75.0f)        /* percent */
#define PHASE_DUTY_STEP                (0.01f)        /* percent step */
#define ADC_TRIG_DUTY                  (50.0f)        /* percent */

/* ISR priority for ATOM events (period event on base channel) */
#define ISR_PRIORITY_ATOM              (25)

/* LED pin (compound macro: expands to two arguments: port pointer, pin index) */
#define LED                            (&MODULE_P03), 9

/* ======================== Validated EGTM TOUT pin symbols ========================
 * Only a subset is provided. The requested pins (P20.8..P20.13, P33.0) are not
 * present in the validated list provided for this template. Use NULL_PTR placeholders
 * and replace with the correct &IfxEgtm_ATOMx_y[_N]_TOUTz_Pxx_y_OUT symbols during
 * board integration.
 */
#define PHASE_U_HS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P20_8_OUT */
#define PHASE_U_LS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT if matches, else correct TOUT for P20.9 */
#define PHASE_V_HS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P20_10_OUT */
#define PHASE_V_LS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P20_11_OUT */
#define PHASE_W_HS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P20_12_OUT */
#define PHASE_W_LS                     (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P20_13_OUT */
#define ADC_TRIG_OUT                   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_x_TOUT_P33_0_OUT if used on a pad */

/* ======================== Module State ======================== */

typedef struct
{
    IfxEgtm_Pwm              pwm;                                  /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* persistent channels array */
    float32                  dutyCycles[NUM_OF_CHANNELS];          /* duty state in percent */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];           /* per-phase dead-time */
    float32                  phases[NUM_OF_CHANNELS];              /* phase offsets (sec or percent as API expects) */
} EgtmAtom3phState;

/* Static 3-phase PWM state */
IFX_STATIC EgtmAtom3phState g_egtm3ph;

/* Separate static state for the ADC trigger channel (ATOM ch3) */
typedef struct
{
    IfxEgtm_Pwm         pwm;                 /* single-channel PWM handle */
    IfxEgtm_Pwm_Channel channel[1];          /* persistent channel */
    float32             duty;                /* stored duty */
} EgtmAtomTrigState;

IFX_STATIC EgtmAtomTrigState g_egtmTrig;

/* ======================== ISR and Callback Declarations ======================== */

/* Hardware ISR: required minimal body (toggle LED). Priority is ISR_PRIORITY_ATOM. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* SW/API-visible ISR per design contract: also minimal and independent. */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback used by PWM InterruptConfig: must exist, empty body. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* intentionally empty */
}

/* ======================== Public Functions ======================== */

/**
 * Initialize EGTM unified PWM for 3 complementary, center-aligned ATOM channels
 * with 1us dead-time, and a separate edge-aligned trigger channel on ATOM ch3.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Per-channel configuration: outputs, DTM, interrupt on base channel */
    /* Output routing and polarity (complementary: HS active HIGH, LS active LOW) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time config: 1 microsecond rising and falling */
    dtmConfig[0].deadTime.rising     = 1.0e-6f;
    dtmConfig[0].deadTime.falling    = 1.0e-6f;
    dtmConfig[0].fastShutOff         = NULL_PTR;

    dtmConfig[1].deadTime.rising     = 1.0e-6f;
    dtmConfig[1].deadTime.falling    = 1.0e-6f;
    dtmConfig[1].fastShutOff         = NULL_PTR;

    dtmConfig[2].deadTime.rising     = 1.0e-6f;
    dtmConfig[2].deadTime.falling    = 1.0e-6f;
    dtmConfig[2].fastShutOff         = NULL_PTR;

    /* Interrupt configuration for base channel (index 0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Channel 0 (logical) - Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig; /* base channel gets interrupt */

    /* Channel 1 (logical) - Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;

    /* Channel 2 (logical) - Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 4) Complete main configuration */
    config.cluster              = IfxEgtm_Cluster_0;
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;
    config.alignment            = IfxEgtm_Pwm_Alignment_center;
    config.numChannels          = (uint8)NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;
    config.clockSource.atom     = (uint32)IfxEgtm_Cmu_Clk_0;      /* ATOM clock source = CMU CLK0 */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled    = TRUE;
    config.syncStart            = TRUE;                            /* start channels in sync after init */

    /* 5) Enable guard: enable EGTM and clocks only if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* dynamically set EGTM CMU clocks */
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 6) Initialize unified PWM (applies duties and dead-times) */
    IfxEgtm_Pwm_init(&g_egtm3ph.pwm, g_egtm3ph.channels, &config);

    /* 7) Store initial state for runtime updates */
    g_egtm3ph.dutyCycles[0] = channelConfig[0].duty;
    g_egtm3ph.dutyCycles[1] = channelConfig[1].duty;
    g_egtm3ph.dutyCycles[2] = channelConfig[2].duty;

    g_egtm3ph.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtm3ph.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtm3ph.deadTimes[2]  = dtmConfig[2].deadTime;

    g_egtm3ph.phases[0]     = channelConfig[0].phase;
    g_egtm3ph.phases[1]     = channelConfig[1].phase;
    g_egtm3ph.phases[2]     = channelConfig[2].phase;

    /* 8) Configure separate edge-aligned trigger (ATOM logical channel 3) */
    {
        IfxEgtm_Pwm_Config        trigCfg;
        IfxEgtm_Pwm_ChannelConfig trigChCfg[1];
        IfxEgtm_Pwm_DtmConfig     trigDtm[1];
        IfxEgtm_Pwm_OutputConfig  trigOut[1];

        IfxEgtm_Pwm_initConfig(&trigCfg, &MODULE_EGTM);

        /* Edge-aligned, single channel on next logical index */
        trigOut[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_OUT; /* optional pad routing */
        trigOut[0].complementaryPin       = NULL_PTR;
        trigOut[0].polarity               = Ifx_ActiveState_high;
        trigOut[0].complementaryPolarity  = Ifx_ActiveState_low;
        trigOut[0].outputMode             = IfxPort_OutputMode_pushPull;
        trigOut[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        trigDtm[0].deadTime.rising        = 0.0f;
        trigDtm[0].deadTime.falling       = 0.0f;
        trigDtm[0].fastShutOff            = NULL_PTR;

        trigChCfg[0].timerCh              = IfxEgtm_Pwm_SubModule_Ch_3;
        trigChCfg[0].phase                = 0.0f;
        trigChCfg[0].duty                 = ADC_TRIG_DUTY;
        trigChCfg[0].dtm                  = &trigDtm[0];
        trigChCfg[0].output               = &trigOut[0];
        trigChCfg[0].mscOut               = NULL_PTR;
        trigChCfg[0].interrupt            = NULL_PTR;

        trigCfg.cluster                   = IfxEgtm_Cluster_0;
        trigCfg.subModule                 = IfxEgtm_Pwm_SubModule_atom;
        trigCfg.alignment                 = IfxEgtm_Pwm_Alignment_edge;
        trigCfg.numChannels               = 1U;
        trigCfg.channels                  = &trigChCfg[0];
        trigCfg.frequency                 = PWM_FREQUENCY;
        trigCfg.clockSource.atom          = (uint32)IfxEgtm_Cmu_Clk_0;
        trigCfg.dtmClockSource            = IfxEgtm_Dtm_ClockSource_cmuClock0;
        trigCfg.syncUpdateEnabled         = TRUE;
        trigCfg.syncStart                 = FALSE;

        IfxEgtm_Pwm_init(&g_egtmTrig.pwm, g_egtmTrig.channel, &trigCfg);
        g_egtmTrig.duty = trigChCfg[0].duty;

        /* Connect EGTM trigger to ADC trigger mux. Parameters are platform specific.
           Use ATOM0 source, channel 3, and ADC Trigger Signal 0 as placeholders. */
        {
            boolean trigOk = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                       (IfxEgtm_TrigSource)0,   /* e.g., ATOM0 source */
                                                       (IfxEgtm_TrigChannel)3,  /* channel 3 */
                                                       (IfxEgtm_Cfg_AdcTriggerSignal)0);
            if (trigOk == FALSE)
            {
                /* Application may handle routing failure if needed. */
            }
        }
    }

    /* 9) Configure LED pin as push-pull output so ISR can toggle it */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Atomically update all three PWM phase duties, wrapping at 100% to 0% then adding step.
 * No timing logic; caller is responsible for calling this periodically.
 */
void updateEgtmAtomDuty(void)
{
    /* Wrap-then-increment rule (no loops) */
    if ((g_egtm3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[0] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[1] = 0.0f; }
    if ((g_egtm3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtm3ph.dutyCycles[2] = 0.0f; }

    g_egtm3ph.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtm3ph.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtm3ph.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply update atomically while preserving alignment and dead-times */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtm3ph.pwm, (float32 *)g_egtm3ph.dutyCycles);
}
