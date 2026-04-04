/*
 * egtmatompwm.h
 * Public API for EGTM ATOM 3-phase inverter PWM (TC4xx)
 */
#ifndef EGTMATOMPWM_H
#define EGTMATOMPWM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtomDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTMATOMPWM_H */
