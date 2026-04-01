/**********************************************************************************************************************
 *  FILE: egtm_atom_tmadc_consolidated.h
 *  BRIEF: Consolidated eGTM ATOM + TMADC driver (TC4xx) — public API
 *  NOTE : Header contains only include guard and public prototypes as per module rules.
 *********************************************************************************************************************/
#ifndef EGTM_ATOM_TMADC_CONSOLIDATED_H
#define EGTM_ATOM_TMADC_CONSOLIDATED_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize consolidated eGTM ATOM PWM (3-phase complementary, center-aligned with DTM) and TMADC trigger path.
 * - Configures ATOM0 CH0–CH2 (complementary pairs) at 20 kHz with 1 µs dead-time and initial duties 25/50/75 %
 * - Sets up ATOM trigger channel (CH3) and routes trigger to TMADC via IfxEgtm_Trigger APIs
 * - Initializes TMADC module
 */
void initEgtmAtom3phInv(void);

#ifdef __cplusplus
}
#endif

#endif /* EGTM_ATOM_TMADC_CONSOLIDATED_H */
