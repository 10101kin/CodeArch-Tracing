/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver: eGTM ATOM 3-Phase Complementary PWM (TC4xx)
 * - Submodule: ATOM (Cluster 0)
 * - Unified driver: IfxEgtm_Pwm
 * - Center-aligned 20 kHz, complementary pairs with 1 us dead-time
 * - Initial duties: U=25%, V=50%, W=75%
 * - Interrupt: priority 20 (CPU0), ISR toggles LED P03.9
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY              (20000.0f)   /* Hz */
#define ISR_PRIORITY_ATOM          (20)
#define PHASE_U_DUTY               (25.0f)      /* percent */
#define PHASE_V_DUTY               (50.0f)      /* percent */
#define PHASE_W_DUTY               (75.0f)      /* percent */
#define PHASE_DUTY_STEP            (10.0f)      /* percent per update (not used in init) */

/* LED: P03.9 (used by ISR) */
#define LED                        &MODULE_P03, 9

/* =============================
 * Pin Map Macros (validated symbols only)
 *
 * NOTE:
 *  - The following macros map the requested pins to IfxEgtm pin symbols.
 *  - Only symbols from the validated list are used. Pins not present in the
 *    validated list are left as NULL_PTR and must be resolved during board
 *    integration by selecting the correct &IfxEgtm_ATOMx_y[_N]_TOUTz_Pxx_y_OUT
 *    symbol from IfxEgtm_PinMap.h for the target device/package.
 * ============================= */
/* Phase U: HS=P20.8/TOUT64 (not in validated list) | LS=P20.9/TOUT65 (validated) */
#define PHASE_U_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT64_P20_8_OUT for target */
#define PHASE_U_LS                 (&IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT)

/* Phase V: HS=P21.4/TOUT55 (not in validated list) | LS=P20.11/TOUT67 (not in validated list) */
#define PHASE_V_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT55_P21_4_OUT for target */
#define PHASE_V_LS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_yN_TOUT67_P20_11_OUT for target */

/* Phase W: HS=P20.12/TOUT68 (not in validated list) | LS=P20.13/TOUT69 (not in validated list) */
#define PHASE_W_HS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_y_TOUT68_P20_12_OUT for target */
#define PHASE_W_LS                 (NULL_PTR) /* Replace with &IfxEgtm_ATOMx_yN_TOUT69_P20_13_OUT for target */

/* =============================
 * Persistent Module State
 * ============================= */

typedef struct
{
    IfxEgtm_Pwm              pwm;                                  /* driver handle */
    IfxEgtm_Pwm_Channel      channels[NUM_OF_CHANNELS];            /* persistent channel data */
    float32                  dutyCycles[NUM_OF_CHANNELS];          /* cached duties in percent */
    float32                  phases[NUM_OF_CHANNELS];              /* cached phases in fraction (0..1) */
    IfxEgtm_Pwm_DeadTime     deadTimes[NUM_OF_CHANNELS];           /* cached dead-times */
} EgtmAtom3phInv_State;

IFX_STATIC EgtmAtom3phInv_State g_egtmAtom3phInv;                  /* module-level state */

/* =============================
 * ISR and Period Callback
 * ============================= */

/* Period-event callback (assigned into InterruptConfig). Body must be empty. */
void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* ISR installed via priority by the high-level driver. Body must only toggle LED. */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM)
{
    IfxPort_togglePin(LED);
}

/* =============================
 * Public API
 * ============================= */

/**
 * Initialize a 3-phase complementary PWM using the unified eGTM PWM driver on ATOM in cluster 0.
 * - 3 logical channels (U, V, W) as complementary pairs
 * - Center-aligned, 20 kHz, 1 us dead-time (rising/falling)
 * - Initial duties: U=25%, V=50%, W=75%
 * - syncStart/syncUpdate enabled
 * - ATOM clock source: CLK0; DTM clock source: CMU clock 0
 * - Interrupt: pulse notify on base channel (ch0), CPU0, prio 20
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Local configuration structures */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];

    /* 2) Load driver defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration (complementary pairs) */
    /* Phase U */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;  /* HS: active-high */
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;  /* LS: active-low  */
    output[0].polarity              = Ifx_ActiveState_high;
    output[0].complementaryPolarity = Ifx_ActiveState_low;
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

    /* 4) Dead-time configuration: 1 us rising and falling for each pair */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[0].fastShutOff      = NULL_PTR;

    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[1].fastShutOff      = NULL_PTR;

    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;
    dtmConfig[2].fastShutOff      = NULL_PTR;

    /* 5) Interrupt configuration (base channel only) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Channel configuration (logical indices 0..2) */
    /* CH0 -> Phase U */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;  /* base channel */

    /* CH1 -> Phase V */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;          /* only CH0 has interrupt */

    /* CH2 -> Phase W */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Main PWM configuration */
    config.cluster             = IfxEgtm_Cluster_0;                         /* eGTM Cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;                /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;              /* center-aligned */
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;                              /* 20 kHz */
    config.clockSource.atom   = (uint32)IfxEgtm_Cmu_Clk_0;                  /* ATOM CLK0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;          /* DTM clock 0 */
    config.syncUpdateEnabled  = TRUE;                                       /* synced updates */
    config.syncStart          = TRUE;                                       /* synced start */

    /* 8) eGTM enable and CMU clock configuration (guarded) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 9) Initialize PWM driver: programs channels, routes pins, applies duty/phase/dead-time */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* 10) Cache initial state in persistent module state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.phases[0] = channelConfig[0].phase;
    g_egtmAtom3phInv.phases[1] = channelConfig[1].phase;
    g_egtmAtom3phInv.phases[2] = channelConfig[2].phase;

    g_egtmAtom3phInv.deadTimes[0] = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1] = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2] = dtmConfig[2].deadTime;

    /* 11) Configure LED GPIO (used by ISR) as push-pull output AFTER PWM init */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}
