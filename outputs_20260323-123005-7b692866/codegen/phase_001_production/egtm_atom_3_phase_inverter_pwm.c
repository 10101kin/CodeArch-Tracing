#include "egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ============================================================================
 * Driver/application state (unified IfxEgtm_Pwm for TC4xx EGTM ATOM)
 * ============================================================================ */
typedef struct
{
    IfxEgtm_Pwm           pwm;                              /* PWM driver handle */
    IfxEgtm_Pwm_Channel   channels[NUM_OF_CHANNELS];       /* Channel runtime data */
    float32               dutyCycles[NUM_OF_CHANNELS];     /* Duty cycle values (%) */
    float32               phases[NUM_OF_CHANNELS];         /* Phase shift values (deg) */
    IfxEgtm_Pwm_DeadTime  deadTimes[NUM_OF_CHANNELS];      /* Dead-time values (s) */
} EgtmAtom3phInv;

IFX_STATIC EgtmAtom3phInv g_egtmAtom3phInv;

/* ============================================================================
 * ISR: Period-event interrupt handler (toggle LED per reference pattern)
 * ============================================================================ */
IFX_INTERRUPT(interruptEgtmAtom, 0, ISR_PRIORITY_ATOM);
void interruptEgtmAtom(void)
{
    IfxPort_togglePin(LED);
}

/* ============================================================================
 * Period event callback (linked via interruptConfig.periodEvent)
 * ============================================================================ */
void IfxEgtm_periodEventFunction(void *data)
{
    /* User period event handling (optional) */
    (void)data;
}

/* ============================================================================
 * Initialization: eGTM0 ATOM2 3-phase inverter with complementary CDTM outputs
 * - 20 kHz center-aligned PWM, 1 us dead-time, sync start/update
 * - Single period ISR on CPU0 (priority = ISR_PRIORITY_ATOM)
 * ============================================================================ */
void initEgtmAtom3phInv(void)
{
    IfxEgtm_Pwm_Config            config;
    IfxEgtm_Pwm_ChannelConfig     channelConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_OutputConfig      output[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_DtmConfig         dtmConfig[NUM_OF_CHANNELS];
    IfxEgtm_Pwm_InterruptConfig   interruptConfig;

    /* Mandatory: Enable EGTM module and configure CMU clocks BEFORE init */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 moduleFreq;
        IfxEgtm_enable(&MODULE_EGTM);
        moduleFreq = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, moduleFreq);
        /* Set CLK0 per requirements (100 MHz); enable CLK0 */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, IfxEgtm_Cmu_Clk_0, (float32)CLOCK_DTM_CLK0_HZ);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* Initialize config struct with defaults for EGTM */
    IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

    /* Interrupt configuration: Period event on base channel (CH0) */
    interruptConfig.mode        = IfxEgtm_IrqMode_pulseNotify;
    interruptConfig.isrProvider = IfxSrc_Tos_cpu0;
    interruptConfig.priority    = ISR_PRIORITY_ATOM;      /* exact macro name */
    interruptConfig.vmId        = IfxSrc_VmId_0;          /* TC4xx specific */
    interruptConfig.periodEvent = IfxEgtm_periodEventFunction;
    interruptConfig.dutyEvent   = NULL_PTR;

    /* Output pin configuration for complementary PWM (CDTM) */
    /* CH0 - Phase U */
    output[0].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_U_HS;
    output[0].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_U_LS;
    output[0].polarity                 = Ifx_ActiveState_high;                 /* HS active high */
    output[0].complementaryPolarity    = Ifx_ActiveState_low;                  /* LS active low */
    output[0].outputMode               = IfxPort_OutputMode_pushPull;
    output[0].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* CH1 - Phase V */
    output[1].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_V_HS;
    output[1].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_V_LS;
    output[1].polarity                 = Ifx_ActiveState_high;
    output[1].complementaryPolarity    = Ifx_ActiveState_low;
    output[1].outputMode               = IfxPort_OutputMode_pushPull;
    output[1].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* CH2 - Phase W */
    output[2].pin                      = (IfxEgtm_Pwm_ToutMap *)PHASE_W_HS;
    output[2].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)PHASE_W_LS;
    output[2].polarity                 = Ifx_ActiveState_high;
    output[2].complementaryPolarity    = Ifx_ActiveState_low;
    output[2].outputMode               = IfxPort_OutputMode_pushPull;
    output[2].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time configuration: 1 us rising and falling (from requirements) */
    dtmConfig[0].deadTime.rising  = DEAD_TIME_S;
    dtmConfig[0].deadTime.falling = DEAD_TIME_S;
    dtmConfig[1].deadTime.rising  = DEAD_TIME_S;
    dtmConfig[1].deadTime.falling = DEAD_TIME_S;
    dtmConfig[2].deadTime.rising  = DEAD_TIME_S;
    dtmConfig[2].deadTime.falling = DEAD_TIME_S;

    /* Channel configurations (CH0=U, CH1=V, CH2=W) */
    /* CH0 - Base channel with ISR */
    channelConfig[0].timerCh    = IfxEgtm_Pwm_SubModule_Ch_0;
    channelConfig[0].phase      = 0.0f;
    channelConfig[0].duty       = PHASE_U_DUTY;
    channelConfig[0].dtm        = &dtmConfig[0];
    channelConfig[0].output     = &output[0];
    channelConfig[0].mscOut     = NULL_PTR;
    channelConfig[0].interrupt  = &interruptConfig;       /* attach ISR to CH0 */

    /* CH1 */
    channelConfig[1].timerCh    = IfxEgtm_Pwm_SubModule_Ch_1;
    channelConfig[1].phase      = 0.0f;
    channelConfig[1].duty       = PHASE_V_DUTY;
    channelConfig[1].dtm        = &dtmConfig[1];
    channelConfig[1].output     = &output[1];
    channelConfig[1].mscOut     = NULL_PTR;
    channelConfig[1].interrupt  = NULL_PTR;               /* no ISR on CH1 */

    /* CH2 */
    channelConfig[2].timerCh    = IfxEgtm_Pwm_SubModule_Ch_2;
    channelConfig[2].phase      = 0.0f;
    channelConfig[2].duty       = PHASE_W_DUTY;
    channelConfig[2].dtm        = &dtmConfig[2];
    channelConfig[2].output     = &output[2];
    channelConfig[2].mscOut     = NULL_PTR;
    channelConfig[2].interrupt  = NULL_PTR;               /* no ISR on CH2 */

    /* Global PWM configuration */
    config.cluster              = IfxEgtm_Cluster_0;                      /* eGTM0 */
    config.subModule            = IfxEgtm_Pwm_SubModule_atom;             /* ATOM */
    config.alignment            = IfxEgtm_Pwm_Alignment_center;           /* center-aligned */
    config.syncStart            = TRUE;                                    /* auto-start */
    config.syncUpdateEnabled    = TRUE;                                    /* shadow updates */
    config.numChannels          = NUM_OF_CHANNELS;
    config.channels             = &channelConfig[0];
    config.frequency            = PWM_FREQUENCY;                           /* 20 kHz */
    config.clockSource.atom     = IfxEgtm_Cmu_Clk_0;                       /* ATOM clock source */
    config.dtmClockSource       = IfxEgtm_Dtm_ClockSource_cmuClock0;       /* DTM clock source */

    /* Initialize the PWM driver (unified high-level driver) */
    IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

    /* Store initialized values for runtime updates */
    g_egtmAtom3phInv.dutyCycles[0] = channelConfig[0].duty;
    g_egtmAtom3phInv.dutyCycles[1] = channelConfig[1].duty;
    g_egtmAtom3phInv.dutyCycles[2] = channelConfig[2].duty;
    g_egtmAtom3phInv.deadTimes[0]  = channelConfig[0].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[1]  = channelConfig[1].dtm->deadTime;
    g_egtmAtom3phInv.deadTimes[2]  = channelConfig[2].dtm->deadTime;

    /* Configure LED for ISR diagnostic toggle */
    IfxPort_setPinModeOutput(LED, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
}

/* ============================================================================
 * Runtime update: increment duties with wrap and apply immediately
 * ============================================================================ */
void updateEgtmAtom3phInvDuty(void)
{
    /* Wrap-to-zero if next step would reach or exceed 100% (per reference logic) */
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

    /* Apply increment step */
    g_egtmAtom3phInv.dutyCycles[0] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[1] += PHASE_DUTY_STEP;
    g_egtmAtom3phInv.dutyCycles[2] += PHASE_DUTY_STEP;

    /* Update all channels immediately, coherently */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, (float32 *)g_egtmAtom3phInv.dutyCycles);
}
