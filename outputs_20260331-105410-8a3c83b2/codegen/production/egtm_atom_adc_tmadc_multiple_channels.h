/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 * Public API for EGTM ATOM PWM + TMADC trigger driver (TC4xx)
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void initEgtmAtomAdcTrigger(void);

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
