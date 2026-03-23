#include "IfxVmt.h"

static uint32 s_cnt_IfxCcu6_enableModule = 0u;

void IfxCcu6_enableModule(void) {
    s_cnt_IfxCcu6_enableModule++;
}

uint32 IfxVmt_Mock_GetCallCount_IfxCcu6_enableModule(void) {
    return s_cnt_IfxCcu6_enableModule;
}

void IfxVmt_Mock_Reset(void) {
    s_cnt_IfxCcu6_enableModule = 0u;
}
