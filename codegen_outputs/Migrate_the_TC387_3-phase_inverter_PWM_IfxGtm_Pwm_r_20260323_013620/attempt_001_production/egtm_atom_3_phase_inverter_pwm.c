#include "egtm_atom_3_phase_inverter_pwm.h"

#include "IfxEgtm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* ========================= Configuration Macros (from requirements) ========================= */
#define NUM_OF_CHANNELS          (3)
#define PWM_FREQUENCY            (20000.0f)   /* 20 kHz */
#define ISR_PRIORITY_ATOM        (20)

/* Initial duty cycles in percent (reference pattern) */
#define PHASE_U_DUTY             (25.0f)
#define PHASE_V_DUTY             (50.0f)
#define PHASE_W_DUTY             (75.0f)
#define PHASE_DUTY_STEP          (10.0f)

/* LED status pin (heartbeat). Note: finalize after board pin confirmation */
#define LED                      &MODULE_P13, 0

/* ========================= ATOM1 TOUT Mapping (TBD: board confirmation required) ========================= */
/*
 * TOUT_MAPPING_STATUS = TBD - requires KIT_A3G_TC4D7_LITE pinout confirmation
 * TOUT_MAPPING_DEFAULTS_APPLIED = True
 * NOTE: Select the first available TOUT mapping for ATOM1 CH0/1/2 and their complementary N-pins
 *       from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE. Replace the NULL_PTR placeholders below with
 *       concrete TOUT mappings once board pins are confirmed (e.g., &IfxEgtm_ATOM1_0_TOUTxxx_Pyy_z_OUT).
 */
#define PHASE_U_HS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_0_TOUTxxx_Pyy_z_OUT */
#define PHASE_U_LS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_0N_TOUTxxx_Pyy_z_OUT */
#define PHASE_V_HS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_1_TOUTxxx_Pyy_z_OUT */
#define PHASE_V_LS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_1N_TOUTxxx_Pyy_z_OUT */
#define PHASE_W_HS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_2_TOUTxxx_Pyy_z_OUT */
#define PHASE_W_LS_TOUT          NULL_PTR  /* e.g., &IfxEgtm_ATOM1_2N_TOUTxxx_Pyy_z_OUT */

/* ========================= Driver State Structure ========================= */
typedef struct
{
    IfxEgtm_Pwm           pwm;                               /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32               dutyCycles[NUM_OF_CHANNELS];       /* Duty in percent */
    float32               phases[NUM_OF_CHANNELS];           /* Phase offsets */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];        /* Dead-time values */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* Module-global state */

/* ========================= ISR and Callback ========================= */
/* Period-event ISR: toggle LED only (minimal processing) */
IFX_INTERRUPT(EgtmAtomIsr_periodEvent, 0, ISR_PRIORITY_ATOM);
void EgtmAtomIsr_periodEvent(void)
{
    IfxPort_togglePin(LED);
}

/* Period event callback for unified driver (unused payload) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Optional: user period-event handling hook */
    (void)data;
}

/* ========================= Initialization ========================= */
void initEgtmAtom3phInv(void)
{
    /* 1) Enable the eGTM module and CMU clocks (MANDATORY) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
        /* Set GCLK to module frequency and route CLK0 from GCLK; enable CLK0 */
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Initialize module state defaults for duties and phases */
    g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
    g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
    g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;
    g_egtmAtom3phInv.phases[0]     = 0.0f;
    g_egtmAtom3phInv.phases[1]     = 0.0f;
    g_egtmAtom3phInv.phases[2]     = 0.0f;

    /* 2) Initialize main PWM configuration */
    IfxEgtm_Pwm_Config config;
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);   /* EXACT signature */

    /* 3) Per-channel configuration containers */
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    /* 3a) OUTPUT pin routing via unified driver (complementary) */
    /* Phase U: CH0 */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS_TOUT;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS_TOUT;
    output[0].polarity               = Ifx_ActiveState_high;
    output[0].complementaryPolarity  = Ifx_ActiveState_low;
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V: CH1 */
    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS_TOUT;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS_TOUT;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W: CH2 */
    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS_TOUT;
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS_TOUT;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 3b) DEAD-TIME configuration: 1.0 us rising and falling for each channel */
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 3c) INTERRUPT configuration: period-event on base channel, CPU0 priority 20 */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;     /* EXACT macro name */
    interruptConfig.vmId        = IfxSrc_VmId_0;         /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 3d) CHANNEL configuration: indices 0,1,2; CH0 as base with ISR */
    /* CH0 - Phase U */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = g_egtmAtom3phInv.phases[0];
    channelConfig[0].duty       = g_egtmAtom3phInv.dutyCycles[0];
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;     /* Period interrupt on CH0 */

    /* CH1 - Phase V */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = g_egtmAtom3phInv.phases[1];
    channelConfig[1].duty       = g_egtmAtom3phInv.dutyCycles[1];
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;             /* No interrupt on CH1 */

    /* CH2 - Phase W */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = g_egtmAtom3phInv.phases[2];
    channelConfig[2].duty       = g_egtmAtom3phInv.dutyCycles[2];
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;             /* No interrupt on CH2 */

    /* 4) Set main config fields for ATOM center-aligned 3-phase inverter */
    config.cluster            = IfxEgtm_Cluster_0;                 /* Cluster selection */
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;        /* ATOM submodule */
    config.alignment          = IfxEgtm_Pwm_Alignment_center;      /* Center aligned */
    config.syncStart          = TRUE;                               /* Auto-start channels */
    config.syncUpdateEnabled  = TRUE;                               /* Shadow register updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = channelConfig;
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;                  /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM CMU clock 0 */

    /* 5) Initialize the PWM driver (unified) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config); /* VOID return */

    /* 6) Store initial values for runtime */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* 7) Configure LED GPIO and install ISR (toggle on period event) */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxCpu_Irq_installInterruptHandler((void*)EgtmAtomIsr_periodEvent, (uint32)ISR_PRIORITY_ATOM);

    /* Note:
     * - Do NOT call IfxEgtm_Pwm_startSyncedChannels(): syncStart=TRUE already starts channels
     * - Do NOT call IfxEgtm_PinMap_setAtomTout(): unified driver routes pins from output[].pin
     * - Do NOT call IfxEgtm_Pwm_enableChannelsSyncUpdate(): syncUpdateEnabled=TRUE already set
     */
}

/* ========================= Runtime Duty Update ========================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-around check per channel, then increment by step */
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

    /* Apply new duty values immediately (unified multi-channel API) */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
