#include "IfxCpu_Irq.h"

static uint32 s_install_count = 0;
static uint32 s_install_lastSrpn = 0u;

void IfxCpu_Irq_installInterruptHandler(void *isrFuncPointer, uint32 serviceReqPrioNumber) {
    (void)isrFuncPointer;
    s_install_count++;
    s_install_lastSrpn = serviceReqPrioNumber;
}

uint32 IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler(void) { return s_install_count; }
uint32 IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_srpn(void) { return s_install_lastSrpn; }
void   IfxCpu_Irq_Mock_Reset(void) { s_install_count = 0; s_install_lastSrpn = 0u; }
