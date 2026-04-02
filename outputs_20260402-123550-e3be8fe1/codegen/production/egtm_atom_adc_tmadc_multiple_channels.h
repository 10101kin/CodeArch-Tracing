/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 * Public API for EGTM ATOM 3-phase inverter PWM + TMADC trigger module.
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Public functions */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void updateEgtmAtomDuty(void);
void resultISR(void);
void IfxEgtm_periodEventFunction(void *data);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
