/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM PWM + TMADC trigger driver (TC4xx).
 *
 * Note: Including files should include Ifx_Types.h before this header
 *       to make float32 available to the compiler.
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void initEgtmAtomAdcTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
