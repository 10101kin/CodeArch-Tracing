/*
 * egtm_atom_3_phase_inverter_pwm.c
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Implementation
 */
#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ============================
 * Driver state
 * ============================ */
typedef struct {
    IfxEgtm_Pwm          pwm;                             /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];       /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];     /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];         /* Phase shift values (deg or %) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];      /* Dead time values (s) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* module-local driver instance */

/* ============================
 * ISR and callback
 * ============================ */
/* Period event callback (linked via interruptConfig) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* User period event handling can be placed here if needed */
    (void)data;
}

/* Private ISR body for reuse */
static void interruptEgtmPwmPeriod(void)
{
    IfxPort_togglePin(LED);
}

/* Vector ISR wrapper with exact macro and priority name */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    interruptEgtmPwmPeriod();
}

/* ============================
 * Initialization per SW Detailed Design
 * ============================ */
void initEgtmAtom3phInv(void)
{
    /* 1) Enable eGTM and CMU clock input (Clk1) at required frequency */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Program GCLK to requirements value and enable CMU clocks */
    IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, TIMING_CMU_CLOCK_HZ);
    /* Select ATOM clock source: Clk1 uses GCLK as input */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_1, TRUE);
    /* Enable CMU CLK0 for DTM and CLK1 for ATOM time base */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)(IFXEGTM_CMU_CLKEN_CLK0 | IFXEGTM_CMU_CLKEN_CLK1));

    /* 2) Initialize unified PWM configuration with defaults */
    IfxEgtm_Pwm_Config config;
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Submodule, alignment, frequency, clocks, sync behavior */
    config.cluster           = IfxEgtm_Cluster_1;                 /* Proposed cluster; update if HW differs */
    config.subModule         = IfxEgtm_Pwm_SubModule_atom;
    config.alignment         = IfxEgtm_Pwm_Alignment_center;      /* Center-aligned (up-down) */
    config.frequency         = PWM_FREQUENCY;                      /* 20 kHz */
    config.clockSource.atom  = IfxEgtm_Cmu_Clk_1;                  /* Use CMU CLK1 */
    config.dtmClockSource    = IfxEgtm_Dtm_ClockSource_cmuClock0;  /* DTM clock */
    config.syncUpdateEnabled = TRUE;                               /* Shadow update at period */
    config.syncStart         = FALSE;                              /* We'll start via API per design step 11 */

    /* 3) Output mapping (main + complementary), polarities and pad configuration */
    IfxEgtm_Pwm_OutputConfig output[NUM_OF_CHANNELS];

    output[0].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[0].polarity               = Ifx_ActiveState_high;      /* HS active high */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;       /* LS active low */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
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

    /* 4) Attach DTM configuration with 1.0us rising/falling */
    IfxEgtm_Pwm_DtmConfig dtmConfig[NUM_OF_CHANNELS];
    dtmConfig[0].deadTime.rising  = 1.0e-6f;
    dtmConfig[0].deadTime.falling = 1.0e-6f;
    dtmConfig[1].deadTime.rising  = 1.0e-6f;
    dtmConfig[1].deadTime.falling = 1.0e-6f;
    dtmConfig[2].deadTime.rising  = 1.0e-6f;
    dtmConfig[2].deadTime.falling = 1.0e-6f;

    /* 5) Single period interrupt on base channel (CH0) */
    IfxEgtm_Pwm_InterruptConfig interruptConfig;
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;     /* EXACT macro name per reference */
    interruptConfig.vmId        = IfxSrc_VmId_0;         /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* 6) Initialize per-channel configuration entries */
    IfxEgtm_Pwm_ChannelConfig channelConfig[NUM_OF_CHANNELS];
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    /* Base channel CH0 (use Phase V as base to mirror reference ordering) */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = PHASE_V_DUTY;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;
    channelConfig[0].interrupt = &interruptConfig;  /* Period ISR on base channel */

    /* Sync channel CH1 - Phase U */
    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = PHASE_U_DUTY;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;
    channelConfig[1].interrupt = NULL_PTR;

    /* Sync channel CH2 - Phase W */
    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = PHASE_W_DUTY;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* Link channels to main config */
    config.numChannels = NUM_OF_CHANNELS;
    config.channels    = channelConfig;

    /* 7) Apply pin mux for all outputs using eGTM PinMap API */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 8) Initialize the PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.channels, &config);

    /* 9) Program deadtime for all channels using bulk API */
    g_egtmAtom3phInv.deadTimes[0].rising  = 1.0e-6f;
    g_egtmAtom3phInv.deadTimes[0].falling = 1.0e-6f;
    g_egtmAtom3phInv.deadTimes[1].rising  = 1.0e-6f;
    g_egtmAtom3phInv.deadTimes[1].falling = 1.0e-6f;
    g_egtmAtom3phInv.deadTimes[2].rising  = 1.0e-6f;
    g_egtmAtom3phInv.deadTimes[2].falling = 1.0e-6f;
    IfxEgtm_Pwm_updateChannelsDeadTime(&g_egtmAtom3phInv.pwm, (IfxEgtm_Pwm_DeadTime *)g_egtmAtom3phInv.deadTimes);

    /* Store initial duty for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    /* 10) Install ISR into CPU interrupt vector (base channel priority) */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* 11) Start synchronized PWM channels */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Configure LED pin for ISR diagnostics */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ============================
 * Runtime duty update
 * ============================ */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap at 100%, then increment by fixed step per channel */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += PHASE_DUTY_STEP;
    }

    /* Bulk update without immediate apply; sync-update applies at period */
    IfxEgtm_Pwm_updateChannelsDuty(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
