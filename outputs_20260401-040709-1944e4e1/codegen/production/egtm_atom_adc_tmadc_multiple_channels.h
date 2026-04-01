/*
 * egtm_atom_adc_tmadc_multiple_channels.h
 *
 * Public API for EGTM ATOM PWM + TMADC trigger migration (TC4xx).
 *
 * Note:
 *  - Do not include headers, typedefs, or macros here per project rules.
 *  - Watchdog control must remain in CpuX_Main.c files.
 */
#ifndef EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initializes EGTM ATOM PWM with three complementary inverter channels and one 50% trigger channel */
void initEgtmAtom(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
