/*
 * egtm_atom_3_phase_inverter_pwm.c
 *
 * TC4xx eGTM ATOM 3-Phase Inverter PWM - Production Source
 *
 * Implements unified IfxEgtm_Pwm driver initialization and runtime update per SW Detailed Design.
 */

#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxCpu_Irq.h"
#include "IfxPort.h"

/* ======================== Pin Routing Placeholders (TBD) ======================== */
/*
 * Pin routing must be provided via valid IfxEgtm ATOM TOUT map symbols for your board.
 * Example format:
 *   #define PHASE_U_HS   (&IfxEgtm_ATOM0_0_TOUTxxx_Pyy_z_OUT)
 *   #define PHASE_U_LS   (&IfxEgtm_ATOM0_1_TOUTxxx_Pyy_z_OUT)  // Complementary
 *
 * Until assigned, default these to NULL to allow compilation in environments that mock iLLD.
 * Replace NULL with actual PinMap symbols in production builds on hardware.
 */
#ifndef PHASE_U_HS
#define PHASE_U_HS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif
#ifndef PHASE_U_LS
#define PHASE_U_LS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif
#ifndef PHASE_V_HS
#define PHASE_V_HS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif
#ifndef PHASE_V_LS
#define PHASE_V_LS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif
#ifndef PHASE_W_HS
#define PHASE_W_HS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif
#ifndef PHASE_W_LS
#define PHASE_W_LS   ((IfxEgtm_Pwm_ToutMap*)0)
#endif

/* ======================== Local Types and State ======================== */

typedef struct {
    IfxEgtm_Pwm          pwm;                          /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];   /* Channel data after init */
    float32              dutyCycles[NUM_OF_CHANNELS];  /* Duty cycle values in percent */
    float32              phases[NUM_OF_CHANNELS];      /* Phase shift values (deg or sec as configured) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];  /* Dead-time values (rising/falling) */
} EgtmAtom3phInv;

static EgtmAtom3phInv g_egtmAtom3phInv;  /* Driver instance */

/* ======================== Forward Declarations (private) ======================== */
static void configureEgtmClocks(void);
static void errorStatusHook(uint32 errorCode);

/* ======================== Period Event Callback ======================== */
void IfxEgtm_periodEventFunction(void *data)
{
    /* Period event hook (optional user extension point) */
    (void)data;
}

/* ======================== ISR ======================== */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    /* Minimal ISR: service unified driver and toggle LED */
    IfxEgtm_Pwm_interruptHandler(&g_egtmAtom3phInv.channels[0], NULL_PTR);
    IfxPort_togglePin(LED);
}

/* ======================== Private Functions ======================== */

/* Configure eGTM CMU clocks: set GCLK to 300 MHz (expected), select CLK0 from GCLK, set CLK0 to 100 MHz, enable CLK0 */
static void configureEgtmClocks(void)
{
    /* Enable the eGTM module */
    IfxEgtm_enable(&MODULE_EGTM);

    /* Program GCLK to expected 300 MHz (project requirement) */
    IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, (float32)SYSTEM_GCLK_TARGET_HZ);

    /* Route CLK0 from global clock and set divider/count to derive 100 MHz */
    IfxEgtm_Cmu_selectClkInput(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, TRUE);

    {
        float32 gclkHz = IfxEgtm_Cmu_getGclkFrequency(&MODULE_EGTM);
        float32 target = (float32)EGTM_CLK0_TARGET_HZ;
        uint32  count  = (uint32)((gclkHz / ((target > 0.0f) ? target : 1.0f)));
        if (count == 0U) { count = 1U; }
        IfxEgtm_Cmu_setClkCount(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, count);
    }

    /* Enable CLK0; FXCLK enable is device-specific - enable at minimum CLK0 as required */
    IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
}

/* Error/status hook (stub). Extend to SMU as needed. */
static void errorStatusHook(uint32 errorCode)
{
    (void)errorCode; /* Suppress unused warning in stub */
}

/* ======================== Public Functions ======================== */

void initEgtmAtom3phInv(void)
{
    /* Step 1: Mandatory eGTM enable + CMU clock setup */
    configureEgtmClocks();

    /* Step 2: Prepare main PWM configuration (unified driver) */
    IfxEgtm_Pwm_Config           config;
    IfxEgtm_Pwm_ChannelConfig    channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig     output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig        dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig  interruptConfig;

    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Step 3: Output configuration for complementary pairs (U, V, W) */
    /* Phase U */
    output[0].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_U_HS;
    output[0].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_U_LS;
    output[0].polarity                   = Ifx_ActiveState_high;  /* High-side non-inverted */
    output[0].complementaryPolarity      = Ifx_ActiveState_low;   /* Low-side inverted */
    output[0].outputMode                 = IfxPort_OutputMode_pushPull;
    output[0].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase V */
    output[1].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_V_HS;
    output[1].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_V_LS;
    output[1].polarity                   = Ifx_ActiveState_high;
    output[1].complementaryPolarity      = Ifx_ActiveState_low;
    output[1].outputMode                 = IfxPort_OutputMode_pushPull;
    output[1].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Phase W */
    output[2].pin                        = (IfxEgtm_Pwm_ToutMap*)PHASE_W_HS;
    output[2].complementaryPin           = (IfxEgtm_Pwm_ToutMap*)PHASE_W_LS;
    output[2].polarity                   = Ifx_ActiveState_high;
    output[2].complementaryPolarity      = Ifx_ActiveState_low;
    output[2].outputMode                 = IfxPort_OutputMode_pushPull;
    output[2].padDriver                  = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Step 4: DTM/CDTM dead-time (1 us rising/falling) */
    dtmConfig[0].deadTime.rising         = 1.0e-6f;
    dtmConfig[0].deadTime.falling        = 1.0e-6f;
    dtmConfig[1].deadTime.rising         = 1.0e-6f;
    dtmConfig[1].deadTime.falling        = 1.0e-6f;
    dtmConfig[2].deadTime.rising         = 1.0e-6f;
    dtmConfig[2].deadTime.falling        = 1.0e-6f;

    /* Step 5: Channel configurations (base + two sync channels) */
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[0]);
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[1]);
    IfxEgtm_Pwm_initChannelConfig(&channelConfig[2]);

    /* Interrupt configuration for period event on base channel only */
    interruptConfig.mode                 = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider          = IfxSrc_Tos_cpu0;
    interruptConfig.priority             = ISR_PRIORITY_ATOM;
    interruptConfig.vmId                 = IfxSrc_VmId_0;
    interruptConfig.periodEvent          = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent            = NULL_PTR;

    /* Base channel: ATOM0 Ch0 (U) */
    channelConfig[0].timerCh             = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase               = 0.0f;
    channelConfig[0].duty                = 0.0f;  /* Initial duty per requirements */
    channelConfig[0].dtm                 = &dtmConfig[0];
    channelConfig[0].output              = &output[0];
    channelConfig[0].mscOut              = NULL_PTR;
    channelConfig[0].interrupt           = &interruptConfig;  /* Period IRQ on base */

    /* Sync channel: ATOM0 Ch2 (V) */
    channelConfig[1].timerCh             = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[1].phase               = 0.0f;
    channelConfig[1].duty                = 0.0f;
    channelConfig[1].dtm                 = &dtmConfig[1];
    channelConfig[1].output              = &output[1];
    channelConfig[1].mscOut              = NULL_PTR;
    channelConfig[1].interrupt           = NULL_PTR;

    /* Sync channel: ATOM0 Ch4 (W) */
    channelConfig[2].timerCh             = IfxEgtm_Pwm_SubModule_Ch_4;
    channelConfig[2].phase               = 0.0f;
    channelConfig[2].duty                = 0.0f;
    channelConfig[2].dtm                 = &dtmConfig[2];
    channelConfig[2].output              = &output[2];
    channelConfig[2].mscOut              = NULL_PTR;
    channelConfig[2].interrupt           = NULL_PTR;

    /* Step 6: Global PWM configuration */
    config.cluster                       = IfxEgtm_Cluster_0;
    config.subModule                     = IfxEgtm_Pwm_SubModule_atom;
    config.alignment                     = IfxEgtm_Pwm_Alignment_center;
    config.syncStart                     = TRUE;   /* Sync start */
    config.syncUpdateEnabled             = TRUE;   /* Shadow update */
    config.numChannels                   = NUM_OF_CHANNELS;
    config.channels                      = &channelConfig[0];
    config.frequency                     = PWM_FREQUENCY;        /* 20 kHz */
    config.clockSource.atom              = IfxEgtm_Cmu_Clk_0;    /* ATOM uses Clk enum */
    config.dtmClockSource                = IfxEgtm_Dtm_ClockSource_cmuClock0;

    /* Route ISR (install handler) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptEgtmAtom, ISR_PRIORITY_ATOM);

    /* Step 7: Initialize PWM driver and start synchronously */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    /* Store initial runtime state */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Step 8: Configure status LED */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

void updateEgtmAtom3phInvDuty(void)
{
    /* Ramp duties independently with wrap, then apply immediately */
    uint8 i;
    for (i = 0U; i < NUM_OF_CHANNELS; i++)
    {
        if ((g_egtmAtom3phInv.dutyCycles[i] + (float32)PHASE_DUTY_STEP) >= 100.0f)
        {
            g_egtmAtom3phInv.dutyCycles[i] = 0.0f;
        }
        g_egtmAtom3phInv.dutyCycles[i] += (float32)PHASE_DUTY_STEP;
    }

    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32*)g_egtmAtom3phInv.dutyCycles);
}
