/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-phase complementary PWM + ADC trigger on TC4xx.
 *
 * Notes:
 * - Follows iLLD init patterns and TC4xx EGTM migration (GTM->EGTM).
 * - No watchdog control here (Cpu0_Main.c is the only allowed place).
 * - Uses only documented/mock-available APIs.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD includes */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* ====================================================================== */
/* Macros (NUMERICAL VALUES from requirements and migration confirmation)  */
/* ====================================================================== */
#define NUM_OF_CHANNELS                (3u)
#define PWM_FREQUENCY                  (20000.0f)
#define PHASE_U_DUTY                  (25.0f)
#define PHASE_V_DUTY                  (50.0f)
#define PHASE_W_DUTY                  (75.0f)
#define PHASE_DUTY_STEP                (0.01f)
#define ISR_PRIORITY_ATOM              (25u) /* ADC result ISR priority per user req */

/* LED: P03.9 (compound macro to expand into (port, pin)) */
#define LED                            &MODULE_P03, 9

/* ====================================================================== */
/* Pin macros (validated or placeholders if not provided in the template)  */
/* ====================================================================== */
/* User-requested pins (validate against provided list). U phase is available; V/W are TBD. */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT (validated pin symbol) */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT (validated pin symbol) */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT (validated pin symbol) */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT (validated pin symbol) */

/* ADC trigger (ATOM0 CH3) on P33.0: placeholder until validated symbol is available */
#define ADC_TRIG_TOUT   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT when available */

/* ====================================================================== */
/* Module state                                                           */
/* ====================================================================== */
#include "Compilers.h" /* for IFX_STATIC keyword */

typedef struct
{
    IfxEgtm_Pwm                 pwm;                                /* PWM driver handle */
    IfxEgtm_Pwm_Channel         channels[NUM_OF_CHANNELS];          /* Persistent channel objects */
    float32                     dutyCycles[NUM_OF_CHANNELS];        /* Duty in percent */
    float32                     phases[NUM_OF_CHANNELS];            /* Phase in percent */
    IfxEgtm_Pwm_DeadTime        deadTimes[NUM_OF_CHANNELS];         /* Dead-time seconds */
    IfxEgtm_Atom_Timer          triggerTimer;                       /* ATOM0 CH3 trigger timer driver */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;

/* ====================================================================== */
/* Internal prototypes                                                    */
/* ====================================================================== */
static void configureEgtmAtomTrigger(uint8 channel);

/* Optional empty callback symbol to fit common patterns (not used here) */
void IfxEgtm_periodEventFunction(void *data) { (void)data; }

/* ====================================================================== */
/* ISR body required by design (toggle LED)                               */
/* ====================================================================== */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}

/* ====================================================================== */
/* Private helpers                                                        */
/* ====================================================================== */
/** Route ATOM0 CH[channel] to TOUT (if available) and connect to ADC trigger fabric. */
static void configureEgtmAtomTrigger(uint8 channel)
{
    /* Route ATOM0 CH3 to physical TOUT (if the mapping is provided) */
    if (ADC_TRIG_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ADC_TRIG_TOUT,
                                   IfxPort_OutputMode_pushPull,
                                   IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* Connect EGTM Cluster 0 ATOM0 CH3 to ADC trigger fabric.
       Note: Edge polarity / MUX selection are system-level settings not exposed
       by the available mock signature. Select source=ATOM0, channel=3, signal=0. */
    (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                    (IfxEgtm_TrigSource)0,      /* ATOM0 source */
                                    (IfxEgtm_TrigChannel)channel,
                                    (IfxEgtm_Cfg_AdcTriggerSignal)0 /* AdcTriggerSignal_0 */);
}

/* ====================================================================== */
/* Public API                                                             */
/* ====================================================================== */
/**
 * Initialize EGTM CLS0 ATOM resources for 3-phase complementary PWM (CH0..CH2) and 
 * ATOM0 CH3 as 50% edge-aligned ADC trigger source.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare local configuration structures */
    IfxEgtm_Pwm_Config         config;
    IfxEgtm_Pwm_ChannelConfig  channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig   output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig      dtmConfig[NUM_OF_CHANNELS];

    /* 2) Load defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate complementary outputs (HS active-high, LS active-low) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS; /* TBD: provide validated symbol */
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS; /* TBD: provide validated symbol */
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS; /* TBD: provide validated symbol */
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS; /* TBD: provide validated symbol */
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1.0 us rising/falling, DTM clock = CMU CLK0 */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Channel configuration: logical timerCh 0..2, center-aligned via main config */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = NULL_PTR; /* No PWM period interrupt */

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

    /* Main PWM configuration */
    config.cluster             = IfxEgtm_Cluster_0;
    config.subModule           = IfxEgtm_Pwm_SubModule_atom;
    config.alignment           = IfxEgtm_Pwm_Alignment_center;
    config.numChannels         = (uint8)NUM_OF_CHANNELS;
    config.channels            = &channelConfig[0];
    config.frequency           = PWM_FREQUENCY;
    config.clockSource.atom    = (uint32)IfxEgtm_Cmu_Clk_0;       /* Use CMU CLK0 for ATOM */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncUpdateEnabled   = TRUE;                            /* Shadow transfer at period end */
    config.syncStart           = TRUE;                            /* Start channels in sync */

    /* 6) Enable guard: enable EGTM module and required clocks (GCLK and CLK0) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified EGTM PWM (applies initial duties and dead-time) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* Persist initial state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 8) Configure ATOM0 CH3 as 50% edge-aligned trigger source */
    (void)IfxEgtm_Atom_Timer_setFrequency(&g_egtmAtom3phInv.triggerTimer, PWM_FREQUENCY);
    {
        uint32 period = IfxEgtm_Atom_Timer_getPeriod(&g_egtmAtom3phInv.triggerTimer);
        uint32 triggerPoint = period / 2u; /* 50% position */
        IfxEgtm_Atom_Timer_setTrigger(&g_egtmAtom3phInv.triggerTimer, triggerPoint);
        IfxEgtm_Atom_Timer_run(&g_egtmAtom3phInv.triggerTimer);
    }

    /* 9) Route CH3 to TOUT and connect to ADC trigger fabric */
    configureEgtmAtomTrigger(3u);

    /* 10) Configure LED GPIO as push-pull output */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three logical PWM channel duties immediately.
 * requestDuty[0..2] in percent [0..100].
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    if (d0 < 0.0f)  { d0 = 0.0f; }  else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f)  { d1 = 0.0f; }  else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f)  { d2 = 0.0f; }  else if (d2 > 100.0f) { d2 = 100.0f; }

    g_egtmAtom3phInv.dutyCycles[0] = d0;
    g_egtmAtom3phInv.dutyCycles[1] = d1;
    g_egtmAtom3phInv.dutyCycles[2] = d2;

    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_0, d0);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_1, d1);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3phInv.pwm, IfxEgtm_Pwm_SyncChannelIndex_2, d2);
}
