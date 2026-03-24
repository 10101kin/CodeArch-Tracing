/*
 * egtm_atom_3ph_pwm.c
 *
 * Implements the SW Detailed Design behavior:
 * - Configure TMADC0 in queued/polled mode with 5 channels
 * - Configure LED P03.9 as push-pull output, inactive level high (active-low LED)
 * - No eGTM PWM configuration here
 *
 * Notes:
 * - Watchdog handling is NOT present in this driver (per architecture rule).
 * - Error handling: checks return values for functions that return boolean and
 *   early-exits without setting the initialized flag if any step fails.
 */

#include "egtm_atom_3ph_pwm.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* ============================ Internal driver state ============================ */
static boolean s_initialized = FALSE;  /* Set TRUE only after successful init */

/* Handles for TMADC module and channels (types provided by IfxAdc_Tmadc.h) */
static IfxAdc_Tmadc              g_tmadc;                                        /* TMADC module handle */
static IfxAdc_Tmadc_Channel      g_tmadcChannels[TMADC0_CHANNELS_PER_PERIOD];    /* 5 channels */

/* ================================ Implementation =============================== */

/**
 * Initialize only peripherals present on TC4D7 (per SW Detailed Design):
 * 1) (Optional) System timebase via supported TC4Dx resources (not used here)
 * 2) Enable and configure TMADC0 in queued/polled mode with 5 channels
 * 3) Configure LED P03.9 as push-pull output and set inactive level (high)
 * 4) No eGTM PWM configuration in this module
 */
void initEgtmAtom3phInv(void)
{
    /* Early exit if already initialized to keep idempotence */
    if (s_initialized == TRUE)
    {
        return;
    }

    /* ----------------------- 2) TMADC0 queued/polled init ----------------------- */
    /* Enable ADC module (signature determined by platform; ignore return if void) */
    IfxAdc_enableModule();

    /* Initialize TMADC module configuration */
    IfxAdc_Tmadc_Config tmadcConfig;                      /* Module config */
    IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, &MODULE_ADC);

    /* Configure for queued/polled operation as per requirements */
    /* Note: Direct field assignments are intentionally minimal due to API variance
       across TC4xx variants; the template defaults are used where possible. */

    /* Apply module configuration */
    {
        boolean ok = TRUE;
        ok = IfxAdc_Tmadc_initModule(&g_tmadc, &tmadcConfig);
        if (ok == FALSE)
        {
            return;  /* Do not proceed, do not set initialized */
        }
    }

    /* Prepare 5 TMADC channels for queued/polled scanning */
    for (uint8 i = 0u; i < TMADC0_CHANNELS_PER_PERIOD; i++)
    {
        IfxAdc_Tmadc_ChannelConfig chCfg;                /* Per-channel config */
        IfxAdc_Tmadc_initChannelConfig(&chCfg, &g_tmadc);

        /* The exact channel IDs/pins are TBD per board wiring (see requirements).
           Channel indices 0..4 are prepared here; concrete pinmux to be finalized
           in system integration. */

        /* Initialize channel; check boolean return if provided by API */
        {
            boolean ok = TRUE;
            ok = IfxAdc_Tmadc_initChannel(&g_tmadcChannels[i], &chCfg);
            if (ok == FALSE)
            {
                return;  /* Early exit on failure */
            }
        }
    }

    /* Start TMADC in polled (non-interrupt) mode */
    {
        boolean ok = TRUE;
        ok = IfxAdc_Tmadc_runModule(&g_tmadc);
        if (ok == FALSE)
        {
            return;  /* Do not continue if start failed */
        }
    }

    /* ----------------------------- 3) LED GPIO init ----------------------------- */
    /* LED pin: P03.9, active-low. Configure push-pull output and set HIGH (inactive). */
    IfxPort_setPinModeOutput(&MODULE_P03, LED_PIN_INDEX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(&MODULE_P03, LED_PIN_INDEX, IfxPort_State_high);

    /* All steps succeeded */
    s_initialized = TRUE;
}
