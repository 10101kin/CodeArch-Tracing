/*
 * egtm_atom_tmadc_consolidated.h
 *
 * Public API for consolidated eGTM ATOM + TMADC driver (TC4xx).
 */
#ifndef EGTM_ATOM_TMADDC_CONSOLIDATED_H
#define EGTM_ATOM_TMADDC_CONSOLIDATED_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize consolidated eGTM ATOM 3-phase inverter PWM + ATOM trigger + TMADC */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_TMADDC_CONSOLIDATED_H */
