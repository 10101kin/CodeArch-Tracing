/*====================================================================================================================
 *  File: egtm_atom_adc_tmadc_multiple_channels.h
 *  Brief: Public API for EGTM ATOM PWM (3-phase inverter + ADC trigger)
 *====================================================================================================================*/
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(float32 *requestDuty);
void initEgtmAtomAdcTrigger(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
