#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* Enums */
typedef enum {
    IfxEgtm_ChXSel_a = 0,
    IfxEgtm_ChXSel_b = 1,
    IfxEgtm_ChXSel_c = 2,
    IfxEgtm_ChXSel_d = 3,
    IfxEgtm_ChXSel_e = 4,
    IfxEgtm_ChXSel_f = 5,
    IfxEgtm_ChXSel_g = 6,
    IfxEgtm_ChXSel_h = 7,
    IfxEgtm_ChXSel_i = 8,
    IfxEgtm_ChXSel_j = 9,
    IfxEgtm_ChXSel_k = 10,
    IfxEgtm_ChXSel_l = 11,
    IfxEgtm_ChXSel_m = 12,
    IfxEgtm_ChXSel_n = 13
} IfxEgtm_ChXSel;

typedef enum {
    IfxEgtm_ToutSel_0  = 0,
    IfxEgtm_ToutSel_1  = 1,
    IfxEgtm_ToutSel_2  = 2,
    IfxEgtm_ToutSel_3  = 3,
    IfxEgtm_ToutSel_4  = 4,
    IfxEgtm_ToutSel_5  = 5,
    IfxEgtm_ToutSel_6  = 6,
    IfxEgtm_ToutSel_7  = 7,
    IfxEgtm_ToutSel_8  = 8,
    IfxEgtm_ToutSel_9  = 9,
    IfxEgtm_ToutSel_10 = 10,
    IfxEgtm_ToutSel_11 = 11,
    IfxEgtm_ToutSel_12 = 12,
    IfxEgtm_ToutSel_13 = 13,
    IfxEgtm_ToutSel_14 = 14,
    IfxEgtm_ToutSel_15 = 15,
    IfxEgtm_ToutSel_16 = 16,
    IfxEgtm_ToutSel_17 = 17,
    IfxEgtm_ToutSel_18 = 18,
    IfxEgtm_ToutSel_19 = 19,
    IfxEgtm_ToutSel_20 = 20,
    IfxEgtm_ToutSel_21 = 21,
    IfxEgtm_ToutSel_22 = 22,
    IfxEgtm_ToutSel_23 = 23,
    IfxEgtm_ToutSel_24 = 24,
    IfxEgtm_ToutSel_25 = 25,
    IfxEgtm_ToutSel_26 = 26,
    IfxEgtm_ToutSel_27 = 27,
    IfxEgtm_ToutSel_28 = 28,
    IfxEgtm_ToutSel_29 = 29
} IfxEgtm_ToutSel;

/* Functions to mock */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

#endif /* IFXEGTM_PINMAP_H */
