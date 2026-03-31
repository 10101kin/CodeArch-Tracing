/**
 * @file egtm_atom_3_phase_inverter_pwm.c
 * @brief Production driver for EGTM ATOM 3‑phase complementary PWM (TC4xx)
 *
 * Implements initialization of EGTM ATOM0/Cluster 0 using IfxEgtm_Pwm high‑level driver.
 * - 3 phases (U/V/W), complementary outputs, 20 kHz, center‑aligned, 1 µs dead‑time (rising/falling)
 * - Initial duties: U=25%, V=50%, W=75%
 * - Sync start and sync update enabled
 * - Fxclk_0 clock domain
 * - Period event interrupt on base channel (CH0) with priority 20 on CPU0
 * - ISR toggles LED P03.9 (active‑low)
 *
 * Thread-safety: Initialization intended to be called once at startup before concurrency begins.
 * Error handling: All critical init steps are validated where applicable.
 */

/* Own public header */
#include "egtm_atom_3_phase_inverter_pwm.h"

/* iLLD core types */
#include "Ifx_Types.h"

/* High-level EGTM PWM driver */
#include "IfxEgtm_Pwm.h"

/* Port driver */
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* EGTM base/CMU control (transitively included by PWM header in real iLLD; included here for clarity) */
/* Note: According to integration, IfxEgtm_Pwm.h typically brings EGTM/CMU deps; mock build provides symbols. */
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"

/* =========================================================================================
 * Configuration Macros (values from requirements / user-confirmed migration values)
 * ========================================================================================= */
#define NUM_OF_CHANNELS          (3)
#define PWM_FREQUENCY            (20000.0f)
#define ISR_PRIORITY_ATOM        (20)
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)
#define WAIT_TIME                (500.0f)

/* =========================================================================================
 * Pin Macros (validated symbols only; use NULL_PTR placeholders when unavailable)
 * ========================================================================================= */
/*
 * User-requested pins:
 *   U_HS: P20.8  (TOUT64)  → not present in validated list → use NULL_PTR placeholder
 *   U_LS: P20.9  (TOUT65)  → present as ATOM0_0N_TOUT65_P20_9_OUT
 *   V_HS: P21.4  (TOUT55)  → not present in validated list → use NULL_PTR placeholder
 *   V_LS: P20.11 (TOUT67)  → not present in validated list → use NULL_PTR placeholder
 *   W_HS: P20.12 (TOUT68)  → not present in validated list → use NULL_PTR placeholder
 *   W_LS: P20.13 (TOUT69)  → not present in validated list → use NULL_PTR placeholder
 */
#define PHASE_U_HS               (NULL_PTR) /* Replace with validated symbol for TOUT64 on P20_8 when available */
#define PHASE_U_LS               (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)
#define PHASE_V_HS               (NULL_PTR) /* Replace with validated symbol for TOUT55 on P21_4 when available */
#define PHASE_V_LS               (NULL_PTR) /* Replace with validated symbol for TOUT67 on P20_11 when available */
#define PHASE_W_HS               (NULL_PTR) /* Replace with validated symbol for TOUT68 on P20_12 when available */
#define PHASE_W_LS               (NULL_PTR) /* Replace with validated symbol for TOUT69 on P20_13 when available */

/* LED: P03.9, active-low. Use compound macro for (port, pin) arguments. */
#define LED                      &MODULE_P03, 9

/* =========================================================================================
 * Module State
 * ========================================================================================= */
/**
 * @brief Internal persistent state for the EGTM ATOM 3‑phase inverter PWM.
 */
typedef struct
{
    IfxEgtm_Pwm              pwm;                                      /* Driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];                /* Persistent channel runtime data */
    float32                  dutyCycles[NUM_OF_CHANNELS];              /* Duty in percent (0..100) */
    float32                  phases[NUM_OF_CHANNELS];                  /* Phase offsets in seconds or fraction as per driver */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];               /* Dead time per channel */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv; /* IFX_STATIC is the mandated storage qualifier */

/* =========================================================================================
 * ISR and Callback (declared BEFORE init function per structural rules)
 * ========================================================================================= */

/*
 * Period-event ISR for ATOM base channel (priority: ISR_PRIORITY_ATOM, provider: CPU0).
 * Body MUST be minimal: toggle LED only. Do not call driver handlers here.
 */
IFX_INTERRUPT(EgtmAtomIsr, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/*
 * Period-event callback assigned via InterruptConfig.periodEvent.
 * MUST be externally visible (no 'static'), accept void* data, and have an EMPTY body.
 */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =========================================================================================
 * Initialization
 * ========================================================================================= */
/**
 * @brief Initialize EGTM ATOM0 Cluster 0 to generate a 3‑phase complementary, center‑aligned PWM.
 *
 * Behavior:
 * - Declares local config structures (main, channels[3], dtm[3], interrupt, outputs[3])
 * - Initializes main config with IfxEgtm_Pwm_initConfig
 * - Populates output[0..2] with complementary pairs and polarities
 * - Sets dtmConfig[0..2] with 1e‑6 s dead‑time for rising/falling
 * - Configures interrupt on base channel (channel 0) for period event
 * - Fills channelConfig[0..2] with CH0..CH2 and phase/duty/mscOut/interrupt
 * - Completes main config: cluster=0, subModule=ATOM, center‑aligned, freq=20 kHz,
 *   syncStart+syncUpdate, FXCLK0 for ATOM clock source, suitable DTM clock source
 * - EGTM enable guard: enable module, set GCLK unity divider, program CLK0 frequency,
 *   enable necessary EGTM clocks
 * - Calls IfxEgtm_Pwm_init once
 * - Stores initial duty/dead-time into module state
 * - Configures LED GPIO as push‑pull output (do not drive level here)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare configuration structures (locals) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Initialize main config with defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs, polarity, pad driver) */
    /* Phase U */
    output[0].pin                    = PHASE_U_HS;    /* High-side */
    output[0].complementaryPin       = PHASE_U_LS;    /* Low-side  */
    output[0].polarity               = Ifx_ActiveState_high; /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;  /* LS active low  */
    output[0].mode                   = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                    = PHASE_V_HS;
    output[1].complementaryPin       = PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].mode                   = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                    = PHASE_W_HS;
    output[2].complementaryPin       = PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].mode                   = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time configuration: 1 us for both edges */
    dtmConfig[0].deadTime.rising = 1e-6f;
    dtmConfig[0].deadTime.falling = 1e-6f;
    dtmConfig[0].fastShutOff = NULL_PTR; /* Not used */

    dtmConfig[1].deadTime.rising = 1e-6f;
    dtmConfig[1].deadTime.falling = 1e-6f;
    dtmConfig[1].fastShutOff = NULL_PTR;

    dtmConfig[2].deadTime.rising = 1e-6f;
    dtmConfig[2].deadTime.falling = 1e-6f;
    dtmConfig[2].fastShutOff = NULL_PTR;

    /* 5) Interrupt configuration for base channel period event */
    interruptConfig.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;  /* Pulse-notify */
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;                  /* CPU0 */
    interruptConfig.priority    = ISR_PRIORITY_ATOM;                /* Priority 20 */
    interruptConfig.vmId        = IfxSrc_VmId_0;                    /* VM 0 */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;      /* Period callback */
    interruptConfig.dutyEvent   = NULL_PTR;                         /* No duty callback */

    /* 6) Channel configuration: logical contiguous channels starting from SubModule_Ch_0 */
    /* CH0 → Phase U (base channel, owns interrupt) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_U_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;           /* Not connected to MSC */
    channelConfig[0].interrupt = &interruptConfig;   /* Base channel interrupt */

    /* CH1 → Phase V */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_V_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;           /* Interrupt only on base channel */

    /* CH2 → Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Complete main configuration */
    config.cluster             = IfxEgtm_Cluster_0;                   /* Target cluster index: 0 */
    config.subModule           = IfxEgtm_SubModule_atom;              /* Use ATOM sub-module */
    config.alignment           = IfxEgtm_Pwm_Alignment_centerAligned; /* Center-aligned */
    config.syncStart           = TRUE;                                /* Start all channels together */
    config.syncUpdateEnabled   = TRUE;                                /* Transfer at period end */
    config.frequency           = PWM_FREQUENCY;                       /* 20 kHz */
    config.channels            = channelConfig;                       /* Attach channel configs */
    config.numChannels         = NUM_OF_CHANNELS;                     /* 3 channels */
    /* Clock configuration */
    config.clockSource.atom    = IfxEgtm_Atom_Ch_ClkSrc_cmuFxclk0;    /* ATOM from FXCLK_0 */
    config.dtmClockSource      = IfxEgtm_Dtm_ClockSource_cmuClock0;   /* DTM clock source */

    /* 8) EGTM enable guard and CMU setup */
    if (IfxEgtm_isEnabled(&MODULE_EGTM) == FALSE)
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);      /* Dynamic read */
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, 1u, 1u);              /* Unity divider (GCLK = module clock) */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, moduleFreq); /* Program CLK0 */
        /* Enable required EGTM clocks (FXCLK and CLK0). Exact mask is device-specific; 0x3 is a placeholder enabling both. */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, 0x00000003u);
    }

    /* 9) Initialize PWM driver */
    {
        boolean ok = IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &config);
        if (ok == TRUE)
        {
            /* 10) Store initial duty/dead-time into persistent state for later updates */
            g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
            g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
            g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

            g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
            g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
            g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

            g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
            g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
            g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;
        }
        else
        {
            /* Initialization failed: leave state untouched; application may handle error. */
            return;
        }
    }

    /* 11) Configure LED GPIO mode (push-pull). Do not drive level here. */
    IfxPort_setPinModeOutput(&MODULE_P03, 9u, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
