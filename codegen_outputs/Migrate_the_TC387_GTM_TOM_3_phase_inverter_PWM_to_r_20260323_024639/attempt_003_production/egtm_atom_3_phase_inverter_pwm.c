#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ========================================================================
 * Internal driver data structure (matches TC4xx EGTM unified driver pattern)
 * ======================================================================== */
typedef struct {
    IfxEgtm_Pwm          pwm;                           /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];     /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];   /* Duty cycle values in percent */
    float32              phases[NUM_OF_CHANNELS];       /* Phase shift values in percent */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];    /* Dead-time values */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* Static module instance */

/* ========================================================================
 * Forward declarations (private)
 * ======================================================================== */
static void configureEgtmClocks(void);
static void errorStatusHook(uint32 errorCode);

/* Period event callback (unified driver pattern) */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Stub for period event; extend if needed */
    (void)data;
}

/* ========================================================================
 * ISR (period event on base channel) - exact macro name per reference
 * ======================================================================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Delegate servicing to unified driver handler (base channel) */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
    /* Optional diagnostic: toggle LED at period boundary */
    IfxPort_togglePin(LED);
}

/* ========================================================================
 * Private helpers
 * ======================================================================== */
static void configureEgtmClocks(void)
{
    /* Enable module */
    IfxEgtm_enable(&MODULE_EGTM);

    /* Set GCLK to current module frequency (keeps default) */
    {
        float32 gclk = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, gclk);
    }

    /* Select CLK0 source = global and set divider to achieve 100 MHz */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);
    {
        float32 gclkHz  = IfxEgtm_Cmu_getGclkFrequency(&MODULE_EGTM);
        float32 tgtHz   = 100000000.0f; /* 100 MHz */
        uint32  count   = (uint32)((gclkHz / tgtHz) - 1.0f);
        IfxEgtm_Cmu_setClkCount(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, count);
    }

    /* Enable required clocks (CLK0). FXCLK enabling can be added if required by application */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
}

static void errorStatusHook(uint32 errorCode)
{
    /* Stub: integrate with SMU if required */
    (void)errorCode;
}

/* ========================================================================
 * Initialization: EGTM ATOM 3-phase inverter using unified IfxEgtm_Pwm
 * ======================================================================== */
void initEgtmAtom3phInv(void)
{
    /* 1) Enable module and configure CMU clocks as per requirements */
    configureEgtmClocks();

    /* 2) Prepare main PWM config (center-aligned, sync start/update) */
    IfxEgtm_Pwm_Config          config;
    IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Initialize per-channel defaults */
    for (uint8 i = 0u; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
    }

    /* 3) Output configuration (pins routed via unified driver, complementary high/low) */
    output[0].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity              = Ifx_ActiveState_high;  /* HS non-inverted */
    output[0].complementaryPolarity = Ifx_ActiveState_low;   /* LS inverted */
    output[0].outputMode            = IfxPort_OutputMode_pushPull;
    output[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity              = Ifx_ActiveState_high;
    output[1].complementaryPolarity = Ifx_ActiveState_low;
    output[1].outputMode            = IfxPort_OutputMode_pushPull;
    output[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                   = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin      = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity              = Ifx_ActiveState_high;
    output[2].complementaryPolarity = Ifx_ActiveState_low;
    output[2].outputMode            = IfxPort_OutputMode_pushPull;
    output[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Optional explicit pin routing via PinMap API (kept to match SW design call list) */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_U_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_U_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_V_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_V_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_W_HS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap*)PHASE_W_LS, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 4) Configure CDTM dead-time for each phase (1 us rising/falling) */
    dtmConfig[0].deadTime = DEADTIME_DEADTIME_SEC; /* 1 us */
    dtmConfig[1].deadTime = DEADTIME_DEADTIME_SEC; /* 1 us */
    dtmConfig[2].deadTime = DEADTIME_DEADTIME_SEC; /* 1 us */

    /* 5) Initialize channel configuration array (base channel first, then sync channels) */
    /* Base: U (ATOM0 ch0) */
    channelConfig[0].timerCh   = EGTM_RESOURCES_ATOM_CH_U_BASE;
    channelConfig[0].phase     = 0.0f;
    channelConfig[0].duty      = DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_U;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];
    channelConfig[0].mscOut    = NULL_PTR;

    /* Sync: V (ATOM0 ch2) */
    channelConfig[1].timerCh   = EGTM_RESOURCES_ATOM_CH_V_BASE;
    channelConfig[1].phase     = 0.0f;
    channelConfig[1].duty      = DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_V;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];
    channelConfig[1].mscOut    = NULL_PTR;

    /* Sync: W (ATOM0 ch4) */
    channelConfig[2].timerCh   = EGTM_RESOURCES_ATOM_CH_W_BASE;
    channelConfig[2].phase     = 0.0f;
    channelConfig[2].duty      = DUTY_UPDATE_INITIAL_DUTY_PER_PHASE_W;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];
    channelConfig[2].mscOut    = NULL_PTR;

    /* 6) Configure the period interrupt on base channel only (provider: cpu0, prio 20) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    channelConfig[0].interrupt  = &interruptConfig; /* base channel gets ISR */
    channelConfig[1].interrupt  = NULL_PTR;         /* others: no ISR */
    channelConfig[2].interrupt  = NULL_PTR;

    /* 7) Finalize global PWM config and initialize driver */
    config.cluster            = EGTM_RESOURCES_CLUSTER;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;  /* start all channels synchronously */
    config.syncUpdateEnabled  = TRUE;  /* shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Install ISR handler before enabling outputs */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* Initialize PWM driver */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Explicit start (sync) kept to match SW design sequence */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Store initial values for runtime */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = dtmConfig[0].deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = dtmConfig[1].deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = dtmConfig[2].deadTime;
    g_egtmAtom3phInv.phases[0]     = 0.0f;
    g_egtmAtom3phInv.phases[1]     = 0.0f;
    g_egtmAtom3phInv.phases[2]     = 0.0f;

    /* 8) Configure status LED pin */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ========================================================================
 * Runtime update: independent ramp per phase with wrap, immediate apply
 * ======================================================================== */
void updateEgtmAtom3phInvDuty(void)
{
    /* Increment with wrap at 100% for each phase independently */
    for (uint8 i = 0u; i < (uint8)NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + DUTY_UPDATE_STEP_PERCENT) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += DUTY_UPDATE_STEP_PERCENT;
    }

    /* Apply immediately to synchronized channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
