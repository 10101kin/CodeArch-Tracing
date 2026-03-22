/*
 * egtm_atom_3_phase_inverter_pwm.c
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxEgtm.h"

/* =============================
 * Pin Routing (EGTM ATOM0, Cluster_0) - adapt as needed per hardware
 * Using complementary mapping via *_N symbols.
 * U: ch0, V: ch2, W: ch4
 * ============================= */
#define PHASE_U_HS   &IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
#define PHASE_U_LS   &IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
#define PHASE_V_HS   &IfxEgtm_ATOM0_2_TOUT2_P02_2_OUT
#define PHASE_V_LS   &IfxEgtm_ATOM0_1N_TOUT3_P02_3_OUT
#define PHASE_W_HS   &IfxEgtm_ATOM0_4_TOUT4_P02_4_OUT
#define PHASE_W_LS   &IfxEgtm_ATOM0_2N_TOUT5_P02_5_OUT

/* =============================
 * Driver/application state (unified driver handle + runtime arrays)
 * ============================= */

typedef struct {
    IfxEgtm_Pwm          pwm;                                /* PWM Driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];          /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];         /* Duty cycle values (%) */
    float32              phases[NUM_OF_CHANNELS];             /* Phase shift values */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];         /* Dead time values (s) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv; /* internal driver instance */

/* =============================
 * Forward statics
 * ============================= */
static void configureEgtmClocks(void);
static void errorStatusHook(uint32 errorCode);

/* =============================
 * ISR: Period event (base channel)
 * ============================= */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Minimal ISR: delegate to unified driver handler + optional LED toggle */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
    IfxPort_togglePin(LED);
}

/* =============================
 * Period event callback (linked via interrupt config)
 * ============================= */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Placeholder: user period callback (optional) */
    (void)data;
}

/* =============================
 * Private helpers
 * ============================= */
static void configureEgtmClocks(void)
{
    /* Enable eGTM and configure GCLK and CLK0 for 100 MHz */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* Set GCLK to module frequency */
    {
        float32 moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
    }

    /* Route CLK0 from GCLK and set divider to achieve 100 MHz */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE); /* useGlobal = TRUE */
    {
        float32 gclk = IfxEgtm_Cmu_getGclkFrequency(&MODULE_EGTM);
        float32 desired = 100000000.0f; /* 100 MHz */
        uint32  count;
        if (gclk > desired)
        {
            /* Typical: f_clk = GCLK / (count + 1) */
            count = (uint32)((gclk / desired) - 1.0f);
        }
        else
        {
            /* If GCLK <= desired, set count = 0 (no division) */
            count = 0u;
        }
        IfxEgtm_Cmu_setClkCount(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, count);
    }

    /* Enable CLK0 (and related clocking as required by platform) */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)IFXEGTM_CMU_CLKEN_CLK0);
}

static void errorStatusHook(uint32 errorCode)
{
    (void)errorCode; /* Stub: extend with SMU integration if needed */
}

/* =============================
 * Init: Configure 3-phase inverter on ATOM with complementary outputs
 * ============================= */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    /* 1) eGTM enable + CMU clocks (GCLK, CLK0=100 MHz) */
    configureEgtmClocks();

    /* 2) Initialize unified PWM config */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* 3) Output configuration per phase (route HS/LS via OutputConfig pins) */
    output[0].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;  /* U */
    output[0].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity               = Ifx_ActiveState_high;              /* HS non-inverted */
    output[0].complementaryPolarity  = Ifx_ActiveState_low;               /* LS inverted */
    output[0].outputMode             = IfxPort_OutputMode_pushPull;
    output[0].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[1].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;  /* V */
    output[1].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity               = Ifx_ActiveState_high;
    output[1].complementaryPolarity  = Ifx_ActiveState_low;
    output[1].outputMode             = IfxPort_OutputMode_pushPull;
    output[1].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    output[2].pin                    = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;  /* W */
    output[2].complementaryPin       = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity               = Ifx_ActiveState_high;
    output[2].complementaryPolarity  = Ifx_ActiveState_low;
    output[2].outputMode             = IfxPort_OutputMode_pushPull;
    output[2].padDriver              = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* 4) Dead-time (CDTM) per phase: rising/falling = 1 us */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[0].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[1].deadTime.falling = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_SEC;
    dtmConfig[2].deadTime.falling = DEAD_TIME_SEC;

    /* 5) Channel configuration (base + sync channels) */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);
        channelConfig[i].phase  = 0.0f;
        channelConfig[i].mscOut = NULL_PTR;
    }

    /* Assign ATOM0 base channels for 3 complementary pairs: U:ch0, V:ch2, W:ch4 */
    channelConfig[0].timerCh   = IfxEgtm_Pwm_SubModule_Ch_0;   /* U */
    channelConfig[0].duty      = DUTY_INIT_U_PERCENT;
    channelConfig[0].dtm       = &dtmConfig[0];
    channelConfig[0].output    = &output[0];

    channelConfig[1].timerCh   = IfxEgtm_Pwm_SubModule_Ch_2;   /* V */
    channelConfig[1].duty      = DUTY_INIT_V_PERCENT;
    channelConfig[1].dtm       = &dtmConfig[1];
    channelConfig[1].output    = &output[1];

    channelConfig[2].timerCh   = IfxEgtm_Pwm_SubModule_Ch_4;   /* W */
    channelConfig[2].duty      = DUTY_INIT_W_PERCENT;
    channelConfig[2].dtm       = &dtmConfig[2];
    channelConfig[2].output    = &output[2];

    /* 6) Interrupt configuration: period event on base channel (cpu0, prio 20) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;
    interruptConfig.vmId        = IfxSrc_VmId_0;  /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    channelConfig[0].interrupt = &interruptConfig;  /* Base channel ISR */
    channelConfig[1].interrupt = NULL_PTR;
    channelConfig[2].interrupt = NULL_PTR;

    /* 7) Main PWM config */
    config.cluster            = IfxEgtm_Cluster_0;
    config.subModule          = IfxEgtm_Pwm_SubModule_atom;
    config.alignment          = IfxEgtm_Pwm_Alignment_center;
    config.syncStart          = TRUE;                    /* auto-start after init */
    config.syncUpdateEnabled  = TRUE;                    /* shadow updates */
    config.numChannels        = NUM_OF_CHANNELS;
    config.channels           = &channelConfig[0];
    config.frequency          = PWM_FREQUENCY_HZ;
    config.clockSource.atom   = IfxEgtm_Cmu_Clk_0;       /* ATOM uses Clk enum */
    config.dtmClockSource     = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Initialize PWM driver (no return value) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store runtime copies */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;

    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Install ISR handler (vector table) */
    IfxCpu_Irq_installInterruptHandler((void*)&interruptEgtmAtom, (uint32)ISR_PRIORITY_ATOM);

    /* Status LED output config */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* =============================
 * Runtime duty update (independent U/V/W ramp with wrap)
 * ============================= */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap if next step crosses 100%, then increment */
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

    /* Apply immediately to synchronized channels */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
