/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 * Public API for EGTM ATOM + TMADC multi-channel PWM/trigger driver
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize unified EGTM PWM instance (3 complementary inverter channels + 1 trigger channel) */
void initEgtmAtom(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
