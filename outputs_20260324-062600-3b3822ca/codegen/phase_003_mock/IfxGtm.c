#include "IfxGtm.h"

/* Counter */
static uint32 s_enable_count = 0;

/* Replaced by stateful mock below */





/* Stateful GTM enable mock (auto-injected for TDD) */
static uint32 s_gtm_enable_count = 0;
static boolean s_gtm_enabled = FALSE;

void IfxGtm_enable(Ifx_GTM *gtm) {
    (void)gtm;
    s_gtm_enable_count++;
    s_gtm_enabled = TRUE;
}

boolean IfxGtm_isEnabled(Ifx_GTM *gtm) {
    (void)gtm;
    return s_gtm_enabled;
}

uint32 IfxGtm_Mock_GetCallCount_enable(void) { return s_gtm_enable_count; }

void IfxGtm_Mock_Reset(void) {
    s_gtm_enable_count = 0;
    s_gtm_enabled = FALSE;
}
