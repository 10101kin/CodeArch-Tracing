#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm_Tom_PwmHl.h"

/* API from driver list */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

#endif /* IFXGTM_PINMAP_H */
