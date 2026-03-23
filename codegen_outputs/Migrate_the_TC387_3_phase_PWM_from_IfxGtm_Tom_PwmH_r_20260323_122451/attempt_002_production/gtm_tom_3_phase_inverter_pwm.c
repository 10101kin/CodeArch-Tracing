/*
 * GTM TOM 3-Phase Inverter PWM - Unified IfxGtm_Pwm driver (TC3xx)
 * See header for detailed description and requirements mapping.
 */
#include "gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Pwm.h"

/* Internal driver state structure (module-local) */
typedef struct
{
    IfxGtm_Pwm          pwm;                               /* PWM driver handle */
    IfxGtm_Pwm_Channel  channels[NUM_OF_CHANNELS];         /* Channel data after init */
    float32             dutyCycles[NUM_OF_CHANNELS];       /* Duty cycles in percent */
    float32             phases[NUM_OF_CHANNELS];           /* Phase offsets (not used) */
    IfxGtm_Pwm_DeadTime deadTimes[NUM_OF_CHANNELS];        /* Dead times (0 per req.) */
} GtmTom3phInv;

/* Static module instance */
IFX_STATIC GtmTom3phInv g_gtmTom3phInv;

/*
 * Initialize GTM TOM PWM as per SW Detailed Design behavior description
 */
void initGtmTomPwm(void)
{
    /* 1) Enable GTM and functional clocks (FXCLK) */
    IfxGtm_enable(&MODULE_GTM);

    /* Configure CMU clocks for robust operation; align GCLK and CLK0 to module freq */
    {
        float32 moduleFreq = IfxGtm_Cmu_getModuleFrequency(&MODULE_GTM);
        IfxGtm_Cmu_setGclkFrequency(&MODULE_GTM, moduleFreq);
        IfxGtm_Cmu_setClkFrequency(&MODULE_GTM, IfxGtm_Cmu_Clk_0, moduleFreq);
        /* Enable FXCLK (TOM uses Fxclk0 in this configuration) */
        IfxGtm_Cmu_enableClocks(&MODULE_GTM, IFXGTM_CMU_CLKEN_FXCLK);
    }

    /* 2) Route TOM TOUT pins (single-ended outputs) via PinMap API as required */
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_U_TOM_TOUT,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_V_TOM_TOUT,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_PadDriver_cmosAutomotiveSpeed1);
    IfxGtm_PinMap_setTomTout((IfxGtm_Tom_ToutMap *)PHASE_W_TOM_TOUT,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_PadDriver_cmosAutomotiveSpeed1);

    /* 3) Prepare unified PWM configuration */
    IfxGtm_Pwm_Config          config;
    IfxGtm_Pwm_ChannelConfig   chCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_OutputConfig    outCfg[NUM_OF_CHANNELS];
    IfxGtm_Pwm_DtmConfig       dtmCfg[NUM_OF_CHANNELS];

    IfxGtm_Pwm_initConfig(&config, &MODULE_GTM);

    /* Config fields per requirements */
    config.cluster             = IfxGtm_Cluster_0;                 /* TOM1 resides in cluster 0 on TC3xx */
    config.subModule           = IfxGtm_Pwm_SubModule_tom;         /* TOM submodule */
    config.alignment           = IfxGtm_Pwm_Alignment_center;      /* Center-aligned */
    config.syncStart           = FALSE;                            /* We'll start explicitly */
    config.syncUpdateEnabled   = TRUE;                             /* Synchronous shadow updates */
    config.numChannels         = NUM_OF_CHANNELS;
    config.channels            = chCfg;                            /* Array of channel configs */
    config.frequency           = TIMING_PWM_FREQUENCY_HZ;          /* 20 kHz */
    config.clockSource.tom     = IfxGtm_Cmu_Fxclk_0;               /* TOM uses Fxclk enum */
    config.dtmClockSource      = IfxGtm_Dtm_ClockSource_cmuClock0; /* DTM clock (deadtime=0) */

    /* 4) Configure each channel */
    /* Output/pad/polarity (single-ended) and DTM (0 ns) */
    outCfg[0].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_V_TOM_TOUT;  /* CH4 -> map first logical channel V */
    outCfg[0].complementaryPin      = NULL_PTR;                                 /* single-ended */
    outCfg[0].polarity              = Ifx_ActiveState_high;
    outCfg[0].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[0].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[0].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[1].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_U_TOM_TOUT;  /* CH2 -> logical channel U */
    outCfg[1].complementaryPin      = NULL_PTR;
    outCfg[1].polarity              = Ifx_ActiveState_high;
    outCfg[1].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[1].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[1].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    outCfg[2].pin                   = (IfxGtm_Pwm_ToutMap *)PHASE_W_TOM_TOUT;  /* CH6 -> logical channel W */
    outCfg[2].complementaryPin      = NULL_PTR;
    outCfg[2].polarity              = Ifx_ActiveState_high;
    outCfg[2].complementaryPolarity = Ifx_ActiveState_low;
    outCfg[2].outputMode            = IfxPort_OutputMode_pushPull;
    outCfg[2].padDriver             = IfxPort_PadDriver_cmosAutomotiveSpeed1;

    /* Dead-time: disabled per requirement (0 ns) */
    dtmCfg[0].deadTime = 0.0f;
    dtmCfg[1].deadTime = 0.0f;
    dtmCfg[2].deadTime = 0.0f;

    /* Initialize and populate channel configs */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        IfxGtm_Pwm_initChannelConfig(&chCfg[i]);
        chCfg[i].phase   = 0.0f;                 /* no phase shift */
        chCfg[i].dtm     = &dtmCfg[i];
        chCfg[i].output  = &outCfg[i];
        chCfg[i].interrupt = NULL_PTR;           /* no ISR in this module */
    }

    /* Timer channel selection: TOM1 channels 4,2,6 in logical order V,U,W */
    chCfg[0].timerCh = IfxGtm_Pwm_SubModule_Ch_4; chCfg[0].duty = DUTY_50_PERCENT; /* V */
    chCfg[1].timerCh = IfxGtm_Pwm_SubModule_Ch_2; chCfg[1].duty = DUTY_25_PERCENT; /* U */
    chCfg[2].timerCh = IfxGtm_Pwm_SubModule_Ch_6; chCfg[2].duty = DUTY_75_PERCENT; /* W */

    /* 5) Initialize PWM driver */
    IfxGtm_Pwm_init(&g_gtmTom3phInv.pwm, &g_gtmTom3phInv.channels[0], &config);

    /* 6) Store initial duties for runtime updates and apply once via shadow */
    g_gtmTom3phInv.dutyCycles[0] = chCfg[0].duty;
    g_gtmTom3phInv.dutyCycles[1] = chCfg[1].duty;
    g_gtmTom3phInv.dutyCycles[2] = chCfg[2].duty;

    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.dutyCycles);

    /* 7) Start synchronized channels (updates take effect on period-match) */
    IfxGtm_Pwm_startSyncedChannels(&g_gtmTom3phInv.pwm);
}

void updateGtmTomPwmDutyCycles(void)
{
    /* Increment by fixed step and wrap at 100% to 0% */
    for (uint8 i = 0; i < NUM_OF_CHANNELS; i++)
    {
        g_gtmTom3phInv.dutyCycles[i] += TIMING_DUTY_STEP_PERCENT;
        if (g_gtmTom3phInv.dutyCycles[i] >= DUTY_MAX)
        {
            g_gtmTom3phInv.dutyCycles[i] = DUTY_MIN;
        }
    }

    /* Apply via multi-channel duty update (shadowed; sync @ period) */
    IfxGtm_Pwm_updateChannelsDuty(&g_gtmTom3phInv.pwm, g_gtmTom3phInv.dutyCycles);
}
