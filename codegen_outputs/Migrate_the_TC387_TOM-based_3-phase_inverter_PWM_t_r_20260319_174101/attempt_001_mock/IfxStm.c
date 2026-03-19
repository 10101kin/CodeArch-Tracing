#include "IfxStm.h"

static uint32 s_getFrequency_count = 0u;
static uint32 s_getTicksUs_count = 0u;
static uint32 s_getTicksMs_count = 0u;
static uint32 s_disableCompIrq_count = 0u;
static uint32 s_enableCompIrq_count = 0u;

static uint64 s_getFrequency_ret = 0ull;
static uint64 s_getTicksUs_ret = 0ull;
static uint64 s_getTicksMs_ret = 0ull;

static uint32 s_getTicksUs_last = 0u;
static uint32 s_getTicksMs_last = 0u;
static uint32 s_disableCompIrq_lastComparator = 0u;
static uint32 s_enableCompIrq_lastComparator = 0u;

uint64 IfxStm_getFrequency(void)
{
    s_getFrequency_count++;
    return s_getFrequency_ret;
}

uint64 IfxStm_getTicksFromMicroseconds(uint32 microSeconds)
{
    s_getTicksUs_count++;
    s_getTicksUs_last = microSeconds;
    return s_getTicksUs_ret;
}

uint64 IfxStm_getTicksFromMilliseconds(uint32 milliSeconds)
{
    s_getTicksMs_count++;
    s_getTicksMs_last = milliSeconds;
    return s_getTicksMs_ret;
}

void IfxStm_disableComparatorInterrupt(Ifx_CPU *cpu, IfxStm_Comparator comparator)
{
    (void)cpu;
    s_disableCompIrq_count++;
    s_disableCompIrq_lastComparator = (uint32)comparator;
}

void IfxStm_enableComparatorInterrupt(Ifx_CPU *cpu, IfxStm_Comparator comparator)
{
    (void)cpu;
    s_enableCompIrq_count++;
    s_enableCompIrq_lastComparator = (uint32)comparator;
}

uint32 IfxStm_Mock_GetCallCount_getFrequency(void) { return s_getFrequency_count; }
uint32 IfxStm_Mock_GetCallCount_getTicksFromMicroseconds(void) { return s_getTicksUs_count; }
uint32 IfxStm_Mock_GetCallCount_getTicksFromMilliseconds(void) { return s_getTicksMs_count; }
uint32 IfxStm_Mock_GetCallCount_disableComparatorInterrupt(void) { return s_disableCompIrq_count; }
uint32 IfxStm_Mock_GetCallCount_enableComparatorInterrupt(void) { return s_enableCompIrq_count; }

void   IfxStm_Mock_SetReturn_getFrequency(uint64 ret) { s_getFrequency_ret = ret; }
void   IfxStm_Mock_SetReturn_getTicksFromMicroseconds(uint64 ret) { s_getTicksUs_ret = ret; }
void   IfxStm_Mock_SetReturn_getTicksFromMilliseconds(uint64 ret) { s_getTicksMs_ret = ret; }

uint32 IfxStm_Mock_GetLastArg_getTicksFromMicroseconds_us(void) { return s_getTicksUs_last; }
uint32 IfxStm_Mock_GetLastArg_getTicksFromMilliseconds_ms(void) { return s_getTicksMs_last; }
uint32 IfxStm_Mock_GetLastArg_disableComparatorInterrupt_comparator(void) { return s_disableCompIrq_lastComparator; }
uint32 IfxStm_Mock_GetLastArg_enableComparatorInterrupt_comparator(void) { return s_enableCompIrq_lastComparator; }

void IfxStm_Mock_Reset(void)
{
    s_getFrequency_count = 0u;
    s_getTicksUs_count = 0u;
    s_getTicksMs_count = 0u;
    s_disableCompIrq_count = 0u;
    s_enableCompIrq_count = 0u;

    s_getFrequency_ret = 0ull;
    s_getTicksUs_ret = 0ull;
    s_getTicksMs_ret = 0ull;

    s_getTicksUs_last = 0u;
    s_getTicksMs_last = 0u;
    s_disableCompIrq_lastComparator = 0u;
    s_enableCompIrq_lastComparator = 0u;
}
