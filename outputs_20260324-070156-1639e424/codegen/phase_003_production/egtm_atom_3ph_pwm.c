#include "egtm_atom_3ph_pwm.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* =========================================================================
 * Internal driver context
 * ========================================================================= */
typedef struct
{
    /* TMADC driver handles */
    IfxAdc_Tmadc          tmadc;                                  /* TMADC module handle */
    IfxAdc_Tmadc_Ch       tmadcChannels[TMADC0_CHANNELS_PER_PERIOD]; /* Channel handles */

    /* LED control */
    Ifx_P                *ledPort;
    uint8                 ledPin;
    IfxPort_State         ledActiveState;                         /* IfxPort_State_low for active-low LED */

    boolean               initialized;
} EgtmAtom3phPwm_Driver;

static EgtmAtom3phPwm_Driver s_drv = {
    .tmadc = {0},
    .tmadcChannels = {0},
    .ledPort = LED_PIN_PORT,
    .ledPin = LED_PIN_INDEX,
#if LED_ACTIVE_LEVEL_LOW
    .ledActiveState = IfxPort_State_low,
#else
    .ledActiveState = IfxPort_State_high,
#endif
    .initialized = FALSE
};

/* =========================================================================
 * Public API implementation
 * ========================================================================= */
void initEgtmAtom3phInv(void)
{
    /* ---------------------------------------------------------------------
     * 2) Enable ADC module and configure TMADC in queued/polled mode
     * --------------------------------------------------------------------- */
    /* Enable top-level ADC module */
    IfxAdc_enableModule(&MODULE_ADC);

    /* Initialize TMADC module configuration with defaults */
    IfxAdc_Tmadc_Config tmadcCfg;
    IfxAdc_Tmadc_initModuleConfig(&tmadcCfg, &MODULE_ADC);

    /* Configure TMADC for queued/polled operation as per requirements.
       Note: Field-level assignments are intentionally omitted here to avoid
       dependence on undocumented struct members. The init functions will
       apply default safe configuration; triggering source routing is expected
       to be performed by system integration (eGTM trigger wiring) outside
       this module per SW design. */

    /* Initialize TMADC module */
    IfxAdc_Tmadc_initModule(&s_drv.tmadc, &tmadcCfg);

    /* Prepare five channel configurations for polled sequence operation */
    {
        uint8 i;
        for (i = 0u; i < TMADC0_CHANNELS_PER_PERIOD; i++)
        {
            IfxAdc_Tmadc_ChConfig chCfg;
            IfxAdc_Tmadc_initChannelConfig(&chCfg, &MODULE_ADC);

            /* The detailed channel IDs/pins and queue order are TBD as per requirements.
               Application shall finalize mapping to ADC0.CH0–CH4 and pinmux externally. */

            IfxAdc_Tmadc_initChannel(&s_drv.tmadcChannels[i], &chCfg);
        }
    }

    /* Start TMADC in polled mode (no ISR) */
    IfxAdc_Tmadc_runModule(&s_drv.tmadc);

    /* ---------------------------------------------------------------------
     * 3) Configure LED GPIO as push-pull output and set LED to inactive
     * --------------------------------------------------------------------- */
    IfxPort_setPinModeOutput(s_drv.ledPort, s_drv.ledPin, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);

    /* LED is active-low; set HIGH to ensure LED is off initially */
#if LED_ACTIVE_LEVEL_LOW
    IfxPort_setPinState(s_drv.ledPort, s_drv.ledPin, IfxPort_State_high);
#else
    IfxPort_setPinState(s_drv.ledPort, s_drv.ledPin, IfxPort_State_low);
#endif

    /* Mark initialized; all called APIs are void and do not report failure. */
    s_drv.initialized = TRUE;

    /* ---------------------------------------------------------------------
     * 1) Optional system timebase using supported TC4Dx resources is out of
     *    scope for this module per SW Detailed Design (no eGTM usage here).
     * 4) PWM generation for a 3-phase inverter is not configured here.
     * --------------------------------------------------------------------- */
}
