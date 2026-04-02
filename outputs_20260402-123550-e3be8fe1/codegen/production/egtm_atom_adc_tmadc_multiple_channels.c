/*
 * egtm_atom_adc_tmadc_multiple_channels.c
 *
 * Production driver: 3-phase center-aligned complementary PWM on EGTM ATOM0
 * with synchronized TMADC triggering and minimal ISR for debug GPIO toggle.
 *
 * Notes:
 * - Follows iLLD initialization patterns and uses high-level IfxEgtm_Pwm driver.
 * - No watchdog disable here (must be in CpuX main files only).
 * - No STM timing logic in this driver.
 */

#include "egtm_atom_adc_tmadc_multiple_channels.h"

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ========================= Macros and Configuration Constants ========================= */

/* Channel counts and PWM timing */
#define NUM_OF_CHANNELS        (3u)
#define PWM_FREQUENCY          (20000.0f)       /* 20 kHz */
#define PHASE_DUTY_STEP        (0.01f)          /* Increment step in percent */

/* Initial duty cycles in percent */
#define PHASE_U_DUTY           (25.0f)
#define PHASE_V_DUTY           (50.0f)
#define PHASE_W_DUTY           (75.0f)

/* ISR priority for PWM period event (used by unified PWM driver) */
#define ISR_PRIORITY_ATOM      (25)

/* Debug GPIO (P03.9) as compound macro (port, pin) */
#define LED                    &MODULE_P03, 9

/* ========================= Pin Routing (TOUT) Macros ========================= */
/*
 * Use ONLY validated pin symbols. The user requested pins P20.8/9, P20.10/11, P20.12/13.
 * These are not present in the validated list provided here, so assign NULL_PTR placeholders.
 * Replace NULL_PTR with the correct IfxEgtm_ATOMx_y[_N]_TOUTz_Pxx_y_OUT symbols during integration.
 */
#define PHASE_U_HS   (NULL_PTR) /* Requested: P20.8  → replace with &IfxEgtm_ATOM0_0_TOUTxx_P20_8_OUT when available */
#define PHASE_U_LS   (NULL_PTR) /* Requested: P20.9  → replace with &IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT if appropriate */
#define PHASE_V_HS   (NULL_PTR) /* Requested: P20.10 → replace with &IfxEgtm_ATOM0_1_TOUTxx_P20_10_OUT when available */
#define PHASE_V_LS   (NULL_PTR) /* Requested: P20.11 → replace with &IfxEgtm_ATOM0_1N_TOUTxx_P20_11_OUT when available */
#define PHASE_W_HS   (NULL_PTR) /* Requested: P20.12 → replace with &IfxEgtm_ATOM0_2_TOUTxx_P20_12_OUT when available */
#define PHASE_W_LS   (NULL_PTR) /* Requested: P20.13 → replace with &IfxEgtm_ATOM0_2N_TOUTxx_P20_13_OUT when available */

/* ========================= Module State ========================= */

typedef struct
{
    IfxEgtm_Pwm           pwm;                                     /* Unified PWM handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];               /* Persistent channel handles */
    float32               dutyCycles[NUM_OF_CHANNELS];             /* Duty in percent */
    float32               phases[NUM_OF_CHANNELS];                 /* Phase in percent/deg units per config */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];              /* Per-phase dead-time parameters */
} EgtmAtomState;

IFX_STATIC EgtmAtomState g_egtmAtomState;

/* ========================= ISR and Callback Declarations ========================= */
/* PWM period ISR stub (installed by unified driver via InterruptConfig). Body must be minimal. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* Unified PWM driver callback for period event (must exist, do nothing). */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ========================= Public API Implementations ========================= */

void initEgtmAtom3phInv(void)
{
    /* Step 1: Declare all configuration structs as locals */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* Step 2: Initialize main PWM config with defaults using module handle */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Step 3: Configure outputs for complementary pairs (top/bottom) */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;    /* High-side active high */
    output[0].complementaryPolarity = Ifx_ActiveState_low;     /* Low-side active low  */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Step 4: Configure DTM dead-time (1 us rising, 1 us falling) */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* Step 5: Interrupt configuration for base channel period event */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = (Ifx_Priority)ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Step 6: Per-channel configuration (logical indices 0..2) */
    /* Channel 0 → Phase U */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig; /* Base channel carries interrupt */

    /* Channel 1 → Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;         /* no ISR on this channel */

    /* Channel 2 → Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;         /* no ISR on this channel */

    /* Step 7: Main configuration fields */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;
    config.cluster           = IfxEgtm_Cluster_0;
    config.alignment         = IfxEgtm_Pwm_Alignment_center;
    config.numChannels       = (uint8)NUM_OF_CHANNELS;
    config.channels          = &channelConfig[0];
    config.frequency         = PWM_FREQUENCY;
    config.clockSource.atom  = (uint32)IfxEgtm_Cmu_Clk_0;            /* ATOM clock source → CMU CLK0 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;    /* DTM driven from CMU CLK0 */
#if IFXEGTM_PWM_IS_HIGH_RES_AVAILABLE
    config.highResEnable     = FALSE;
    config.dtmHighResEnable  = FALSE;
#endif
    config.syncUpdateEnabled = TRUE;
    config.syncStart         = TRUE;

    /* Step 7 (continued): Enable guard for EGTM CMU clocks */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Dynamically match GCLK and CLK0 to module frequency (1:1 div) */
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* Step 8: Initialize unified PWM (applies duties, alignment, outputs, dead-time, and IRQ) */
    IfxEgtm_Pwm_init(&g_egtmAtomState.pwm, &g_egtmAtomState.channels[0], &config);

    /* Step 9: Store initial state (duties, phases, dead-times) into persistent module state */
    g_egtmAtomState.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtomState.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtomState.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtomState.phases[0] = channelConfig[0].phase;
    g_egtmAtomState.phases[1] = channelConfig[1].phase;
    g_egtmAtomState.phases[2] = channelConfig[2].phase;

    g_egtmAtomState.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtomState.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtomState.deadTimes[2] = dtmConfig[2].deadTime;

    /* Step 10: Configure EGTM → ADC trigger route (ATOM0 CH3 → TMADC trigger signal 0) */
    {
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                   IfxEgtm_TrigSource_atom0,   /* ATOM0 source */
                                                   IfxEgtm_TrigChannel_3,      /* Channel 3 as trigger */
                                                   IfxEgtm_Cfg_AdcTriggerSignal_0);
        (void)trigOk; /* In production, check and handle error if FALSE */
    }

    /* Step 11: Initialize and start TMADC (module + channels + enable result event + run) */
    {
        IfxAdc_Tmadc        tmadc;
        IfxAdc_Tmadc_Config tmadcConfig;
        IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&tmadc, &tmadcConfig);

        /* Configure 5 channels: CH0..CH4 */
        {
            IfxAdc_Tmadc_Ch       ch[5];
            IfxAdc_Tmadc_ChConfig chCfg;
            uint8 i;
            for (i = 0; i < 5u; ++i)
            {
                IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
                /* Channel-specific selections (channel index, one-shot, SAR Core 0, 100ns sampling)
                 * are applied via default config or device configuration in integration. */
                IfxAdc_Tmadc_initChannel(&ch[i], &chCfg);
                IfxAdc_Tmadc_enableChannelEvent(&ch[i], IfxAdc_TmadcServReq_sr0, IfxAdc_TmadcEventSel_resultReady);
            }
        }

        IfxAdc_Tmadc_runModule(&tmadc);
    }

    /* Step 12: Configure debug GPIO as output push-pull (ISR toggles it) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy requested duty values (percent) directly into module state and update atomically */
    g_egtmAtomState.dutyCycles[0] = requestDuty[0];
    g_egtmAtomState.dutyCycles[1] = requestDuty[1];
    g_egtmAtomState.dutyCycles[2] = requestDuty[2];

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.pwm, (float32 *)g_egtmAtomState.dutyCycles);
}

void updateEgtmAtomDuty(void)
{
    /* Incrementally adjust duties with wrap rule, then update atomically */
    if ((g_egtmAtomState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtomState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtomState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtomState.dutyCycles[2] = 0.0f; }

    g_egtmAtomState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtomState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtomState.dutyCycles[2] += PHASE_DUTY_STEP;

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtomState.pwm, (float32 *)g_egtmAtomState.dutyCycles);
}

void resultISR(void)
{
    /* Minimal ISR: toggle the configured debug GPIO and return. */
    IfxPort_togglePin(LED);
}
