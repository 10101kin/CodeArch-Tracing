#include "egtm_atom_3_phase_inverter.h"
#include "IfxAdc_Tmadc.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Dtm.h"
#include "IfxEgtm_PinMap.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxPort.h"
#include "IfxWtu.h"

/* Internal driver state */
static EgtmAtom3phInv g_egtmAtom3phInv;  /* Zero-initialized by default */
static IfxAdc_Tmadc   g_tmadc;           /* TMADC module handle */
static boolean        s_initialized = FALSE;

/*
 * Compute DTM deadtime ticks from microseconds, using the requirement's DTM clock assumption.
 * Assumption: DTM clock = 100 MHz => 1 us = 100 ticks.
 */
static uint16 calcDeadtimeTicks(float32 deadtimeUs)
{
    float32 ticksF = deadtimeUs * DTM_CLOCK_ASSUMPTION_MHZ; /* ticks = us * MHz */
    if (ticksF < 0.0f)
    {
        ticksF = 0.0f;
    }
    uint16 ticks = (uint16)(ticksF + 0.5f); /* round to nearest */
    return ticks;
}

/* Initialize the eGTM and TMADC subsystems and configure PWM and GPIO according to 3-phase inverter requirements. */
void initEgtmAtom3phInv(void)
{
    /* 1) Initialize watchdog configuration structures via WTU (no disable here; per platform policy) */
    {
        IfxWtu_Config wtuCfg;
        IfxWtu_initConfig(&wtuCfg);
        /* Platform-specific WTU instances are provided by BSP; using NULL here as placeholder, per iLLD signature */
        IfxWtu_initCpuWatchdog((Ifx_WTU_WDTCPU *)0, &wtuCfg);
        IfxWtu_initSystemWatchdog((Ifx_WTU_WDTSYS *)0, &wtuCfg);
    }

    /* 2) Enable the eGTM module and its clocks (MANDATORY for TC4xx/EGTM) */
    if (!IfxEgtm_isEnabled(&MODULE_EGTM))
    {
        float32 frequency;
        IfxEgtm_enable(&MODULE_EGTM);
        frequency = IfxEgtm_Cmu_getModuleFrequency(&MODULE_EGTM);
        IfxEgtm_Cmu_setGclkFrequency(&MODULE_EGTM, frequency);
        /* CLK0 set to module frequency (align with requirements) */
        IfxEgtm_Cmu_setClkFrequency(&MODULE_EGTM, (IfxEgtm_Cmu_Clk)EGTM_CLOCK_CLK0, frequency);
        IfxEgtm_Cmu_enableClocks(&MODULE_EGTM, (uint32)IFXEGTM_CMU_CLKEN_CLK0);
    }

    /* 3) Build unified PWM configuration for three synchronous ATOM channels in the same cluster */
    {
        IfxEgtm_Pwm_Config          config;
        IfxEgtm_Pwm_ChannelConfig   channelConfig[NUM_OF_CHANNELS];
        IfxEgtm_Pwm_OutputConfig    output[NUM_OF_CHANNELS];
        IfxEgtm_Pwm_DtmConfig       dtmConfig[NUM_OF_CHANNELS];

        /* Initialize config with module reference */
        IfxEgtm_Pwm_initConfig(&config, &MODULE_EGTM);

        /* Global group settings per requirements */
        config.cluster            = (IfxEgtm_Cluster)EGTM_CLUSTER_0;
        config.subModule          = (IfxEgtm_Pwm_SubModule)PWM_SUBMODULE_ATOM; /* ATOM submodule */
        config.alignment          = (IfxEgtm_Pwm_Alignment)PWM_ALIGNMENT_CENTER; /* center-aligned */
        config.syncStart          = TRUE;   /* start channels in sync after init */
        config.syncUpdateEnabled  = TRUE;   /* shadow register update enabled */
        config.numChannels        = NUM_OF_CHANNELS;
        config.channels           = &channelConfig[0];
        config.frequency          = PWM_FREQUENCY_HZ;
        config.clockSource.atom   = (IfxEgtm_Cmu_Clk)EGTM_CLOCK_CLK0;
        config.dtmClockSource     = (IfxEgtm_Dtm_ClockSource)EGTM_DTM_CLOCK_CMUCLK0;

        /* Prepare per-channel configuration */
        uint16 deadTicks = calcDeadtimeTicks(PWM_DEADTIME_US);

        for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
        {
            IfxEgtm_Pwm_initChannelConfig(&channelConfig[i]);

            /* Attach output config: complementary output with dead-time via DTM/CDTM */
            output[i].pin                     = (IfxEgtm_Pwm_ToutMap *)0; /* Pin assignment via OutputConfig; provide mapping symbol here */
            output[i].complementaryPin        = (IfxEgtm_Pwm_ToutMap *)0; /* Complementary pin mapping symbol here */
            output[i].polarity                = Ifx_ActiveState_high;     /* main output active-high */
            output[i].complementaryPolarity   = Ifx_ActiveState_low;      /* complementary active-low for inverter */
            output[i].outputMode              = IfxPort_OutputMode_pushPull;
            output[i].padDriver               = IfxPort_PadDriver_cmosAutomotiveSpeed1;

            /* Link output + DTM to channel config */
            channelConfig[i].output  = &output[i];
            channelConfig[i].dtm     = &dtmConfig[i];

            /* Select ATOM timer channel index within cluster 0: CH0, CH1, CH2 */
            channelConfig[i].timerCh = (uint8)i;  /* cast to the driver's expected channel index */

            /* Initial duty per requirements */
            if (i == 0u)      { channelConfig[i].duty = PHASE_U_DUTY_PERCENT; }
            else if (i == 1u) { channelConfig[i].duty = PHASE_V_DUTY_PERCENT; }
            else              { channelConfig[i].duty = PHASE_W_DUTY_PERCENT; }

            /* No phase shift required for U/V/W base config */
            channelConfig[i].phase = 0.0f;

            /* Program DTM settings for rise/fall dead-time via low-level APIs (CDTM mode) */
            /* Note: DTM instance pointer is SoC-specific; using 0 as placeholder per signature. */
            IfxEgtm_Dtm_setClockSource((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_ClockSource)EGTM_DTM_CLOCK_CMUCLK0);
            IfxEgtm_Dtm_setRelrise((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_Ch)i, deadTicks);
            IfxEgtm_Dtm_setRelfall((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_Ch)i, deadTicks);
            IfxEgtm_Dtm_setOutput1Function((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_Ch)i, (IfxEgtm_Dtm_Output1Function)0);
            IfxEgtm_Dtm_setOutput1Select((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_Ch)i, (IfxEgtm_Dtm_Output1Select)0);
            IfxEgtm_Dtm_setOutput1Polarity((Ifx_EGTM_CLS_CDTM_DTM *)0, (IfxEgtm_Dtm_Ch)i, (IfxEgtm_Dtm_OutputPolarity)0);

            /* Optional explicit PinMap configuration via generic API (the unified driver also handles routing via OutputConfig) */
            IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
            IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
        }

        /* Initialize the PWM driver (unified high-level API) */
        IfxEgtm_Pwm_init(&g_egtmAtom3phInv.pwm, &g_egtmAtom3phInv.channels[0], &config);

        /* Store initial runtime values */
        g_egtmAtom3phInv.dutyCycles[0] = PHASE_U_DUTY_PERCENT;
        g_egtmAtom3phInv.dutyCycles[1] = PHASE_V_DUTY_PERCENT;
        g_egtmAtom3phInv.dutyCycles[2] = PHASE_W_DUTY_PERCENT;
        g_egtmAtom3phInv.phases[0]     = 0.0f;
        g_egtmAtom3phInv.phases[1]     = 0.0f;
        g_egtmAtom3phInv.phases[2]     = 0.0f;
        /* Dead-time record for runtime reference (ticks -> convert to us for record if needed) */
        g_egtmAtom3phInv.deadTimes[0].rising  = PWM_DEADTIME_US;
        g_egtmAtom3phInv.deadTimes[0].falling = PWM_DEADTIME_US;
        g_egtmAtom3phInv.deadTimes[1].rising  = PWM_DEADTIME_US;
        g_egtmAtom3phInv.deadTimes[1].falling = PWM_DEADTIME_US;
        g_egtmAtom3phInv.deadTimes[2].rising  = PWM_DEADTIME_US;
        g_egtmAtom3phInv.deadTimes[2].falling = PWM_DEADTIME_US;
    }

    /* 4/5) ADC trigger channel (ATOM0 C0 CH3) routing to ADC fabric and optionally to a pin */
    {
        /* Route eGTM trigger to TMADC using eGTM Trigger API */
        (void)IfxEgtm_Trigger_trigToAdc((IfxEgtm_Cluster)ADC_TRIG_CLUSTER,
                                        (IfxEgtm_TrigSource)ADC_TRIG_SOURCE_ATOM0,
                                        (IfxEgtm_TrigChannel)ADC_TRIG_CHANNEL_3,
                                        (IfxEgtm_Cfg_AdcTriggerSignal)ADC_TRIG_SIGNAL_TMADC0);

        /* Optional: drive trigger out on a board pin (P33.0 per requirements) via PinMap API */
        IfxEgtm_PinMap_setAtomTout((IfxEgtm_Atom_ToutMap *)0, IfxPort_OutputMode_pushPull, IfxPort_PadDriver_cmosAutomotiveSpeed1);
    }

    /* 6) Initialize and start TMADC0 module; channel details TBD (requirements say 5 channels) */
    {
        IfxAdc_Tmadc_Config   adcConfig;
        IfxAdc_Tmadc_ChConfig chCfg;
        IfxAdc_Tmadc_Ch       chObj; /* Placeholder single channel object; replicate per actual needs */

        IfxAdc_Tmadc_initModuleConfig(&adcConfig, &MODULE_ADC);
        IfxAdc_Tmadc_initModule(&g_tmadc, &adcConfig);

        /* Configure one representative channel now (IDs/pins TBD per HW). Replicate for 5 channels as needed. */
        IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);
        IfxAdc_Tmadc_initChannel(&chObj, &chCfg);

        /* Start TMADC module */
        IfxAdc_Tmadc_runModule(&g_tmadc);
    }

    /* 7) Configure LED GPIO as low-active push-pull output, default high (LED off) */
    IfxPort_setPinModeOutput(LED_PORT, LED_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(LED_PORT, LED_PIN, IfxPort_State_high);

    /* 8) Start synchronized PWM channels together */
    IfxEgtm_Pwm_startSyncedChannels(&g_egtmAtom3phInv.pwm);

    s_initialized = TRUE;
}

/* Runtime duty update: writes 3 requested duty values to the PWM driver immediately (percent units) */
void updateEgtmAtom3phInvDuty(float32 *requestDuty)
{
    if (s_initialized == FALSE)
    {
        return; /* Early-exit if not initialized (safety for tests and runtime) */
    }

    /* 1) Read three duty requests and update internal buffer */
    for (uint8 i = 0u; i < NUM_OF_CHANNELS; i++)
    {
        g_egtmAtom3phInv.dutyCycles[i] = requestDuty[i];
    }

    /* 2) Apply new duty values immediately; frequency/phase/deadtime unchanged */
    IfxEgtm_Pwm_updateChannelsDutyImmediate(&g_egtmAtom3phInv.pwm, g_egtmAtom3phInv.dutyCycles);
}
