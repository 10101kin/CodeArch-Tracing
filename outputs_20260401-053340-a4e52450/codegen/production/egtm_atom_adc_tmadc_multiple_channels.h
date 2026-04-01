/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM 3-phase inverter PWM + TMADC trigger driver
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Ifx_Types.h"

/* Public APIs */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void interruptEgtmAtom(void);
void IfxEgtm_periodEventFunction(void *data);
void resultISR(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
