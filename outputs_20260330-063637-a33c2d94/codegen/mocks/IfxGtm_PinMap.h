#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* TOM Tout mapping types */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
    uint8  tom;
    uint8  channel;
} IfxGtm_Tom_ToutMap;

typedef const IfxGtm_Tom_ToutMap IfxGtm_Tom_ToutMapP;

/* Generic PWM Tout map for external pin symbols */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
} IfxGtm_Pwm_ToutMap;

/* Function from DRIVERS TO MOCK */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

#endif /* IFXGTM_PINMAP_H */
