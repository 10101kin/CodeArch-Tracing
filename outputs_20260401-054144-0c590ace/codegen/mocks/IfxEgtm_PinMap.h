#ifndef IFXEGTM_PINMAP_H
#define IFXEGTM_PINMAP_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

typedef enum {
    IfxEgtm_ChXSel_a = 0,
    IfxEgtm_ChXSel_b,
    IfxEgtm_ChXSel_c,
    IfxEgtm_ChXSel_d,
    IfxEgtm_ChXSel_e,
    IfxEgtm_ChXSel_f,
    IfxEgtm_ChXSel_g,
    IfxEgtm_ChXSel_h,
    IfxEgtm_ChXSel_i,
    IfxEgtm_ChXSel_j,
    IfxEgtm_ChXSel_k,
    IfxEgtm_ChXSel_l,
    IfxEgtm_ChXSel_m,
    IfxEgtm_ChXSel_n
} IfxEgtm_ChXSel;

typedef enum {
    IfxEgtm_ToutSel_0 = 0,  IfxEgtm_ToutSel_1,  IfxEgtm_ToutSel_2,  IfxEgtm_ToutSel_3,
    IfxEgtm_ToutSel_4,      IfxEgtm_ToutSel_5,  IfxEgtm_ToutSel_6,  IfxEgtm_ToutSel_7,
    IfxEgtm_ToutSel_8,      IfxEgtm_ToutSel_9,  IfxEgtm_ToutSel_10, IfxEgtm_ToutSel_11,
    IfxEgtm_ToutSel_12,     IfxEgtm_ToutSel_13, IfxEgtm_ToutSel_14, IfxEgtm_ToutSel_15,
    IfxEgtm_ToutSel_16,     IfxEgtm_ToutSel_17, IfxEgtm_ToutSel_18, IfxEgtm_ToutSel_19,
    IfxEgtm_ToutSel_20,     IfxEgtm_ToutSel_21, IfxEgtm_ToutSel_22, IfxEgtm_ToutSel_23,
    IfxEgtm_ToutSel_24,     IfxEgtm_ToutSel_25, IfxEgtm_ToutSel_26, IfxEgtm_ToutSel_27,
    IfxEgtm_ToutSel_28,     IfxEgtm_ToutSel_29
} IfxEgtm_ToutSel;

/* PinMap API */
void IfxEgtm_PinMap_setAtomTout(IfxEgtm_Atom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver);

#endif
