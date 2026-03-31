/*
 * egtm_atom_3_phase_inverter_pwm.h
 *
 * Public API for the EGTM ATOM 3-Phase Inverter PWM driver.
 *
 * Note:
 *  - Watchdog control must NOT be added here. Follow AURIX standard: only in CpuX_Main.c.
 */
#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Public API */
void initEgtmAtom3phInv(void);
void updateEgtmAtom3phInvDuty(void);

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
