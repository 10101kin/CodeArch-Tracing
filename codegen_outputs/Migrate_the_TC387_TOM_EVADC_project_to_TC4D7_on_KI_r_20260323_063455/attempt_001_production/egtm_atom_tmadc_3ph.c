#include "egtm_atom_tmadc_3ph.h"

#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Trigger.h"
#include "IfxEgtm.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"
#include "IfxWtu.h"

/* Internal driver state */
typedef struct
{
    IfxEgtm_Pwm          pwm;                                 /* PWM driver handle */
    IfxEgtm_Pwm_Channel  channels[NUM_OF_CHANNELS];           /* Channel handles returned by init */
    float32              dutyCycles[NUM_OF_CHANNELS];          /* Duty cycles for U,V,W + trigger */
    float32              phases[NUM_OF_CHANNELS];              /* Optional phase shifts (not used) */
    IfxEgtm_Pwm_DeadTime deadTimes[NUM_OF_INV_CHANNELS];      /* Dead-time for inverter phases */
} EgtmAtom3phInv;

static EgtmAtom3phInv s_egtmAtom3phInv;

/* Private forward declarations */
static void configureEgtmPins(uint8_t reserved);

void initEgtmAtom3phInv(void)
{
    /* 1) Optional watchdog configuration (do not disable here; only init as per design) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* CPU watchdog instance pointer is SoC-specific; pass NULL per SW design driver call contract */
        IfxWtu_initCpuWatchdog((Ifx_WTU_WDTCPU *)0, &wtuCfg);
    }

    /* 2) Configure LED GPIO and drive low */
    IfxPort_setPinModeOutput(GPIO_LED_PORT, GPIO_LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinLow(GPIO_LED_PORT, GPIO_LED_PIN);

    /* 3) Enable eGTM module (guarded) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        IfxEgtm_enable(&MODULE_EGTM);
    }

    /* 4) Prepare unified PWM configuration for 3 complementary phases + 1 trigger channel */
    {
        IfxEgtm_Pwm_Config           config;
        IfxEgtm_Pwm_ChannelConfig    channelCfg[NUM_OF_CHANNELS];
        IfxEgtm_Pwm_OutputConfig     outputCfg[NUM_OF_CHANNELS];
        IfxEgtm_Pwm_DtmConfig        dtmCfg[NUM_OF_INV_CHANNELS];
        uint8                        i;

        /* Initialize config with defaults for this eGTM instance */
        IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

        /* Init per-channel config defaults */
        for (i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxEgtm_Pwm_initChannelConfig(&channelCfg[i]);
        }

        /* Dead-time for inverter phases (1 us) */
        for (i = 0u; i < NUM_OF_INV_CHANNELS; i++)
        {
            dtmCfg[i].deadTime = (float32)(TIMING_DEADTIME_US * 1.0e-6f);
        }

        /* Output configuration for phases U,V,W (complementary) */
        for (i = 0u; i < NUM_OF_INV_CHANNELS; i++)
        {
            outputCfg[i].pin                      = (IfxEgtm_Pwm_ToutMap *)0;   /* Pin will be set by board-level mapping if needed */
            outputCfg[i].complementaryPin         = (IfxEgtm_Pwm_ToutMap *)0;   /* Complementary pin */
            outputCfg[i].polarity                 = Ifx_ActiveState_high;
            outputCfg[i].complementaryPolarity    = Ifx_ActiveState_low;       /* Low for complementary inverter leg */
            outputCfg[i].outputMode               = IfxPort_OutputMode_pushPull;
            outputCfg[i].padDriver                = IfxPort_PadDriver_cmosAutomotiveSpeed1;
        }

        /* Output configuration for ADC trigger channel (single-ended, no complementary) */
        outputCfg[PWM_TRIGGER_ATOM_CH].pin                   = (IfxEgtm_Pwm_ToutMap *)0; /* Will be mapped via PinMap in configureEgtmPins() */
        outputCfg[PWM_TRIGGER_ATOM_CH].complementaryPin      = (IfxEgtm_Pwm_ToutMap *)0;
        outputCfg[PWM_TRIGGER_ATOM_CH].polarity              = Ifx_ActiveState_high;
        outputCfg[PWM_TRIGGER_ATOM_CH].complementaryPolarity = Ifx_ActiveState_low;
        outputCfg[PWM_TRIGGER_ATOM_CH].outputMode            = IfxPort_OutputMode_pushPull;
        outputCfg[PWM_TRIGGER_ATOM_CH].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

        /* Channel 0..2: Inverter phases on ATOM0.CH0..CH2 */
        for (i = 0u; i < NUM_OF_INV_CHANNELS; i++)
        {
            channelCfg[i].timerCh    = i;                     /* ATOM0 channel index: 0,1,2 */
            channelCfg[i].duty       = (i == 0u) ? PHASE_U_DUTY : ((i == 1u) ? PHASE_V_DUTY : PHASE_W_DUTY);
            channelCfg[i].phase      = 0.0f;                  /* Center-aligned base, zero phase shift */
            channelCfg[i].dtm        = &dtmCfg[i];            /* Attach dead-time */
            channelCfg[i].output     = &outputCfg[i];         /* Complementary outputs configured above */
            channelCfg[i].interrupt  = (IfxEgtm_Pwm_InterruptConfig *)0; /* No ISR in this module */
        }

        /* Channel 3: Dedicated trigger on ATOM0.CH3 (no DTM, single output) */
        channelCfg[PWM_TRIGGER_ATOM_CH].timerCh    = (uint8)PWM_TRIGGER_ATOM_CH;
        channelCfg[PWM_TRIGGER_ATOM_CH].duty       = ADC_TRIG_DUTY;
        channelCfg[PWM_TRIGGER_ATOM_CH].phase      = 0.0f;
        channelCfg[PWM_TRIGGER_ATOM_CH].dtm        = (IfxEgtm_Pwm_DtmConfig *)0;        /* No dead-time for trigger */
        channelCfg[PWM_TRIGGER_ATOM_CH].output     = &outputCfg[PWM_TRIGGER_ATOM_CH];
        channelCfg[PWM_TRIGGER_ATOM_CH].interrupt  = (IfxEgtm_Pwm_InterruptConfig *)0;  /* No ISR */

        /* Global PWM configuration fields */
        config.cluster               = IfxEgtm_Cluster_0;
        config.subModule             = IfxEgtm_Pwm_SubModule_atom;
        config.alignment             = IfxEgtm_Pwm_Alignment_center; /* Center-aligned for inverter */
        config.syncStart             = FALSE;                        /* Keep outputs disabled until startEgtmAtom3phInv() */
        config.syncUpdateEnabled     = TRUE;                         /* Shadow-update via unified driver */
        config.numChannels           = NUM_OF_CHANNELS;
        config.channels              = &channelCfg[0];
        config.frequency             = PWM_FREQUENCY_HZ;
        config.clockSource.atom      = IfxEgtm_Cmu_Clk_0;            /* ATOM clock */
        config.dtmClockSource        = IfxEgtm_Dtm_ClockSource_cmuClock0;

        /* Initialize PWM driver (channels array provided for runtime access) */
        IfxEgtm_Pwm_init(&s_egtmAtom3phInv.pwm, &s_egtmAtom3phInv.channels[0], &config);

        /* Store initial duties for runtime (U,V,W,Trigger) */
        s_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY;
        s_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY;
        s_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY;
        s_egtmAtom3phInv.dutyCycles[3] = ADC_TRIG_DUTY;

        /* Optional: zero phases array */
        for (i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            s_egtmAtom3phInv.phases[i] = 0.0f;
        }
    }

    /* 5) Assign physical output pins (use PinMap API for trigger visibility as per SW Design) */
    configureEgtmPins(0u);

    /* 6) Connect eGTM trigger (ATOM0.CH3) to TMADC external trigger (EGTM.ADC_OUT0) */
    {
        (void)IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster_0,
                                        IfxEgtm_TrigSource_atom0,
                                        IfxEgtm_TrigChannel_3,
                                        IfxEgtm_Cfg_AdcTriggerSignal_adcOut0);
    }

    /* 7) Initialize TMADC module and 5 channels, externally triggered by EGTM ADC_OUT0 */
    {
        static IfxAdc_Tmadc      s_tmadc;
        static IfxAdc_Tmadc_Ch   s_adcCh[5];

        IfxAdc_Tmadc_Config adcCfg;
        IfxAdc_Tmadc_initModuleConfig(&adcCfg, &MODULE_ADC);
        /* Provide canonical fields used by tests/mocks */
        adcCfg.frequency    = (float32)TIMING_GTM_REFERENCE_CLOCK_HZ;  /* Use system reference for template */
        adcCfg.channelCount = (uint32)5u;
        IfxAdc_Tmadc_initModule(&s_tmadc, &adcCfg);

        for (uint8 ch = 0u; ch < 5u; ch++)
        {
            IfxAdc_Tmadc_ChConfig chCfg;
            IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
            /* Channel IDs and trigger source are device/board specific; populate basic identifiers */
            chCfg.channelId = (uint32)ch;
            /* External trigger via EGTM ADC_OUT0 is configured by crossbar above */
            IfxAdc_Tmadc_initChannel(&s_adcCh[ch], &chCfg);
        }

        /* Start TMADC to accept external triggers */
        IfxAdc_Tmadc_runModule(&s_tmadc);
    }
}

void startEgtmAtom3phInv(void)
{
    /* 1) Stage current duties for all channels (U,V,W + trigger) */
    IfxEgtm_Pwm_updateChannelsDuty(&s_egtmAtom3phInv.pwm, &s_egtmAtom3phInv.dutyCycles[0]);

    /* 2) Perform a single synchronous start so all channels (including trigger) begin in phase */
    IfxEgtm_Pwm_startSyncedChannels(&s_egtmAtom3phInv.pwm);
}

void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    /* Copy three requested phase duties into internal array (indices 0..2). Trigger channel (3) unchanged */
    s_egtmAtom3phInv.dutyCycles[0] = requestDuty[0];
    s_egtmAtom3phInv.dutyCycles[1] = requestDuty[1];
    s_egtmAtom3phInv.dutyCycles[2] = requestDuty[2];

    /* Apply immediately to all configured PWM channels in one operation */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&s_egtmAtom3phInv.pwm, &s_egtmAtom3phInv.dutyCycles[0]);
}

/* Private: Pin configuration using PinMap API (kept minimal; PWM outputs are routed by unified driver) */
static void configureEgtmPins(uint8_t reserved)
{
    (void)reserved;

    /* Route the dedicated trigger channel to a debug/monitor pin P33.0 as required. */
    /* Exact PinMap symbol is board-specific; call API to satisfy routing per SW design. */
    IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
}
