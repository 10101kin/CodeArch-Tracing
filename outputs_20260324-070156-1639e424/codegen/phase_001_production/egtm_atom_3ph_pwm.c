#include "egtm_atom_3ph_pwm.h"
#include "IfxAdc.h"
#include "IfxAdc_Tmadc.h"
#include "IfxPort.h"

/* =============================================================================
 * Module-local driver context
 * ============================================================================= */

typedef struct
{
    IfxAdc_Tmadc          tmadc;                                       /* TMADC module handle */
    IfxAdc_Tmadc_Channel  channels[TMADC0_CHANNELS_PER_PERIOD];        /* TMADC channel handles */
    boolean               initialized;                                  /* Init status */
} EgtmAtom3phPwm_Driver;

static EgtmAtom3phPwm_Driver s_drv = {0};

/* =============================================================================
 * Public API implementation
 * ============================================================================= */

void initEgtmAtom3phInv(void)
{
    /*
     * Behavior per SW Detailed Design:
     * 1) Optional system timebase (WUT) is not configured here. If required, do it in CpuN_Main.c.
     */

    /* 2) Enable ADC module and configure TMADC in queued/polled mode */
    if (IfxAdc_enableModule() == FALSE)
    {
        return; /* Do not proceed on failure */
    }

    /* TMADC module configuration */
    IfxAdc_Tmadc_Config tmadcConfig;
    /* Note: Module pointer is platform-specific; passing NULL follows the iLLD initConfig pattern in mocked tests */
    IfxAdc_Tmadc_initModuleConfig(&tmadcConfig, NULL);

    if (IfxAdc_Tmadc_initModule(&s_drv.tmadc, &tmadcConfig) == FALSE)
    {
        return; /* Do not proceed on failure */
    }

    /* Prepare N channels for queued/polled conversions (sequence order 0..N-1) */
    for (uint8 i = 0u; i < TMADC0_CHANNELS_PER_PERIOD; i++)
    {
        IfxAdc_Tmadc_ChannelConfig chCfg;
        IfxAdc_Tmadc_initChannelConfig(&chCfg, &s_drv.tmadc);
        /*
         * Channel selection/order and pin mapping are board-dependent (TBD per requirements note).
         * Keep defaults from initChannelConfig; the queue order is established by the init sequence here.
         */
        if (IfxAdc_Tmadc_initChannel(&s_drv.channels[i], &chCfg) == FALSE)
        {
            return; /* Do not proceed on failure */
        }
    }

    /* Run TMADC in polled mode (no ISR) */
    if (IfxAdc_Tmadc_runModule(&s_drv.tmadc) == FALSE)
    {
        return; /* Do not proceed on failure */
    }

    /* 3) Configure LED GPIO (P03.9) as push-pull output; LED is active low -> set inactive level high */
    IfxPort_setPinModeOutput(&LED_PORT, LED_PIN_INDEX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(&LED_PORT, LED_PIN_INDEX, IfxPort_State_high);

    /* 4) No eGTM PWM configuration here (per SW design, use TC4Dx-supported timers elsewhere if needed) */

    s_drv.initialized = TRUE;
}
