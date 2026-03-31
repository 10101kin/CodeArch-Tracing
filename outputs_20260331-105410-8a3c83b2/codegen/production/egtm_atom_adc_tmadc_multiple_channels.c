/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production-ready EGTM ATOM PWM driver for TC4xx (KIT_A3G_TC4D7_LITE).
 * Implements:
 *  - 3-phase complementary center-aligned inverter on ATOM CH0..2 @ 20 kHz
 *    with 1 us dead-time (rising/falling) using IfxEgtm_Pwm unified driver
 *  - Single edge-aligned 50% PWM on ATOM CH3 as TMADC trigger source
 *
 * Migration: TC3xx -> TC4xx APIs
 *  IfxGtm_*  -> IfxEgtm_*
 *  IfxScuWdt -> IfxWtu (watchdog handling ONLY in CpuX_Main.c; not here)
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* Unified EGTM PWM high-level driver */
#include "IfxEgtm_Pwm.h"

/* Port driver for LED GPIO */
#include "IfxPort.h"

/* ======================================================================
 * Configuration macros (numeric values from requirements and migration)
 * ====================================================================== */
#define EGTM_INV_NUM_OF_CHANNELS            (3u)
#define EGTM_ADC_TRIG_NUM_OF_CHANNELS       (1u)

#define EGTM_PWM_FREQUENCY_HZ               (20000.0f)
#define PHASE_U_DUTY_INIT_PERCENT           (25.0f)
#define PHASE_V_DUTY_INIT_PERCENT           (50.0f)
#define PHASE_W_DUTY_INIT_PERCENT           (75.0f)
#define PHASE_DUTY_STEP_PERCENT             (0.01f)

/* Interrupt priority for EGTM ATOM (used by ISR and InterruptConfig) */
#define ISR_PRIORITY_ATOM                   (25)

/* LED: P03.9 (compound macro: &MODULE_Pxx, pinIndex) */
#define LED                                 &MODULE_P03, 9

/* ======================================================================
 * Pin routing (use validated/user-requested EGTM TOUT symbols)
 * Routing is performed by unified driver via OutputConfig; no PinMap calls.
 * ====================================================================== */
#define PHASE_U_HS                          (&IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT)
#define PHASE_U_LS                          (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS                          (&IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT)
#define PHASE_V_LS                          (&IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT)
#define PHASE_W_HS                          (&IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT)
#define PHASE_W_LS                          (&IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT)
#define ADC_TRIG_PIN                        (&IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT)

/* ======================================================================
 * Module persistent state (IFX_STATIC as per AURIX style)
 * ====================================================================== */

typedef struct
{
    IfxEgtm_Pwm               pwm;                                        /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_INV_NUM_OF_CHANNELS];         /* persistent channel handles */
    float32                   dutyCycles[EGTM_INV_NUM_OF_CHANNELS];       /* percent */
    float32                   phases[EGTM_INV_NUM_OF_CHANNELS];           /* phase offset in percent */
    IfxEgtm_Pwm_DeadTime      deadTimes[EGTM_INV_NUM_OF_CHANNELS];        /* per-channel dead-times */
} EgtmAtom3phInv_State;

typedef struct
{
    IfxEgtm_Pwm               pwm;                                        /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel       channels[EGTM_ADC_TRIG_NUM_OF_CHANNELS];    /* persistent channel handle */
    float32                   dutyCycles[EGTM_ADC_TRIG_NUM_OF_CHANNELS];  /* percent */
} EgtmAtomAdcTrig_State;

/* IFX_STATIC module instances */
IFX_STATIC EgtmAtom3phInv_State   g_egtmAtom3phInv;
IFX_STATIC EgtmAtomAdcTrig_State  g_egtmAtomAdcTrig;

/* ======================================================================
 * ISR and callback (declared BEFORE init functions)
 * ====================================================================== */

/* Debug ISR for EGTM ATOM: toggle status LED (minimal ISR body) */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Period-event callback (assigned via InterruptConfig; empty body) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ======================================================================
 * Internal helpers
 * ====================================================================== */

/* Enable-guard: strictly follow authoritative CMU enable pattern. */
static void egtm_enableGuard(void)
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

/* ======================================================================
 * Public API implementations
 * ====================================================================== */

/**
 * Initialize a 3-channel complementary, center-aligned PWM inverter on
 * EGTM Cluster 0 ATOM channels 0..2 at 20 kHz with 1 us dead-time on both
 * edges using the unified high-level PWM driver.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local config structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_INV_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_INV_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_INV_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* 2) Initialize main PWM config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output pin configurations (complementary) */
    /* Phase U */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us on both edges (use 1e-6f literal) */
    dtmConfig[0].deadTime.rising = 1.0e-6f; dtmConfig[0].deadTime.falling = 1.0e-6f; dtmConfig[0].fastShutOff = NULL_PTR;
    dtmConfig[1].deadTime.rising = 1.0e-6f; dtmConfig[1].deadTime.falling = 1.0e-6f; dtmConfig[1].fastShutOff = NULL_PTR;
    dtmConfig[2].deadTime.rising = 1.0e-6f; dtmConfig[2].deadTime.falling = 1.0e-6f; dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration (period event on base channel) */
    irqCfg.mode        = (IfxEgtm_IrqMode)0;      /* pulse notify (implementation-defined 0) */
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.vmId        = IfxSrc_VmId_0;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 6) Channel configurations: CH0..2 (logical indices) */
    /* Phase U on logical channel 0 */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;      /* base channel owns the ISR */

    /* Phase V on logical channel 1 */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Phase W on logical channel 2 */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;
    config.syncUpdateEnabled  = TRUE;
    config.numChannels        = (uint8)EGTM_INV_NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0; /* set ONLY .atom for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;

    /* 8) Enable-guard and CMU clock setup */
    egtm_enableGuard();

    /* 9) Initialize the unified PWM once, using persistent channels array */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 10) Store initial duties, phases, and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure status LED GPIO as output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Runtime duty update for the 3-phase inverter.
 * Copies requestDuty[0..2] into persistent state and applies immediately.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy new requests (percent, no scaling) */
    g_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately to all configured channels using array-based API */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}

/**
 * Configure a single edge-aligned 50% PWM on EGTM ATOM channel 3 to act as
 * a TMADC trigger source (output pin routed via unified driver mapping).
 */
void initEgtmAtomAdcTrigger(void)
{
    /* 1) Local config: main PWM, single channel config, and output config */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_ADC_TRIG_NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_ADC_TRIG_NUM_OF_CHANNELS];

    /* 2) Initialize config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Output mapping to trigger line (P33.0) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)ADC_TRIG_PIN;
    output[0].complementaryPin       = NULL_PTR; /* single-pin output */
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low; /* unused */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) Single channel @ CH3, 50% duty, edge-aligned */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_3;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = 50.0f; /* 50% */
    channelConfig[0].dtm       = NULL_PTR;
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR; /* no interrupt for trigger channel */

    /* Main config fields */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_edge;
    config.syncStart          = FALSE;
    config.syncUpdateEnabled  = FALSE;
    config.numChannels        = (uint8)EGTM_ADC_TRIG_NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0; /* set ONLY .atom for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock;      /* not used without DTM */

    /* 4) Ensure EGTM clocking is enabled (guarded) */
    egtm_enableGuard();

    /* 5) Initialize the PWM for the trigger channel */
    IfxEgtm_Pwm_init(&g_egtmAtomAdcTrig.pwm, g_egtmAtomAdcTrig.channels, &config);

    /* Store initial duty into persistent state */
    g_egtmAtomAdcTrig.dutyCycles[0] = channelConfig[0].duty;
}
