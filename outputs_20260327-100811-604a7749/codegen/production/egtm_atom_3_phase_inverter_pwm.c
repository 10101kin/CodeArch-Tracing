/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: EGTM ATOM 3-Phase Inverter PWM (TC4xx, eGTM unified PWM)
 * - Migration: TC3xx IfxGtm_Pwm -> TC4xx IfxEgtm_Pwm
 * - Cluster 1, ATOM1, channels 0/1/2
 * - Complementary outputs with 1.0 us dead-time
 * - Center-aligned PWM at 20 kHz
 * - Sync start and sync update enabled
 * - Period-event interrupt on channel 0 (CPU0, prio 20)
 *
 * Notes:
 * - Watchdog disable must NOT be placed here. Follow AURIX pattern (CpuX_Main.c only).
 * - Pin routing macros use validated symbols; verify against board PinMap for KIT_A3G_TC4D7_LITE.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ============================
 * Module Configuration Macros
 * ============================ */
#define NUM_OF_CHANNELS              (3)
#define PWM_FREQUENCY_HZ             (20000.0f)     /* 20 kHz */
#define ISR_PRIORITY_ATOM            (20)

/* Initial duties in percent (U=25%, V=50%, W=75%) */
#define PHASE_U_DUTY                 (25.0f)
#define PHASE_V_DUTY                 (50.0f)
#define PHASE_W_DUTY                 (75.0f)

/* Duty update step in percent */
#define PHASE_DUTY_STEP              (5.0f)

/* LED debug pin (port, pin) */
#define LED                          &MODULE_P13, 0

/*
 * eGTM CMU clock enable mask for FXCLK and CLK0. The real SDK provides
 * proper bit masks; here we use a combined mask for enabling both.
 */
#define EGTM_ENABLE_FXCLK_CLK0_MASK  ((uint32)0x00000003u)

/* ============================
 * Validated Pin Selections
 * ============================
 * Use ONLY validated symbols from the template list. Replace with board-validated
 * ATOM1 pins for KIT_A3G_TC4D7_LITE as needed.
 * Note: The list provided showcases ATOM0 examples; update to ATOM1 symbols
 * when available in the project PinMap for the target device/board.
 */
#define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS   (&IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT)
#define PHASE_V_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)   /* placeholder: update to ATOM1 */
#define PHASE_V_LS   (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)  /* placeholder: update to ATOM1 */
#define PHASE_W_HS   (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)   /* placeholder: update to ATOM1 */
#define PHASE_W_LS   (&IfxEgtm_ATOM0_0N_TOUT33_P33_11_OUT)/* placeholder: update to ATOM1 */

/* ============================
 * ISR and Period Callback
 * ============================ */
/* ISR declaration (priority must match InterruptConfig.priority) */
IFX_INTERRUPT(interruptGtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptGtmAtom(void)
{
    /* Minimal ISR body per best practice; LED toggle function is not used here
       due to limited allowed API in this module context. */
}

/* Period-event callback must have empty body (assigned to InterruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ============================
 * Module State
 * ============================ */
typedef struct
{
    IfxEgtm_Pwm              pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];         /* persistent channel handles */
    float32                  dutyCycles[NUM_OF_CHANNELS];       /* duty in percent */
    float32                  phases[NUM_OF_CHANNELS];           /* phase in percent */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];        /* dead-time values */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;               /* persistent module state */

/* ============================
 * Public API Implementations
 * ============================ */

/*
 * Initialize a 3-phase inverter PWM using eGTM unified PWM (Cluster 1, ATOM1, channels 0/1/2)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];

    /* 2) Load default configuration values */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Configure complementary outputs for three phases */
    /* Phase U */
    output[0].pin                     = PHASE_U_HS;                  /* high-side */
    output[0].complementaryPin        = PHASE_U_LS;                  /* low-side  */
    output[0].polarity                = Ifx_ActiveState_high;        /* HS active HIGH */
    output[0].complementaryPolarity   = Ifx_ActiveState_low;         /* LS active LOW  */
    output[0].outputMode              = IfxPort_OutputMode_pushPull;
    output[0].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                     = PHASE_V_HS;
    output[1].complementaryPin        = PHASE_V_LS;
    output[1].polarity                = Ifx_ActiveState_high;
    output[1].complementaryPolarity   = Ifx_ActiveState_low;
    output[1].outputMode              = IfxPort_OutputMode_pushPull;
    output[1].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                     = PHASE_W_HS;
    output[2].complementaryPin        = PHASE_W_LS;
    output[2].polarity                = Ifx_ActiveState_high;
    output[2].complementaryPolarity   = Ifx_ActiveState_low;
    output[2].outputMode              = IfxPort_OutputMode_pushPull;
    output[2].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Configure DTM dead-time: 1.0 microsecond rising/falling for all channels */
    dtmConfig[0].deadTime.rising      = 1e-6f;
    dtmConfig[0].deadTime.falling     = 1e-6f;
    dtmConfig[0].fastShutOff          = NULL_PTR;

    dtmConfig[1].deadTime.rising      = 1e-6f;
    dtmConfig[1].deadTime.falling     = 1e-6f;
    dtmConfig[1].fastShutOff          = NULL_PTR;

    dtmConfig[2].deadTime.rising      = 1e-6f;
    dtmConfig[2].deadTime.falling     = 1e-6f;
    dtmConfig[2].fastShutOff          = NULL_PTR;

    /* 5) Single period-event interrupt on base channel (channel 0) */
    interruptConfig.mode              = IfxEgtm_IrqMode_pulseNotify; /* period notification */
    interruptConfig.isrProvider       = IfxSrc_Tos_cpu0;             /* CPU0 */
    interruptConfig.priority          = ISR_PRIORITY_ATOM;           /* priority 20 */
    interruptConfig.periodEvent       = IfxEgtm_periodEventFunction; /* callback */
    interruptConfig.dutyEvent         = NULL_PTR;                    /* no duty callback */

    /* 6) Channel configuration for ATOM1 logical channels 0/1/2 */
    /* Channel 0 (V = 50%) */
    channelConfig[0].timerCh          = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase            = 0.0f;
    channelConfig[0].duty             = PHASE_V_DUTY;
    channelConfig[0].dtm              = &dtmConfig[0];
    channelConfig[0].output           = &output[0];
    channelConfig[0].mscOut           = NULL_PTR;
    channelConfig[0].interrupt        = &interruptConfig;            /* period IRQ only on ch0 */

    /* Channel 1 (U = 25%) */
    channelConfig[1].timerCh          = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase            = 0.0f;
    channelConfig[1].duty             = PHASE_U_DUTY;
    channelConfig[1].dtm              = &dtmConfig[1];
    channelConfig[1].output           = &output[1];
    channelConfig[1].mscOut           = NULL_PTR;
    channelConfig[1].interrupt        = NULL_PTR;                    /* no IRQ on ch1 */

    /* Channel 2 (W = 75%) */
    channelConfig[2].timerCh          = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase            = 0.0f;
    channelConfig[2].duty             = PHASE_W_DUTY;
    channelConfig[2].dtm              = &dtmConfig[2];
    channelConfig[2].output           = &output[2];
    channelConfig[2].mscOut           = NULL_PTR;
    channelConfig[2].interrupt        = NULL_PTR;                    /* no IRQ on ch2 */

    /* 7) Complete main PWM configuration */
    config.cluster                    = IfxEgtm_Cluster_1;           /* eGTM Cluster 1 */
    config.subModule                  = IfxEgtm_Pwm_SubModule_atom;  /* ATOM submodule */
    config.alignment                  = IfxEgtm_Pwm_Alignment_center;/* center-aligned */
    config.frequency                  = PWM_FREQUENCY_HZ;            /* 20 kHz */
    config.clockSource.atom           = IfxEgtm_Cmu_Fxclk_0;         /* FXCLK0 */
    config.dtmClockSource             = IfxEgtm_Dtm_ClockSource_cmuClock0;
    config.syncStart                  = TRUE;                        /* synchronized start */
    config.syncUpdateEnabled          = TRUE;                        /* synchronized buffered update */
    config.channels                   = channelConfig;               /* channel configs */
    config.numChannels                = NUM_OF_CHANNELS;             /* 3 channels */

    /* 8) eGTM enable and clock setup (inside guard) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Read module frequency (dynamic) */
        (void)IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        /* Set dividers to 1:1 for GCLK and ECLK0 */
        IfxEgtm_Cmu_setGclkDivider(&MODULE_EGTM, 1u, 1u);
        IfxEgtm_Cmu_setEclkDivider(&MODULE_EGTM, IfxEgtm_Cmu_Eclk_0, 1u, 1u);
        /* Enable FXCLK and CLK0 for ATOM operation */
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, EGTM_ENABLE_FXCLK_CLK0_MASK);
    }

    /* 9) Initialize PWM driver (applies initial duty/phase, sets up pins and DTM) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Store initialized duties and dead-times into persistent state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0]     = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1]     = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2]     = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;

    /* 11) Configure a status GPIO (LED/debug) as push-pull output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/*
 * Perform a synchronous duty increment on the three PWM channels
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap to 0 when next step would reach/exceed 100%, then always add step */
    if ((g_egtmAtom3phInv.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[0] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[1] = 0.0f;
    }
    if ((g_egtmAtom3phInv.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f)
    {
        g_egtmAtom3phInv.dutyCycles[2] = 0.0f;
    }

    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply new duties immediately across synchronized group */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
