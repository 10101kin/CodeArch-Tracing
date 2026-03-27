#ifndef IFXGTM_PINMAP_H
#define IFXGTM_PINMAP_H
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"

/* TOM TOUT mapping struct (minimal) */
typedef struct {
    uint8 dummy;
} IfxGtm_Tom_ToutMap;

typedef IFX_CONST IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* Function to mock */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

/* Pin symbol externs (required by tests) */
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_1_TOUT11_P00_2_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_2_TOUT12_P00_3_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_3_TOUT13_P00_4_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_4_TOUT14_P00_5_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_5_TOUT15_P00_6_OUT;
extern IfxGtm_Tom_ToutMap IfxGtm_TOM1_6_TOUT16_P00_7_OUT;

#endif /* IFXGTM_PINMAP_H */
