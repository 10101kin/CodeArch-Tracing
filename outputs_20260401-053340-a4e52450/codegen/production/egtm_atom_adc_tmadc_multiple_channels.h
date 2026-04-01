/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 * Public API for EGTM ATOM + TMADC multi-channel driver
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Public APIs (user-facing only) */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
