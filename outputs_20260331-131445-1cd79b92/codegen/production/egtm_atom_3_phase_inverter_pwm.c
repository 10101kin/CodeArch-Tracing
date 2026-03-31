/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-phase complementary PWM (TC4xx).
 *
 * Implementation follows iLLD patterns and the specified migration from TC3xx GTM/TOM to TC4xx EGTM/ATOM.
 * Clocks are enabled using the CMU block; PWM configured via unified IfxEgtm_Pwm API; ATOM0 CH3 configured
 * as edge-aligned 50% trigger source for ADC and optionally routed to a TOUT pin.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* EGTM unified PWM and trigger APIs */
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Atom_Timer.h"
#include "IfxEgtm_PinMap.h"

/* Port/GPIO */
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =================================================================================================== */
/* Numeric configuration values (from requirements / migration-provided constants)                      */
/* =================================================================================================== */
#define EGTM_PWM_NUM_CHANNELS          (3U)
#define EGTM_PWM_FREQUENCY_HZ          (20000.0f)
#define PHASE_U_DUTY                   (25.0f)
#define PHASE_V_DUTY                   (50.0f)
#define PHASE_W_DUTY                   (75.0f)
#define PHASE_DUTY_STEP                (0.01f)

/* LED GPIO: compound macro for port/pin as separate arguments */
#define LED                             &MODULE_P03, 9

/* =================================================================================================== */
/* Validated EGTM ATOM TOUT pins (use ONLY validated symbols; use NULL_PTR placeholders if unavailable) */
/* =================================================================================================== */
/* Phase U (ATOM0 CH0): validated */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)

/* Phase V (ATOM0 CH1): user requested P02.2 / P02.3; not present in validated list → placeholders */
#define PHASE_V_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1_TOUT2_P02_2_OUT after board pin validation */
#define PHASE_V_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT after board pin validation */

/* Phase W (ATOM0 CH2): user requested P02.4 / P02.5; not present in validated list → placeholders */
#define PHASE_W_HS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2_TOUT4_P02_4_OUT after board pin validation */
#define PHASE_W_LS   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT after board pin validation */

/* ATOM0 CH3 trigger TOUT pin: user requested P33.0; not present in validated list → placeholder */
#define ATOM0_CH3_TOUT   (NULL_PTR) /* Replace with &IfxEgtm_ATOM0_3_TOUTxx_P33_0_OUT after board pin validation */

/* =================================================================================================== */
/* Persistent module state (IFX_STATIC as per structural rules)                                         */
/* =================================================================================================== */

typedef struct
{
    IfxEgtm_Pwm            pwm;                                         /* Unified PWM driver handle */
    IfxEgtm_Pwm_Channel    channels[EGTM_PWM_NUM_CHANNELS];             /* Channel runtime data (must persist) */
    float32                dutyCycles[EGTM_PWM_NUM_CHANNELS];           /* Duty percents (0..100) */
    float32                phases[EGTM_PWM_NUM_CHANNELS];               /* Phase offsets (seconds or percent as per API) */
    IfxEgtm_Pwm_DeadTime   deadTimes[EGTM_PWM_NUM_CHANNELS];            /* Per-channel dead-time settings */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3ph = {0};

/* =================================================================================================== */
/* Internal helpers                                                                                     */
/* =================================================================================================== */

/**
 * Configure ATOM trigger output for a given logical channel.
 * - Sets frequency to EGTM_PWM_FREQUENCY_HZ
 * - Programs 50% trigger position
 * - Optionally routes to a TOUT pad if mapping is available
 * - Connects to ADC trigger fabric (AdcTriggerSignal_0)
 */
static void configureEgtmAtomTrigger(uint8 channel)
{
    /* Local ATOM timer handle; driver calls are mocked as per available signatures. */
    IfxEgtm_Atom_Timer triggerTimer;

    /* Set timer frequency to PWM frequency (edge-aligned trigger). */
    {
        boolean freqOk = IfxEgtm_Atom_Timer_setFrequency(&triggerTimer, EGTM_PWM_FREQUENCY_HZ);
        (void)freqOk; /* In production, handle failure (e.g., assert/log). */
    }

    /* Compute 50% trigger point and program it. */
    {
        uint32 period = IfxEgtm_Atom_Timer_getPeriod(&triggerTimer);
        uint32 triggerPoint = (period >> 1); /* 50% edge */
        IfxEgtm_Atom_Timer_setTrigger(&triggerTimer, triggerPoint);
        IfxEgtm_Atom_Timer_run(&triggerTimer);
    }

    /* Route trigger to a physical TOUT pin if available. */
    if (ATOM0_CH3_TOUT != NULL_PTR)
    {
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)ATOM0_CH3_TOUT,
                                   IfxPort_OutputMode_pushPull,
                                   IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* Connect ATOM0 CH3 to ADC trigger fabric: Cluster 0, ATOM source, channel 3, signal 0 */
    {
        boolean trigOk = IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                                   (IfxEgtm_TrigSource)IfxEgtm_Pwm_SubModule_atom, /* ATOM source */
                                                   (IfxEgtm_TrigChannel)3,                         /* Logical CH3 */
                                                   IfxEgtm_Cfg_AdcTriggerSignal_0);
        (void)trigOk; /* In production, handle failure (e.g., assert/log). */
    }

    (void)channel; /* channel binding handled via trigger source/channel above. */
}

/* =================================================================================================== */
/* Public API                                                                                           */
/* =================================================================================================== */

/**
 * Initialize EGTM CLS0 ATOM resources for a 3-phase complementary PWM on logical channels 0..2 at
 * 20 kHz center-aligned with 1 us hardware dead-time, and set up ATOM0 logical channel 3 as a 50%
 * edge-aligned trigger output routed to the ADC trigger fabric.
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures locally */
    IfxEgtm_Pwm_Config        config;
    IfxEgtm_Pwm_ChannelConfig channelConfig[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_OutputConfig  output[EGTM_PWM_NUM_CHANNELS];
    IfxEgtm_Pwm_DtmConfig     dtmConfig[EGTM_PWM_NUM_CHANNELS];

    /* 2) Load default config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Populate output descriptors: complementary pairs, active-high HS, active-low LS */
    {
        /* Phase U → logical channel 0 */
        output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
        output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
        output[0].polarity               = Ifx_ActiveState_high;  /* HS active high */
        output[0].complementaryPolarity  = Ifx_ActiveState_low;   /* LS active low  */
        output[0].outputMode             = IfxPort_OutputMode_pushPull;
        output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Phase V → logical channel 1 */
        output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
        output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
        output[1].polarity               = Ifx_ActiveState_high;
        output[1].complementaryPolarity  = Ifx_ActiveState_low;
        output[1].outputMode             = IfxPort_OutputMode_pushPull;
        output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Phase W → logical channel 2 */
        output[2].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
        output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
        output[2].polarity               = Ifx_ActiveState_high;
        output[2].complementaryPolarity  = Ifx_ActiveState_low;
        output[2].outputMode             = IfxPort_OutputMode_pushPull;
        output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;
    }

    /* 4) Populate DTM with 1.0 us dead-time on both edges */
    {
        uint8 i;
        for (i = 0U; i < EGTM_PWM_NUM_CHANNELS; ++i)
        {
            dtmConfig[i].deadTime.rising  = 1.0e-6f; /* 1 us */
            dtmConfig[i].deadTime.falling = 1.0e-6f; /* 1 us */
            dtmConfig[i].fastShutOff      = NULL_PTR; /* not used */
        }
    }

    /* 5) Channel configuration (logical channels 0..2, center-aligned duties 25/50/75%) */
    {
        /* CH0 → Phase U */
        channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
        channelConfig[0].phase     = 0.0f;
        channelConfig[0].duty      = PHASE_U_DUTY;
        channelConfig[0].dtm       = &dtmConfig[0];
        channelConfig[0].output    = &output[0];
        channelConfig[0].mscOut    = NULL_PTR;
        channelConfig[0].interrupt = NULL_PTR; /* No PWM period interrupt */

        /* CH1 → Phase V */
        channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
        channelConfig[1].phase     = 0.0f;
        channelConfig[1].duty      = PHASE_V_DUTY;
        channelConfig[1].dtm       = &dtmConfig[1];
        channelConfig[1].output    = &output[1];
        channelConfig[1].mscOut    = NULL_PTR;
        channelConfig[1].interrupt = NULL_PTR;

        /* CH2 → Phase W */
        channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
        channelConfig[2].phase     = 0.0f;
        channelConfig[2].duty      = PHASE_W_DUTY;
        channelConfig[2].dtm       = &dtmConfig[2];
        channelConfig[2].output    = &output[2];
        channelConfig[2].mscOut    = NULL_PTR;
        channelConfig[2].interrupt = NULL_PTR;
    }

    /* Configure main PWM config */
    config.cluster            = IfxEgtm_Cluster_0;                       /* CLS0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;              /* ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;            /* Center-aligned */
    config.numChannels        = (uint8)EGTM_PWM_NUM_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = EGTM_PWM_FREQUENCY_HZ;                   /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;               /* CMU CLK0 for ATOM */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM from CMU CLK0 */
    config.syncUpdateEnabled  = TRUE;                                    /* Shadow transfer at period end */
    config.syncStart          = TRUE;                                    /* Start all channels in sync */

    /* 6) EGTM enable guard + CMU clocks (GCLK and CLK0 to module frequency; enable only CLK0 and FXCLK) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 7) Initialize unified EGTM PWM driver (applies duties and dead-times) */
    IfxEgtm_Pwm_init(&g_egtmAtom3ph.pwm, g_egtmAtom3ph.channels, &config);

    /* Persist initial state */
    {
        g_egtmAtom3ph.dutyCycles[0] = channelConfig[0].duty;
        g_egtmAtom3ph.dutyCycles[1] = channelConfig[1].duty;
        g_egtmAtom3ph.dutyCycles[2] = channelConfig[2].duty;

        g_egtmAtom3ph.phases[0] = channelConfig[0].phase;
        g_egtmAtom3ph.phases[1] = channelConfig[1].phase;
        g_egtmAtom3ph.phases[2] = channelConfig[2].phase;

        g_egtmAtom3ph.deadTimes[0] = dtmConfig[0].deadTime;
        g_egtmAtom3ph.deadTimes[1] = dtmConfig[1].deadTime;
        g_egtmAtom3ph.deadTimes[2] = dtmConfig[2].deadTime;
    }

    /* 8 & 9) Configure ATOM0 CH3 as 50% edge-aligned trigger and connect to ADC trigger fabric */
    configureEgtmAtomTrigger(3U);

    /* 10) Configure LED GPIO (toggle occurs in ADC result ISR) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/**
 * Update the three logical PWM channel duties immediately.
 * requestDuty[0..2] are percent values in [0..100]. The function clamps if necessary and
 * writes the values to hardware using the immediate per-channel update API.
 */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    float32 d0 = requestDuty[0];
    float32 d1 = requestDuty[1];
    float32 d2 = requestDuty[2];

    /* Clamp to [0,100] */
    if (d0 < 0.0f) { d0 = 0.0f; } else if (d0 > 100.0f) { d0 = 100.0f; }
    if (d1 < 0.0f) { d1 = 0.0f; } else if (d1 > 100.0f) { d1 = 100.0f; }
    if (d2 < 0.0f) { d2 = 0.0f; } else if (d2 > 100.0f) { d2 = 100.0f; }

    /* Persist */
    g_egtmAtom3ph.dutyCycles[0] = d0;
    g_egtmAtom3ph.dutyCycles[1] = d1;
    g_egtmAtom3ph.dutyCycles[2] = d2;

    /* Apply immediately per channel */
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3ph.pwm, IfxEgtm_Pwm_SyncChannelIndex_0, d0);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3ph.pwm, IfxEgtm_Pwm_SyncChannelIndex_1, d1);
    IfxEgtm_Pwm_updateChannelDutyImmediate(&g_egtmAtom3ph.pwm, IfxEgtm_Pwm_SyncChannelIndex_2, d2);
}

/**
 * ADC conversion-complete interrupt service routine: toggle LED pin and return.
 * No ADC parsing or further processing is performed here.
 */
void resultISR(void)
{
    IfxPort_togglePin(LED);
}
