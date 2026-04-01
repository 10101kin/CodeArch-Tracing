/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM PWM (3-phase complementary + ADC trigger channel)
 */
#ifndef EGTM_ATOM_ADC_TMACD_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMACD_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize EGTM ATOM for 3-phase complementary PWM (U,V,W) plus a single 50% trigger channel */
void initEgtmAtom(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMACD_MULTIPLE_CHANNELS_H */
