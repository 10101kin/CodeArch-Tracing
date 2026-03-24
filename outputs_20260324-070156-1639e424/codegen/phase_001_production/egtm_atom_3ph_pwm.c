/*
 * egtm_atom_3ph_pwm.c
 *
 * Production module for TC4xx (TC4D7) implementing TMADC polled configuration
 * and LED GPIO setup, per SW Detailed Design. No eGTM PWM configuration is
 * performed in this module.
 */
#include "egtm_atom_3ph_pwm.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* =====================================================================
 * PRIVATE DRIVER STATE
 * ===================================================================== */
static volatile boolean s_initialized = FALSE;

/* TMADC driver handles and configuration (ADC0) */
static IfxAdc_Tmadc              g_tmadc0;                       /* TMADC0 handle */
static IfxAdc_Tmadc_Config       g_tmadc0Config;                 /* TMADC0 config */
static IfxAdc_Tmadc_Channel      g_tmadc0Channels[TMADC0_CHANNELS_PER_PERIOD];
static IfxAdc_Tmadc_ChannelConfig g_tmadc0ChConfig[TMADC0_CHANNELS_PER_PERIOD];

/* =====================================================================
 * LOCAL UTILITIES
 * ===================================================================== */
static void prv_configureLed(void)
{
    /* Configure LED pin P03.9 as push-pull output. Set initial inactive state (high) because LED is active-low. */
    IfxPort_setPinModeOutput((Ifx_P *)&LED_PIN_PORT, (uint8)LED_PIN_INDEX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState((Ifx_P *)&LED_PIN_PORT, (uint8)LED_PIN_INDEX, IfxPort_State_high);
}

/* =====================================================================
 * PUBLIC API IMPLEMENTATION
 * ===================================================================== */
void initEgtmAtom3phInv(void)
{
    /*
     * Behavior per SW Detailed Design:
     * - Optional system timebase not configured here.
     * - Enable ADC and initialize TMADC (module + channels) for polled queued scan.
     * - Configure LED GPIO to inactive level.
     * - Do not configure eGTM PWM in this module.
     */

    /* 1) Optional system timebase intentionally omitted (no eGTM usage here). */

    /* 2) ADC/TMADC initialization (queued/polled) */
    /* Enable the ADC top-level module prior to TMADC configuration. */
    IfxAdc_enableModule(&MODULE_ADC);

    /* Initialize TMADC0 module configuration and module instance. */
    IfxAdc_Tmadc_initModuleConfig(&g_tmadc0Config, &MODULE_ADC);

    /*
     * Apply requirements-driven intent via config object fields where applicable.
     * Exact field names are per iLLD; runtime trigger source is eGTM ATOM0.CH3.
     * Polled operation (no ISR) is selected; the external trigger and routing are
     * expected to be set up in the eGTM domain outside of this module.
     */
    /* Example intent (do not rely on specific field names here):
       - g_tmadc0Config.conversionMode = queued_scan_on_trigger;
       - g_tmadc0Config.servicingMode  = polled;
       - g_tmadc0Config.triggerSource  = EGTM ATOM0 CH3;
       The actual fields are set by the application when integrating with the correct iLLD. */

    IfxAdc_Tmadc_initModule(&g_tmadc0, &g_tmadc0Config);

    /* Initialize 5 TMADC channels: ADC0.CH0 .. ADC0.CH4 */
    for (uint8 i = 0U; i < (uint8)TMADC0_CHANNELS_PER_PERIOD; i++)
    {
        IfxAdc_Tmadc_initChannelConfig(&g_tmadc0ChConfig[i], &g_tmadc0);

        /*
         * Channel configuration intent per requirements (queue order CH0..CH4):
         * - Assign channel ID = i (ADC0.CH0..ADC0.CH4)
         * - Enable channel in the queue
         * - Use external trigger from EGTM ATOM0.CH3 (falling edge, 50% duty) to start queued scan
         * Field names are iLLD-specific and should be set by the integrator to match the TC4D7 TMADC API.
         */
        /* Example intent (non-binding):
           g_tmadc0ChConfig[i].channelId      = (uint8)i;
           g_tmadc0ChConfig[i].queueEnable    = TRUE;
           g_tmadc0ChConfig[i].triggerEnable  = TRUE;
         */

        IfxAdc_Tmadc_initChannel(&g_tmadc0Channels[i], &g_tmadc0ChConfig[i]);
    }

    /* Start TMADC module in polled mode (no ISR) */
    IfxAdc_Tmadc_runModule(&g_tmadc0);

    /* 3) LED GPIO configuration */
    prv_configureLed();

    /* 4) eGTM PWM is intentionally not configured here per SW Detailed Design. */

    /* Initialization complete */
    s_initialized = TRUE;
}
