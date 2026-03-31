/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM PWM + TMADC trigger driver (TC4xx).
 *
 * NOTE: This header exposes only the user-facing functions. Implementation
 * resides in egtm_atom_adc_tmadc_multiple_channels.c
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#include "Ifx_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize 3-phase complementary center-aligned PWM (EGTM ATOM CH0..2). */
void initEgtmAtom3phInv(void);

/** Update 3-phase inverter duty cycles immediately (percent values). */
void updateEgtmAtom3phInvDuty(float32 *requestDuty);

/** Initialize a single ATOM channel (CH3) as 50% edge-aligned TMADC trigger. */
void initEgtmAtomAdcTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
