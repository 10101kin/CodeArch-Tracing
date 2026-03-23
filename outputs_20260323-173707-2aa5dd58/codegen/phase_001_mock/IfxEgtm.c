#include "IfxEgtm.h"

static uint32 s_enable_count = 0;

/* Replaced by stateful mock below */




/* Stateful EGTM enable mock (auto-injected for TDD) */
static uint32 s_egtm_enable_count = 0;
static boolean s_egtm_enabled = FALSE;

void IfxEgtm_enable(Ifx_EGTM *egtm) {
    (void)egtm;
    s_egtm_enable_count++;
    s_egtm_enabled = TRUE;
}

boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm) {
    (void)egtm;
    return s_egtm_enabled;
}

uint32 IfxEgtm_Mock_GetCallCount_enable(void) { return s_egtm_enable_count; }

void IfxEgtm_Mock_Reset(void) {
    s_egtm_enable_count = 0;
    s_egtm_enabled = FALSE;
}
