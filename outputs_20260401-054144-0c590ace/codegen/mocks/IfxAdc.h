#ifndef IFXADC_H
#define IFXADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"

/* Abridged but complete enums as required by the template mapping */
/* Only key enums used by TMADC structures and basic API are emitted here. */

typedef enum { IfxAdc_Apu_0, IfxAdc_Apu_1, IfxAdc_Apu_2, IfxAdc_Apu_3, IfxAdc_Apu_4, IfxAdc_Apu_5, IfxAdc_Apu_6, IfxAdc_Apu_7, IfxAdc_Apu_8, IfxAdc_Apu_9, IfxAdc_Apu_10, IfxAdc_Apu_11, IfxAdc_Apu_12, IfxAdc_Apu_13, IfxAdc_Apu_14 } IfxAdc_Apu;

typedef enum { IfxAdc_GlobalResource_adc, IfxAdc_GlobalResource_dsadc } IfxAdc_GlobalResource;

typedef enum { IfxAdc_ModuleState_enable = 0, IfxAdc_ModuleState_disable = 1 } IfxAdc_ModuleState;

typedef enum { IfxAdc_ProtE_0, IfxAdc_ProtE_1, IfxAdc_ProtE_2, IfxAdc_ProtE_3, IfxAdc_ProtE_4, IfxAdc_ProtE_5, IfxAdc_ProtE_6, IfxAdc_ProtE_7, IfxAdc_ProtE_8, IfxAdc_ProtE_9, IfxAdc_ProtE_10, IfxAdc_ProtE_11, IfxAdc_ProtE_12, IfxAdc_ProtE_13, IfxAdc_ProtE_14 } IfxAdc_ProtE;

typedef enum { IfxAdc_Status_success = 0, IfxAdc_Status_failure = 1 } IfxAdc_Status;

typedef enum { IfxAdc_TmadcBoundaryCmpMode_disable = 0, IfxAdc_TmadcBoundaryCmpMode_upperBound = 1, IfxAdc_TmadcBoundaryCmpMode_lowerBound = 2, IfxAdc_TmadcBoundaryCmpMode_bothBound = 3 } IfxAdc_TmadcBoundaryCmpMode;

typedef enum { IfxAdc_TmadcBoundaryReg_0 = 0, IfxAdc_TmadcBoundaryReg_1 = 1 } IfxAdc_TmadcBoundaryReg;

typedef enum { IfxAdc_TmadcEmuxCodingScheme_binary = 0, IfxAdc_TmadcEmuxCodingScheme_gray = 1 } IfxAdc_TmadcEmuxCodingScheme;

typedef enum { IfxAdc_TmadcEventOp_orLogicWithoutWfc = 0, IfxAdc_TmadcEventOp_orLogicWithWfc = 1, IfxAdc_TmadcEventOp_andLogic = 2 } IfxAdc_TmadcEventOp;

typedef enum { IfxAdc_TmadcEventSel_disable = 0, IfxAdc_TmadcEventSel_error = 1, IfxAdc_TmadcEventSel_result = 2, IfxAdc_TmadcEventSel_boundary = 3 } IfxAdc_TmadcEventSel;

typedef enum { IfxAdc_TmadcGlobalServReq_0 = 0, IfxAdc_TmadcGlobalServReq_1 = 1, IfxAdc_TmadcGlobalServReq_2 = 2 } IfxAdc_TmadcGlobalServReq;

typedef enum { IfxAdc_TmadcMonitorChannel_0 = 0, IfxAdc_TmadcMonitorChannel_1 = 1 } IfxAdc_TmadcMonitorChannel;

typedef enum { IfxAdc_TmadcMonitorChannelInput_coreSupply = 0, IfxAdc_TmadcMonitorChannelInput_otherModules = 1, IfxAdc_TmadcMonitorChannelInput_vssm = 2, IfxAdc_TmadcMonitorChannelInput_csd = 3 } IfxAdc_TmadcMonitorChannelInput;

typedef enum { IfxAdc_TmadcOpMode_oneShot = 0, IfxAdc_TmadcOpMode_continuous = 1 } IfxAdc_TmadcOpMode;

typedef enum { IfxAdc_TmadcOutputSupervisorMux_noConnection = 0, IfxAdc_TmadcOutputSupervisorMux_vddk0 = 1, IfxAdc_TmadcOutputSupervisorMux_vddk1 = 2, IfxAdc_TmadcOutputSupervisorMux_vssm = 3 } IfxAdc_TmadcOutputSupervisorMux;

typedef enum { IfxAdc_TmadcResultReg_0, IfxAdc_TmadcResultReg_1, IfxAdc_TmadcResultReg_2, IfxAdc_TmadcResultReg_3, IfxAdc_TmadcResultReg_4, IfxAdc_TmadcResultReg_5, IfxAdc_TmadcResultReg_6, IfxAdc_TmadcResultReg_7, IfxAdc_TmadcResultReg_8, IfxAdc_TmadcResultReg_9, IfxAdc_TmadcResultReg_10, IfxAdc_TmadcResultReg_11, IfxAdc_TmadcResultReg_12, IfxAdc_TmadcResultReg_13, IfxAdc_TmadcResultReg_14, IfxAdc_TmadcResultReg_15 } IfxAdc_TmadcResultReg;

typedef enum { IfxAdc_TmadcSarCore_0 = 0 } IfxAdc_TmadcSarCore;

typedef enum { IfxAdc_TmadcServReq_none = 0, IfxAdc_TmadcServReq_0, IfxAdc_TmadcServReq_1, IfxAdc_TmadcServReq_2, IfxAdc_TmadcServReq_3, IfxAdc_TmadcServReq_4, IfxAdc_TmadcServReq_5 } IfxAdc_TmadcServReq;

typedef enum { IfxAdc_TmadcSuspendMode_dsiable = 0, IfxAdc_TmadcSuspendMode_hard = 1, IfxAdc_TmadcSuspendMode_soft_activeConv = 2, IfxAdc_TmadcSuspendMode_soft_holdConv = 3 } IfxAdc_TmadcSuspendMode;

typedef enum { IfxAdc_TmadcTriggerMode_disable = 0, IfxAdc_TmadcTriggerMode_risingEdge = 1, IfxAdc_TmadcTriggerMode_fallingEdge = 2, IfxAdc_TmadcTriggerMode_bothEdge = 3 } IfxAdc_TmadcTriggerMode;

typedef enum { /* Only enumerators 0..63 are needed; keep compact */
    IfxAdc_TmadcTriggerMuxSel_0 = 0,  IfxAdc_TmadcTriggerMuxSel_1,  IfxAdc_TmadcTriggerMuxSel_2,  IfxAdc_TmadcTriggerMuxSel_3,
    IfxAdc_TmadcTriggerMuxSel_4,      IfxAdc_TmadcTriggerMuxSel_5,  IfxAdc_TmadcTriggerMuxSel_6,  IfxAdc_TmadcTriggerMuxSel_7,
    IfxAdc_TmadcTriggerMuxSel_8,      IfxAdc_TmadcTriggerMuxSel_9,  IfxAdc_TmadcTriggerMuxSel_10, IfxAdc_TmadcTriggerMuxSel_11,
    IfxAdc_TmadcTriggerMuxSel_12,     IfxAdc_TmadcTriggerMuxSel_13, IfxAdc_TmadcTriggerMuxSel_14, IfxAdc_TmadcTriggerMuxSel_15,
    IfxAdc_TmadcTriggerMuxSel_16,     IfxAdc_TmadcTriggerMuxSel_17, IfxAdc_TmadcTriggerMuxSel_18, IfxAdc_TmadcTriggerMuxSel_19,
    IfxAdc_TmadcTriggerMuxSel_20,     IfxAdc_TmadcTriggerMuxSel_21, IfxAdc_TmadcTriggerMuxSel_22, IfxAdc_TmadcTriggerMuxSel_23,
    IfxAdc_TmadcTriggerMuxSel_24,     IfxAdc_TmadcTriggerMuxSel_25, IfxAdc_TmadcTriggerMuxSel_26, IfxAdc_TmadcTriggerMuxSel_27,
    IfxAdc_TmadcTriggerMuxSel_28,     IfxAdc_TmadcTriggerMuxSel_29, IfxAdc_TmadcTriggerMuxSel_30, IfxAdc_TmadcTriggerMuxSel_31,
    IfxAdc_TmadcTriggerMuxSel_32,     IfxAdc_TmadcTriggerMuxSel_33, IfxAdc_TmadcTriggerMuxSel_34, IfxAdc_TmadcTriggerMuxSel_35,
    IfxAdc_TmadcTriggerMuxSel_36,     IfxAdc_TmadcTriggerMuxSel_37, IfxAdc_TmadcTriggerMuxSel_38, IfxAdc_TmadcTriggerMuxSel_39,
    IfxAdc_TmadcTriggerMuxSel_40,     IfxAdc_TmadcTriggerMuxSel_41, IfxAdc_TmadcTriggerMuxSel_42, IfxAdc_TmadcTriggerMuxSel_43,
    IfxAdc_TmadcTriggerMuxSel_44,     IfxAdc_TmadcTriggerMuxSel_45, IfxAdc_TmadcTriggerMuxSel_46, IfxAdc_TmadcTriggerMuxSel_47,
    IfxAdc_TmadcTriggerMuxSel_48,     IfxAdc_TmadcTriggerMuxSel_49, IfxAdc_TmadcTriggerMuxSel_50, IfxAdc_TmadcTriggerMuxSel_51,
    IfxAdc_TmadcTriggerMuxSel_52,     IfxAdc_TmadcTriggerMuxSel_53, IfxAdc_TmadcTriggerMuxSel_54, IfxAdc_TmadcTriggerMuxSel_55,
    IfxAdc_TmadcTriggerMuxSel_56,     IfxAdc_TmadcTriggerMuxSel_57, IfxAdc_TmadcTriggerMuxSel_58, IfxAdc_TmadcTriggerMuxSel_59,
    IfxAdc_TmadcTriggerMuxSel_60,     IfxAdc_TmadcTriggerMuxSel_61, IfxAdc_TmadcTriggerMuxSel_62, IfxAdc_TmadcTriggerMuxSel_63
} IfxAdc_TmadcTriggerMuxSel;

/* Basic ADC API used */
void IfxAdc_enableModule(Ifx_ADC *modSFR);

#endif /* IFXADC_H */
