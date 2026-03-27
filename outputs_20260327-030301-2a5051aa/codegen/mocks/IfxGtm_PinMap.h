#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* Minimal TOM TOUT map type used by Timer/PwmHl */
typedef struct
{
    uint8  tom;      /* TOM index */
    uint8  channel;  /* TOM channel */
    uint8  tout;     /* TOUT number */
    Ifx_P *port;     /* Port module pointer */
    uint8  pin;      /* Port pin index */
} IfxGtm_Tom_ToutMap;
typedef IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* Function required by drivers to mock */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

/* Extern pin symbols referenced in production/tests */
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_1_TOUT11_P00_2_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_4_TOUT14_P00_5_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_5_TOUT15_P00_6_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_0_TOUT10_P00_1_OUT;

#endif /* IFXGTM_PINMAP_H */
