/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM PWM + ADC trigger setup (TC4xx, unified EGTM_Pwm driver)
 *
 * Note:
 *  - No watchdog control here (must be in CpuX_Main.c only)
 *  - No STM timing utilities here; scheduling belongs in application code
 */
#ifndef EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes EGTM ATOM unified PWM for 3-phase complementary drive plus 1 trigger channel */
void initEgtmAtom(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADCMULTIPLE_CHANNELS_H */
