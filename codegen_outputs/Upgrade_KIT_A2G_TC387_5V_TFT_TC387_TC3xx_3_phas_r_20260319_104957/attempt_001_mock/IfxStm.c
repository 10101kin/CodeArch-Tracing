#include "IfxStm.h"

static uint32  s_getFrequency_count = 0;
static float32 s_getFrequency_ret = 0.0f;

static uint32  s_getTicksFromUs_count = 0;
static uint32  s_getTicksFromUs_last = 0u;
static sint32  s_getTicksFromUs_ret = 0;

static uint32  s_getTicksFromMs_count = 0;
static uint32  s_getTicksFromMs_last = 0u;
static sint32  s_getTicksFromMs_ret = 0;

static uint32  s_disableCmpInt_count = 0;
static uint32  s_disableCmpInt_lastComparator = 0u;

static uint32  s_enableCmpInt_count = 0;
static uint32  s_enableCmpInt_lastComparator = 0u;

float32 IfxStm_getFrequency(Ifx_STM *stm) {
    (void)stm;
    s_getFrequency_count++;
    return s_getFrequency_ret;
}

sint32 IfxStm_getTicksFromMicroseconds(Ifx_STM *stm, uint32 microSeconds) {
    (void)stm;
    s_getTicksFromUs_count++;
    s_getTicksFromUs_last = microSeconds;
    return s_getTicksFromUs_ret;
}

sint32 IfxStm_getTicksFromMilliseconds(Ifx_STM *stm, uint32 milliSeconds) {
    (void)stm;
    s_getTicksFromMs_count++;
    s_getTicksFromMs_last = milliSeconds;
    return s_getTicksFromMs_ret;
}

void IfxStm_disableComparatorInterrupt(Ifx_STM *stm, IfxStm_Comparator comparator) {
    (void)stm;
    s_disableCmpInt_count++;
    s_disableCmpInt_lastComparator = (uint32)comparator;
}

void IfxStm_enableComparatorInterrupt(Ifx_STM *stm, IfxStm_Comparator comparator) {
    (void)stm;
    s_enableCmpInt_count++;
    s_enableCmpInt_lastComparator = (uint32)comparator;
}

/* Mock controls */
void   IfxStm_Mock_SetReturn_getFrequency(float32 ret) { s_getFrequency_ret = ret; }
uint32 IfxStm_Mock_GetCallCount_getFrequency(void) { return s_getFrequency_count; }

void   IfxStm_Mock_SetReturn_getTicksFromMicroseconds(sint32 ret) { s_getTicksFromUs_ret = ret; }
uint32 IfxStm_Mock_GetCallCount_getTicksFromMicroseconds(void) { return s_getTicksFromUs_count; }
uint32 IfxStm_Mock_GetLastArg_getTicksFromMicroseconds_us(void) { return s_getTicksFromUs_last; }

void   IfxStm_Mock_SetReturn_getTicksFromMilliseconds(sint32 ret) { s_getTicksFromMs_ret = ret; }
uint32 IfxStm_Mock_GetCallCount_getTicksFromMilliseconds(void) { return s_getTicksFromMs_count; }
uint32 IfxStm_Mock_GetLastArg_getTicksFromMilliseconds_ms(void) { return s_getTicksFromMs_last; }

uint32 IfxStm_Mock_GetCallCount_disableComparatorInterrupt(void) { return s_disableCmpInt_count; }
uint32 IfxStm_Mock_GetLastArg_disableComparatorInterrupt_comparator(void) { return s_disableCmpInt_lastComparator; }

uint32 IfxStm_Mock_GetCallCount_enableComparatorInterrupt(void) { return s_enableCmpInt_count; }
uint32 IfxStm_Mock_GetLastArg_enableComparatorInterrupt_comparator(void) { return s_enableCmpInt_lastComparator; }

void IfxStm_Mock_Reset(void) {
    s_getFrequency_count = 0; s_getFrequency_ret = 0.0f;
    s_getTicksFromUs_count = 0; s_getTicksFromUs_last = 0u; s_getTicksFromUs_ret = 0;
    s_getTicksFromMs_count = 0; s_getTicksFromMs_last = 0u; s_getTicksFromMs_ret = 0;
    s_disableCmpInt_count = 0; s_disableCmpInt_lastComparator = 0u;
    s_enableCmpInt_count = 0; s_enableCmpInt_lastComparator = 0u;
}
