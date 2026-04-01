#ifndef EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define EGTM_ATOM_3_PHASE_INVERTER_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize a 3-channel center-aligned complementary PWM on EGTM ATOM0, Cluster 0.
 */
void initEgtmAtom3phInv(void);

/**
 * Update duty cycles of all three PWM channels by a fixed step with wrap at 100%.
 */
void updateEgtmAtom3phInvDuty(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
