#include "IfxStm.h"

/* MODULE_STM0 is defined in ifx_types.c — only use extern from IfxStm.h */

/* 2026-02-20 Yvan: Added IfxStm_get and IfxStm_getFrequency mocks */
uint64 IfxStm_get(Ifx_STM *stm) {
    (void)stm;  /* Unused in mock */
    return 0ULL;  /* Return 0 for mock timer value */
}

float32 IfxStm_getFrequency(Ifx_STM *stm) {
    (void)stm;  /* Unused in mock */
    return 100000000.0f;  /* Return 100MHz as mock frequency */
}

sint32 IfxStm_getTicksFromMilliseconds(Ifx_STM *stm, uint32 millis) {
    (void)stm;  /* Unused in mock */
    return (sint32)(millis * 1000);  /* Simple conversion for testing */
}

sint32 IfxStm_getTicksFromMicroseconds(Ifx_STM *stm, uint32 micros) {
    (void)stm;  /* Unused in mock */
    return (sint32)micros;
}
