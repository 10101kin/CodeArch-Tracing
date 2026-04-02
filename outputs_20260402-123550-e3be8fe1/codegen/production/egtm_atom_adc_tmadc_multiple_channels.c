/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver for TC4xx EGTM ATOM 3-phase complementary PWM with
 * synchronized TMADC trigger. Follows iLLD unified IfxEgtm_Pwm patterns.
 *
 * Mandatory constraints satisfied:
 * - Uses IfxEgtm_Pwm unified driver with OutputConfig/ChannelConfig/DTM arrays
 * - Enables EGTM clocks via enable guard pattern (GCLK and CLK0)
 * - No watchdog calls here (only allowed in CpuX_Main.c)
 * - No STM usage; no manual SRC setup for PWM
 * - Uses validated pin symbols only; unknown pins set to NULL_PTR with notes
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"

#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros (numeric constants) ====================== */
#define EGTM_NUM_CHANNELS              ((uint8)3u)
#define TMADC_NUM_CHANNELS             ((uint8)5u)

#define PWM_FREQUENCY_HZ               (20000.0f)

#define PHASE_U_DUTY_INIT              (25.0f)
#define PHASE_V_DUTY_INIT              (50.0f)
#define PHASE_W_DUTY_INIT              (75.0f)

#define PHASE_DUTY_STEP                (0.01f)

#define ISR_PRIORITY_ATOM              ((Ifx_Priority)25)

/* Debug GPIO (ISR toggles this pin). Compound form to pass as two args */
#define LED                            &MODULE_P03, 9

/* ========================= Validated EGTM ATOM TOUT pin macros ============ */
/*
 * Only use validated symbols listed by the integration environment.
 * Unknown required pins are set to NULL_PTR and must be mapped during board
 * integration using the proper IfxEgtm_*_PinMap symbol for the given pad.
 */

/* Phase U: P20.8 (HS) / P20.9 (LS) */
#define PHASE_U_HS   (NULL_PTR)                           /* TODO: Map P20.8 to a valid EGTM ATOM0 TOUT symbol */
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT) /* Validated: complementary (N) output at P20.9 */

/* Phase V: P20.10 (HS) / P20.11 (LS) */
#define PHASE_V_HS   (NULL_PTR)                           /* TODO: Map P20.10 to a valid EGTM ATOM0 TOUT symbol */
#define PHASE_V_LS   (NULL_PTR)                           /* TODO: Map P20.11 to a valid EGTM ATOM0 TOUT symbol */

/* Phase W: P20.12 (HS) / P20.13 (LS) */
#define PHASE_W_HS   (NULL_PTR)                           /* TODO: Map P20.12 to a valid EGTM ATOM0 TOUT symbol */
#define PHASE_W_LS   (NULL_PTR)                           /* TODO: Map P20.13 to a valid EGTM ATOM0 TOUT symbol */

/* ========================= Module persistent state ======================== */

typedef struct
{
    IfxEgtm_Pwm             pwm;                                   /* unified PWM driver handle */
    IfxEgtm_Pwm_Channel     channels[EGTM_NUM_CHANNELS];           /* persistent channel handles */
    float32                 dutyCycles[EGTM_NUM_CHANNELS];          /* duty (%) per logical channel */
    float32                 phases[EGTM_NUM_CHANNELS];              /* phase (deg or %) if used */
    IfxEgtm_Pwm_DeadTime    deadTimes[EGTM_NUM_CHANNELS];           /* per-channel dead-times (s) */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph = {0};

/* ========================= Internal callback and ISR ====================== */

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data; /* Intentionally empty callback per design */
}

IFX_INTERRUPT(resultISR, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* ========================= Public API implementations ===================== */

void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration objects (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[EGTM_NUM_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Initialize main PWM config with defaults for the module */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure output[] for complementary PWM (HS active-high, LS active-low) */
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

    /* 4) Configure DTM dead-time: 1 us rising and 1 us falling */
    dtmConfig[0].deadTime.rising = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR;

    dtmConfig[1].deadTime.rising = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Configure interrupt for base channel period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Fill channel configs: logical indices 0..2 with duties 25/50/75%, phase=0 */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY_INIT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;   /* only base channel has interrupt */

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY_INIT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY_INIT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM configuration */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.cluster            = IfxEgtm_Cluster_0;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.numChannels        = EGTM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;          /* ATOM uses CMU Clk_0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_systemClock; /* DTM clock source */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE
    config.highResEnable      = FALSE;
    config.dtmHighResEnable   = FALSE;
#endif
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;

    /* 7) Enable guard: enable EGTM and setup clocks if not already enabled */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize unified PWM (applies duties, outputs, alignment, DTM, and ISR) */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwm, g_egtmAtom3ph.channels, &config);

    /* 9) Store initial duty and dead-time into persistent state */
    g_egtmAtom3ph.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3ph.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3ph.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3ph.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3ph.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3ph.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 10) Configure EGTM -> ADC trigger route using trigger API
     * Note: Exact enums for source/channel/signal depend on device configuration.
     * Here we target ATOM0 channel 3 and ADC Trigger Signal 0 per requirements.
     */
    {
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(
            IfxEgtm_Cluster_0,
            (IfxEgtm_TrigSource)0,      /* Device-specific: select EGTM ATOM source */
            (IfxEgtm_TrigChannel)3,     /* ATOM0 CH3 used for ADC trigger */
            (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */
        );
        (void)trigOk; /* In production, handle error if FALSE */
    }

    /* 11) Initialize and run TMADC (module + channels). Details depend on SoC config. */
    {
        IfxAdc_Tmadc         tmadc;
        IfxAdc_Tmadc_Config  tmadcConfig;
        IfxAdc_Tmadc_Ch      tmadcCh[TMADC_NUM_CHANNELS];
        IfxAdc_Tmadc_ChConfig chCfg;

        IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&tmadc, &tmadcConfig);

        for (uint8 i = 0; i < TMADC_NUM_CHANNELS; ++i)
        {
            IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
            /* Channel-specific configuration (group, input, sampling time, trigger, etc.)
               should be set here as per board design. */
            IfxAdc_Tmadc_initChannel(&tmadcCh[i], &chCfg);
            /* Enable channel event for result notification (service request 0, result-ready) */
            IfxAdc_Tmadc_enableChannelEvent(&tmadcCh[i], (IfxAdc_TmadcServReq)0, (IfxAdc_TmadcEventSel)0);
        }

        IfxAdc_Tmadc_runModule(&tmadc);
    }

    /* 12) Configure debug GPIO as output push-pull (ISR toggles this) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy requested duties (%) to state without scaling */
    g_egtmAtom3ph.dutyCycles[0] = requestDuty[0];
    g_egtmAtom3ph.dutyCycles[1] = requestDuty[1];
    g_egtmAtom3ph.dutyCycles[2] = requestDuty[2];

    /* Apply all three duties atomically */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwm, (float32 *)g_egtmAtom3ph.dutyCycles);
}

void updateEgtmAtomDuty(void)
{
    /* Duty wrap rule: check+reset then unconditional add; no loops */
    if ((g_egtmAtom3ph.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3ph.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3ph.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3ph.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3ph.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3ph.dutyCycles[2] = 0.0f; }

    g_egtmAtom3ph.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3ph.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3ph.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply synchronously (immediate) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3ph.pwm, (float32 *)g_egtmAtom3ph.dutyCycles);
}
