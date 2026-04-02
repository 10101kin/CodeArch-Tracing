/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM + TMADC multi-channel PWM/ADC driver
 */

#ifndef EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void updateEgtmAtomDuty(void);
void resultISR(void);
void IfxEgtm_periodEventFunction(void *data);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H */
