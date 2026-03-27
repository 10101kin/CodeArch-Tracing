/* IfxGtm_PinMap.h - mock */
#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* Minimal Tom Tout mapping type used in Timer Config and pin setup */
typedef struct
{
    void  *tom;       /* TOM SFR base (opaque) */
    uint8  tomIndex;  /* TOM index */
    uint8  channel;   /* Channel index */
    void  *port;      /* Port SFR base (opaque) */
    uint8  pinIndex;  /* Pin index */
    uint8  outIdx;    /* Output index/alt function */
} IfxGtm_Tom_ToutMap;

typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

#endif /* IFXGTM_PINMAP_H */
