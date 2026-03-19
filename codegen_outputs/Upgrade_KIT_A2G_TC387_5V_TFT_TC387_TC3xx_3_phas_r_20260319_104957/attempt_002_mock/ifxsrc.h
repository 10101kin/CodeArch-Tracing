#ifndef IFXSRC_H
#define IFXSRC_H

#include "illd_types/Ifx_Types.h"

/*
 * Auto-generated stub for IfxSrc.h
 * This header was included by other mock headers but didn't exist.
 * Add type/function definitions here as needed.
 */

/* SRC (Service Request Control) types */
/* Ifx_SRC_SRCR - defined in Ifx_Types.h */
/* IfxSrc_Tos - from illd_types/Ifx_Types.h */

void IfxSrc_init(volatile Ifx_SRC_SRCR *src, IfxSrc_Tos tos, uint16 priority);
void IfxSrc_enable(volatile Ifx_SRC_SRCR *src);
void IfxSrc_disable(volatile Ifx_SRC_SRCR *src);
void IfxSrc_clearRequest(volatile Ifx_SRC_SRCR *src);

#endif /* IFXSRC_H */
