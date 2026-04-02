/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM 3-phase complementary PWM with TMADC trigger.
 *
 * Note: This header intentionally contains only function prototypes as per
 *       project structural rules. Types like float32 come from Ifx_Types.h
 *       which must be included by the compilation unit before including this
 *       header.
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void updateEgtmAtomDuty(void);
void resultISR(void);

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
