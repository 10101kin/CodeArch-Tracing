/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * Production driver for EGTM ATOM 3-Phase Inverter PWM using IfxEgtm_Pwm unified API (TC4xx).
 * - 3 complementary pairs (U/V/W)
 * - Center-aligned symmetric PWM at 20 kHz
 * - Dead-time = 0.5 us, Min pulse = 1.0 us
 * - Clock: eGTM Fxclk0 sourced from GCLK (no extra prescaling)
 *
 * Notes:
 * - Watchdog disable must NOT be placed here (Cpu0_Main.c only per AURIX standard).
 * - Pin routing: Uses validated symbols where available; V/W pins are placeholders (NULL_PTR) to be finalized per board.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "Ifx_Types.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"
#include "IfxPort_PinMap.h"

/* =============================
 * Configuration Macros
 * ============================= */
#define NUM_OF_CHANNELS            (3u)
#define PWM_FREQUENCY_HZ           (20000.0f)
#define ISR_PRIORITY_ATOM          (10)

/* LED (Port,Pin compound macro) */
#define LED                        &MODULE_P13, 0

/* Dead-time and minimum pulse (seconds) */
#define PWM_DEADTIME_S             (0.5e-6f)
#define PWM_MIN_PULSE_S            (1.0e-6f)

/* Initial duties in PERCENT (0..100) */
#define PHASE_U_DUTY               (25.0f)
#define PHASE_V_DUTY               (50.0f)
#define PHASE_W_DUTY               (75.0f)

/* Duty update step (percent) */
#define PHASE_DUTY_STEP            (5.0f)

/*
 * Pin routing macros
 * Use ONLY validated symbols; V/W are left NULL pending board confirmation.
 */
#define PHASE_U_HS                 (&IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT)
#define PHASE_U_LS                 (&IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT)
#define PHASE_V_HS                 (NULL_PTR) /* TODO: select from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE */
#define PHASE_V_LS                 (NULL_PTR) /* TODO: select from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE */
#define PHASE_W_HS                 (NULL_PTR) /* TODO: select from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE */
#define PHASE_W_LS                 (NULL_PTR) /* TODO: select from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE */

/* =============================
 * Module state
 * ============================= */
typedef struct
{
    IfxEgtm_Pwm               pwm;                               /* driver handle */
    IfxEgtm_Pwm_Channel       channels[NUM_OF_CHANNELS];         /* persistent channel data (owned by driver) */
    float32                   dutyCycles[NUM_OF_CHANNELS];       /* duty in percent (0..100) */
    float32                   phases[NUM_OF_CHANNELS];           /* phase offsets (not used; reserved) */
    IfxEgtm_Pwm_DeadTime      deadTimes[NUM_OF_CHANNELS];        /* configured dead-times */
} EgtmAtom3ph_State;

IFX_STATIC EgtmAtom3ph_State g_egtmAtom3phState;

/* =============================
 * Forward-declared internal ISR and callback
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

void IfxEgtm_periodEventFunction(void *data)
{
    (void)data;
}

/* =============================
 * Init function
 * ============================= */
/**
 * Initialize the eGTM-based 3-phase inverter PWM using the unified IfxEgtm_Pwm driver.
 * - Center-aligned symmetric, complementary outputs via DTM
 * - 3 logical channels on same ATOM cluster (indices 0..2)
 */
void initEgtmAtom3phInv(void)
{
    /* 1) Declare all configuration structures as local variables */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  irqCfg;

    /* Initialize defaults */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 2) Output configuration: three complementary pairs (U/V/W) */
    output[0].pin                    = PHASE_U_HS;
    output[0].complementaryPin       = PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;   /* HS active HIGH */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;    /* LS active LOW  */
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = PHASE_V_HS;             /* TBD: board pin */
    output[1].complementaryPin       = PHASE_V_LS;             /* TBD: board pin */
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = PHASE_W_HS;             /* TBD: board pin */
    output[2].complementaryPin       = PHASE_W_LS;             /* TBD: board pin */
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3) DTM configuration (dead-time/min pulse) */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; ++i)
    {
        dtmConfig[i].deadTime.risingEdge = PWM_DEADTIME_S;
        dtmConfig[i].deadTime.fallingEdge = PWM_DEADTIME_S;
        dtmConfig[i].minPulse             = PWM_MIN_PULSE_S;
        dtmConfig[i].fastShutOff          = NULL_PTR;  /* not used */
    }

    /* 4) Interrupt configuration (assigned to channel 0) */
    irqCfg.mode        = IfxEgtm_Pwm_IrqMode_pulseNotify;
    irqCfg.isrProvider = IfxSrc_Tos_cpu0;
    irqCfg.priority    = ISR_PRIORITY_ATOM;
    irqCfg.periodEvent = IfxEgtm_periodEventFunction;
    irqCfg.dutyEvent   = NULL_PTR;

    /* 5) Channel configuration: logical indices 0..2 on same ATOM cluster */
    /* U */
    channelConfig[0].timerCh = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase   = 0.0f;
    channelConfig[0].duty    = PHASE_U_DUTY;         /* percent */
    channelConfig[0].dtm     = &dtmConfig[0];
    channelConfig[0].output  = &output[0];
    channelConfig[0].mscOut  = NULL_PTR;
    channelConfig[0].interrupt = &irqCfg;            /* assign ISR to first channel */

    /* V */
    channelConfig[1].timerCh = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase   = 0.0f;
    channelConfig[1].duty    = PHASE_V_DUTY;         /* percent */
    channelConfig[1].dtm     = &dtmConfig[1];
    channelConfig[1].output  = &output[1];
    channelConfig[1].mscOut  = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;           /* no ISR */

    /* W */
    channelConfig[2].timerCh = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase   = 0.0f;
    channelConfig[2].duty    = PHASE_W_DUTY;         /* percent */
    channelConfig[2].dtm     = &dtmConfig[2];
    channelConfig[2].output  = &output[2];
    channelConfig[2].mscOut  = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;           /* no ISR */

    /* 6) Main config */
    config.egtmSFR            = &MODULE_EGTM;
    config.cluster            = IfxEgtm_Cluster_0;                        /* ATOM cluster 0 */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;               /* Use ATOM */
    config.alignment          = IfxEgtm_Pwm_Alignment_centerAlignedSymmetric;
    config.numChannels        = (uint8)NUM_OF_CHANNELS;
    config.channels           = channelConfig;                            /* per-channel config */
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxEgtm_Cmu_Fxclk_0;                      /* Fxclk0 */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;        /* DTM clock 0 */
    config.syncUpdateEnabled  = TRUE;
    config.syncStart          = TRUE;                                     /* start synchronously */

    /* 7) eGTM enable and CMU clocks (inside enable-guard, exact pattern) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM)) {
        IfxEgtm_enable(&MODULE_EGTM);
        float32 freq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, freq);
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, freq);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_FXCLK | IFXEGTM_CMU_CLKEN_CLK0));
    }

    /* 8) Initialize PWM using persistent channels array in module state */
    IfxEgtm_Pwm_init(&g_egtmAtom3phState.pwm, g_egtmAtom3phState.channels, &config);

    /* 9) Store initial duties and dead-times into module state (percent and seconds) */
    g_egtmAtom3phState.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phState.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phState.dutyCycles[2] = PHASE_W_DUTY;

    g_egtmAtom3phState.phases[0] = 0.0f;
    g_egtmAtom3phState.phases[1] = 0.0f;
    g_egtmAtom3phState.phases[2] = 0.0f;

    g_egtmAtom3phState.deadTimes[0].risingEdge = PWM_DEADTIME_S; g_egtmAtom3phState.deadTimes[0].fallingEdge = PWM_DEADTIME_S;
    g_egtmAtom3phState.deadTimes[1].risingEdge = PWM_DEADTIME_S; g_egtmAtom3phState.deadTimes[1].fallingEdge = PWM_DEADTIME_S;
    g_egtmAtom3phState.deadTimes[2].risingEdge = PWM_DEADTIME_S; g_egtmAtom3phState.deadTimes[2].fallingEdge = PWM_DEADTIME_S;

    /* 10) Optional diagnostic LED as push-pull output (post-PWM init) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Duty update function
 * ============================= */
/**
 * Update the three logical phase duties and apply synchronously at the next PWM update point.
 * Uses immediate multi-channel update; percent domain [0..100].
 */
void updateEgtmAtom3phInvDuty(void)
{
    /* DUTY WRAP RULE: check->maybe reset->unconditional add; three explicit blocks */
    if ((g_egtmAtom3phState.dutyCycles[0] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[0] = 0.0f; }
    if ((g_egtmAtom3phState.dutyCycles[1] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[1] = 0.0f; }
    if ((g_egtmAtom3phState.dutyCycles[2] + PHASE_DUTY_STEP) >= 100.0f) { g_egtmAtom3phState.dutyCycles[2] = 0.0f; }

    g_egtmAtom3phState.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phState.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phState.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Apply update immediately; takes effect synchronously at next period boundary */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phState.pwm, (float32*)g_egtmAtom3phState.dutyCycles);
}
