/*
 * egtmatompwm.h
 *
 * Public API for EGTM ATOM PWM 3-phase inverter driver (TC4xx).
 *
 * Note: Do not declare internal callback functions here.
 */
#ifndef EGTMATOMPWM_H
#define EGTMATOMPWM_H

#ifdef __cplusplus
extern "C" {
#endif

void initEgtmAtom3phInv(void);
void updateEgtmAtomDuty(void);
void resultISR(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTMATOMPWM_H */
