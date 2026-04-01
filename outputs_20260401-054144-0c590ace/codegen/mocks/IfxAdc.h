#ifndef IFXADC_H
#define IFXADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxPort.h"

/* Enums (subset as per specification; complete enough for mock usage) */
typedef enum { IfxAdc_Apu_0, IfxAdc_Apu_1, IfxAdc_Apu_2, IfxAdc_Apu_3, IfxAdc_Apu_4, IfxAdc_Apu_5, IfxAdc_Apu_6, IfxAdc_Apu_7, IfxAdc_Apu_8, IfxAdc_Apu_9, IfxAdc_Apu_10, IfxAdc_Apu_11, IfxAdc_Apu_12, IfxAdc_Apu_13, IfxAdc_Apu_14, IfxAdc_Apu_15 } IfxAdc_Apu;

typedef enum { IfxAdc_GlobalResource_adc, IfxAdc_GlobalResource_dsadc, IfxAdc_GlobalResource_cdsp } IfxAdc_GlobalResource;

typedef enum { IfxAdc_ModuleState_enable = 0, IfxAdc_ModuleState_disable = 1 } IfxAdc_ModuleState;

typedef enum { IfxAdc_ProtE_0, IfxAdc_ProtE_1, IfxAdc_ProtE_2, IfxAdc_ProtE_3, IfxAdc_ProtE_4, IfxAdc_ProtE_5, IfxAdc_ProtE_6, IfxAdc_ProtE_7, IfxAdc_ProtE_8, IfxAdc_ProtE_9, IfxAdc_ProtE_10, IfxAdc_ProtE_11, IfxAdc_ProtE_12, IfxAdc_ProtE_13, IfxAdc_ProtE_14, IfxAdc_ProtE_15 } IfxAdc_ProtE;

typedef enum { IfxAdc_Status_success = 0, IfxAdc_Status_failure = 1 } IfxAdc_Status;

typedef enum { IfxAdc_DsadcAuxCicDecFactor_16, IfxAdc_DsadcAuxCicDecFactor_32, IfxAdc_DsadcAuxCicDecFactor_64, IfxAdc_DsadcAuxCicDecFactor_128 } IfxAdc_DsadcAuxCicDecFactor;

typedef enum { IfxAdc_DsadcBoundaryCmpMode_disable = 0, IfxAdc_DsadcBoundaryCmpMode_upperBound = 1, IfxAdc_DsadcBoundaryCmpMode_lowerBound = 2, IfxAdc_DsadcBoundaryCmpMode_bothBound = 3 } IfxAdc_DsadcBoundaryCmpMode;

typedef enum { IfxAdc_DsadcBoundaryServReq_insideBoundary = 0, IfxAdc_DsadcBoundaryServReq_outsideBoundary = 1 } IfxAdc_DsadcBoundaryServReq;

typedef enum { IfxAdc_DsadcCalibrationState_uncalibrated = 0, IfxAdc_DsadcCalibrationState_running = 1, IfxAdc_DsadcCalibrationState_calibrated = 2, IfxAdc_DsadcCalibrationState_terminatedIncorrectly = 3 } IfxAdc_DsadcCalibrationState;

typedef enum { IfxAdc_DsadcCarrierSignalDivFactor_cgp2, IfxAdc_DsadcCarrierSignalDivFactor_cgp4, IfxAdc_DsadcCarrierSignalDivFactor_cgp6, IfxAdc_DsadcCarrierSignalDivFactor_cgp8, IfxAdc_DsadcCarrierSignalDivFactor_cgp10, IfxAdc_DsadcCarrierSignalDivFactor_cgp12, IfxAdc_DsadcCarrierSignalDivFactor_cgp14, IfxAdc_DsadcCarrierSignalDivFactor_cgp16, IfxAdc_DsadcCarrierSignalDivFactor_cgp18, IfxAdc_DsadcCarrierSignalDivFactor_cgp20, IfxAdc_DsadcCarrierSignalDivFactor_cgp22, IfxAdc_DsadcCarrierSignalDivFactor_cgp24, IfxAdc_DsadcCarrierSignalDivFactor_cgp26, IfxAdc_DsadcCarrierSignalDivFactor_cgp28, IfxAdc_DsadcCarrierSignalDivFactor_cgp30, IfxAdc_DsadcCarrierSignalDivFactor_cgp32 } IfxAdc_DsadcCarrierSignalDivFactor;

typedef enum { IfxAdc_DsadcCarrierSignalPol_normal = 0, IfxAdc_DsadcCarrierSignalPol_inverted = 1 } IfxAdc_DsadcCarrierSignalPol;

typedef enum { IfxAdc_DsadcCgOperatingMode_stopped = 0, IfxAdc_DsadcCgOperatingMode_squareWave = 1, IfxAdc_DsadcCgOperatingMode_triangleWave = 2, IfxAdc_DsadcCgOperatingMode_cosineWave = 3 } IfxAdc_DsadcCgOperatingMode;

typedef enum { IfxAdc_DsadcCicDecimationRate_128, IfxAdc_DsadcCicDecimationRate_256 } IfxAdc_DsadcCicDecimationRate;

typedef enum {
    IfxAdc_DsadcCicShift_0to16, IfxAdc_DsadcCicShift_1to17, IfxAdc_DsadcCicShift_2to18, IfxAdc_DsadcCicShift_3to19, IfxAdc_DsadcCicShift_4to20,
    IfxAdc_DsadcCicShift_5to21, IfxAdc_DsadcCicShift_6to22, IfxAdc_DsadcCicShift_7to23, IfxAdc_DsadcCicShift_8to24, IfxAdc_DsadcCicShift_9to25,
    IfxAdc_DsadcCicShift_10to26, IfxAdc_DsadcCicShift_11to27, IfxAdc_DsadcCicShift_12to28, IfxAdc_DsadcCicShift_13to29, IfxAdc_DsadcCicShift_14to30,
    IfxAdc_DsadcCicShift_15to31, IfxAdc_DsadcCicShift_16to32, IfxAdc_DsadcCicShift_17to33, IfxAdc_DsadcCicShift_18to34, IfxAdc_DsadcCicShift_19to35,
    IfxAdc_DsadcCicShift_20to36, IfxAdc_DsadcCicShift_21to37, IfxAdc_DsadcCicShift_22to38, IfxAdc_DsadcCicShift_23to39, IfxAdc_DsadcCicShift_24to40,
    IfxAdc_DsadcCicShift_25to41, IfxAdc_DsadcCicShift_26to42, IfxAdc_DsadcCicShift_27to43, IfxAdc_DsadcCicShift_28to44
} IfxAdc_DsadcCicShift;

typedef enum { IfxAdc_DsadcDataReadWidth_16Bit = 0, IfxAdc_DsadcDataReadWidth_32Bit = 2 } IfxAdc_DsadcDataReadWidth;

typedef enum { IfxAdc_DsadcDitherTrim_disabled = -1, IfxAdc_DsadcDitherTrim_level0, IfxAdc_DsadcDitherTrim_level1, IfxAdc_DsadcDitherTrim_level2, IfxAdc_DsadcDitherTrim_level3 } IfxAdc_DsadcDitherTrim;

typedef enum { IfxAdc_DsadcFifoSrLevel_1, IfxAdc_DsadcFifoSrLevel_2, IfxAdc_DsadcFifoSrLevel_3, IfxAdc_DsadcFifoSrLevel_4 } IfxAdc_DsadcFifoSrLevel;

typedef enum { IfxAdc_DsadcInCfg_inputPin = 0, IfxAdc_DsadcInCfg_vref = 1, IfxAdc_DsadcInCfg_vddmBy2 = 2, IfxAdc_DsadcInCfg_ground = 3 } IfxAdc_DsadcInCfg;

typedef enum { IfxAdc_DsadcInSel_a = 0, IfxAdc_DsadcInSel_b = 1, IfxAdc_DsadcInSel_c = 2, IfxAdc_DsadcInSel_d = 3 } IfxAdc_DsadcInSel;

typedef enum { IfxAdc_DsadcInputMuxActCtrl_preset = 0, IfxAdc_DsadcInputMuxActCtrl_singleStep = 1 } IfxAdc_DsadcInputMuxActCtrl;

typedef enum { IfxAdc_DsadcIntegratorShift_bits4To20, IfxAdc_DsadcIntegratorShift_bits5To21, IfxAdc_DsadcIntegratorShift_bits6To22, IfxAdc_DsadcIntegratorShift_bits7To23, IfxAdc_DsadcIntegratorShift_bits8To24, IfxAdc_DsadcIntegratorShift_bits9To25, IfxAdc_DsadcIntegratorShift_bits10To26 } IfxAdc_DsadcIntegratorShift;

typedef enum { IfxAdc_DsadcIntegratorTriggerMode_none, IfxAdc_DsadcIntegratorTriggerMode_risingEdge, IfxAdc_DsadcIntegratorTriggerMode_fallingEdge, IfxAdc_DsadcIntegratorTriggerMode_alwaysActive } IfxAdc_DsadcIntegratorTriggerMode;

typedef enum { IfxAdc_DsadcIntegratorWindowCtrl_internal = 0, IfxAdc_DsadcIntegratorWindowCtrl_external = 1, IfxAdc_DsadcIntegratorWindowCtrl_software = 2 } IfxAdc_DsadcIntegratorWindowCtrl;

typedef enum { IfxAdc_DsadcModulatorFreq_40MHz = 0, IfxAdc_DsadcModulatorFreq_20MHz = 1, IfxAdc_DsadcModulatorFreq_10MHz = 2 } IfxAdc_DsadcModulatorFreq;

typedef enum { IfxAdc_DsadcOffCompFilter_disabled, IfxAdc_DsadcOffCompFilter_cutoffRate1, IfxAdc_DsadcOffCompFilter_cutoffRate2, IfxAdc_DsadcOffCompFilter_cutoffRate3, IfxAdc_DsadcOffCompFilter_cutoffRate4, IfxAdc_DsadcOffCompFilter_cutoffRate5, IfxAdc_DsadcOffCompFilter_cutoffRate6, IfxAdc_DsadcOffCompFilter_cutoffRate7 } IfxAdc_DsadcOffCompFilter;

typedef enum { IfxAdc_DsadcPwmGeneration_normal = 0, IfxAdc_DsadcPwmGeneration_bitReverse = 1 } IfxAdc_DsadcPwmGeneration;

typedef enum { IfxAdc_DsadcRectifierSignSource_onchip = 0, IfxAdc_DsadcRectifierSignSource_selectedchannel = 1, IfxAdc_DsadcRectifierSignSource_extsignalA = 2, IfxAdc_DsadcRectifierSignSource_extsignalB = 3 } IfxAdc_DsadcRectifierSignSource;

typedef enum { IfxAdc_DsadcSrvReq0_never, IfxAdc_DsadcSrvReq0_gateHigh, IfxAdc_DsadcSrvReq0_gateLow, IfxAdc_DsadcSrvReq0_always } IfxAdc_DsadcSrvReq0;

typedef enum { IfxAdc_DsadcSrvReq3_never = 0, IfxAdc_DsadcSrvReq3_timestamp = 1, IfxAdc_DsadcSrvReq3_cgSync = 2 } IfxAdc_DsadcSrvReq3;

typedef enum { IfxAdc_DsadcSupplyLevel_between3_7V_And_5_5V = 0, IfxAdc_DsadcSupplyLevel_lessThan3_7 = 1 } IfxAdc_DsadcSupplyLevel;

typedef enum { IfxAdc_DsadcTimestampClock_fmod = 0, IfxAdc_DsadcTimestampClock_fmoddiv2 = 1, IfxAdc_DsadcTimestampClock_fmoddiv4 = 2, IfxAdc_DsadcTimestampClock_fmoddiv8 = 3 } IfxAdc_DsadcTimestampClock;

typedef enum { IfxAdc_DsadcTriggerMode_none, IfxAdc_DsadcTriggerMode_risingEdge, IfxAdc_DsadcTriggerMode_fallingEdge, IfxAdc_DsadcTriggerMode_bothEdge } IfxAdc_DsadcTriggerMode;

/* TMADC-related enums used by Tmadc driver */
typedef enum { IfxAdc_TmadcBoundaryCmpMode_disable = 0, IfxAdc_TmadcBoundaryCmpMode_upperBound = 1, IfxAdc_TmadcBoundaryCmpMode_lowerBound = 2, IfxAdc_TmadcBoundaryCmpMode_bothBound = 3 } IfxAdc_TmadcBoundaryCmpMode;

typedef enum { IfxAdc_TmadcBoundaryReg_0 = 0, IfxAdc_TmadcBoundaryReg_1 = 1 } IfxAdc_TmadcBoundaryReg;

typedef enum { IfxAdc_TmadcEmuxCodingScheme_binary = 0, IfxAdc_TmadcEmuxCodingScheme_gray = 1 } IfxAdc_TmadcEmuxCodingScheme;

typedef enum { IfxAdc_TmadcEventOp_orLogicWithoutWfc = 0, IfxAdc_TmadcEventOp_orLogicWithWfc = 1, IfxAdc_TmadcEventOp_andLogic = 2 } IfxAdc_TmadcEventOp;

typedef enum { IfxAdc_TmadcEventSel_disable = 0, IfxAdc_TmadcEventSel_error = 1, IfxAdc_TmadcEventSel_result = 2, IfxAdc_TmadcEventSel_boundary = 3 } IfxAdc_TmadcEventSel;

typedef enum { IfxAdc_TmadcGlobalServReq_0, IfxAdc_TmadcGlobalServReq_1, IfxAdc_TmadcGlobalServReq_2, IfxAdc_TmadcGlobalServReq_3 } IfxAdc_TmadcGlobalServReq;

typedef enum { IfxAdc_TmadcMonitorChannel_0 = 0, IfxAdc_TmadcMonitorChannel_1 = 1 } IfxAdc_TmadcMonitorChannel;

typedef enum { IfxAdc_TmadcMonitorChannelInput_coreSupply = 0, IfxAdc_TmadcMonitorChannelInput_otherModules = 1, IfxAdc_TmadcMonitorChannelInput_vssm = 2, IfxAdc_TmadcMonitorChannelInput_csd = 3 } IfxAdc_TmadcMonitorChannelInput;

typedef enum { IfxAdc_TmadcOpMode_oneShot = 0, IfxAdc_TmadcOpMode_continuous = 1 } IfxAdc_TmadcOpMode;

typedef enum { IfxAdc_TmadcOutputSupervisorMux_noConnection = 0, IfxAdc_TmadcOutputSupervisorMux_vddk0 = 1, IfxAdc_TmadcOutputSupervisorMux_vddk1 = 2, IfxAdc_TmadcOutputSupervisorMux_vssm = 3 } IfxAdc_TmadcOutputSupervisorMux;

typedef enum { IfxAdc_TmadcResultReg_0, IfxAdc_TmadcResultReg_1, IfxAdc_TmadcResultReg_2, IfxAdc_TmadcResultReg_3, IfxAdc_TmadcResultReg_4, IfxAdc_TmadcResultReg_5, IfxAdc_TmadcResultReg_6, IfxAdc_TmadcResultReg_7, IfxAdc_TmadcResultReg_8, IfxAdc_TmadcResultReg_9, IfxAdc_TmadcResultReg_10, IfxAdc_TmadcResultReg_11, IfxAdc_TmadcResultReg_12, IfxAdc_TmadcResultReg_13, IfxAdc_TmadcResultReg_14, IfxAdc_TmadcResultReg_15, IfxAdc_TmadcResultReg_count } IfxAdc_TmadcResultReg;

typedef enum { IfxAdc_TmadcSarCore_0, IfxAdc_TmadcSarCore_1 } IfxAdc_TmadcSarCore;

typedef enum { IfxAdc_TmadcServReq_none = -1, IfxAdc_TmadcServReq_0, IfxAdc_TmadcServReq_1, IfxAdc_TmadcServReq_2, IfxAdc_TmadcServReq_3, IfxAdc_TmadcServReq_4, IfxAdc_TmadcServReq_5, IfxAdc_TmadcServReq_6 } IfxAdc_TmadcServReq;

typedef enum { IfxAdc_TmadcSuspendMode_dsiable = 0, IfxAdc_TmadcSuspendMode_hard = 1, IfxAdc_TmadcSuspendMode_soft_activeConv = 2, IfxAdc_TmadcSuspendMode_soft_holdConv = 3 } IfxAdc_TmadcSuspendMode;

typedef enum { IfxAdc_TmadcTriggerMode_disable = 0, IfxAdc_TmadcTriggerMode_risingEdge = 1, IfxAdc_TmadcTriggerMode_fallingEdge = 2, IfxAdc_TmadcTriggerMode_bothEdge = 3 } IfxAdc_TmadcTriggerMode;

typedef enum { IfxAdc_TmadcTriggerMuxSel_0 = 0, IfxAdc_TmadcTriggerMuxSel_1, IfxAdc_TmadcTriggerMuxSel_2, IfxAdc_TmadcTriggerMuxSel_3, IfxAdc_TmadcTriggerMuxSel_4, IfxAdc_TmadcTriggerMuxSel_5, IfxAdc_TmadcTriggerMuxSel_6, IfxAdc_TmadcTriggerMuxSel_7, IfxAdc_TmadcTriggerMuxSel_8, IfxAdc_TmadcTriggerMuxSel_9, IfxAdc_TmadcTriggerMuxSel_10, IfxAdc_TmadcTriggerMuxSel_11, IfxAdc_TmadcTriggerMuxSel_12, IfxAdc_TmadcTriggerMuxSel_13, IfxAdc_TmadcTriggerMuxSel_14, IfxAdc_TmadcTriggerMuxSel_15, IfxAdc_TmadcTriggerMuxSel_16, IfxAdc_TmadcTriggerMuxSel_17, IfxAdc_TmadcTriggerMuxSel_18, IfxAdc_TmadcTriggerMuxSel_19, IfxAdc_TmadcTriggerMuxSel_20, IfxAdc_TmadcTriggerMuxSel_21, IfxAdc_TmadcTriggerMuxSel_22, IfxAdc_TmadcTriggerMuxSel_23, IfxAdc_TmadcTriggerMuxSel_24, IfxAdc_TmadcTriggerMuxSel_25, IfxAdc_TmadcTriggerMuxSel_26, IfxAdc_TmadcTriggerMuxSel_27, IfxAdc_TmadcTriggerMuxSel_28, IfxAdc_TmadcTriggerMuxSel_29, IfxAdc_TmadcTriggerMuxSel_30, IfxAdc_TmadcTriggerMuxSel_31, IfxAdc_TmadcTriggerMuxSel_32, IfxAdc_TmadcTriggerMuxSel_33, IfxAdc_TmadcTriggerMuxSel_34, IfxAdc_TmadcTriggerMuxSel_35, IfxAdc_TmadcTriggerMuxSel_36, IfxAdc_TmadcTriggerMuxSel_37, IfxAdc_TmadcTriggerMuxSel_38, IfxAdc_TmadcTriggerMuxSel_39, IfxAdc_TmadcTriggerMuxSel_40, IfxAdc_TmadcTriggerMuxSel_41, IfxAdc_TmadcTriggerMuxSel_42, IfxAdc_TmadcTriggerMuxSel_43, IfxAdc_TmadcTriggerMuxSel_44, IfxAdc_TmadcTriggerMuxSel_45, IfxAdc_TmadcTriggerMuxSel_46, IfxAdc_TmadcTriggerMuxSel_47, IfxAdc_TmadcTriggerMuxSel_48, IfxAdc_TmadcTriggerMuxSel_49, IfxAdc_TmadcTriggerMuxSel_50, IfxAdc_TmadcTriggerMuxSel_51, IfxAdc_TmadcTriggerMuxSel_52, IfxAdc_TmadcTriggerMuxSel_53, IfxAdc_TmadcTriggerMuxSel_54, IfxAdc_TmadcTriggerMuxSel_55, IfxAdc_TmadcTriggerMuxSel_56, IfxAdc_TmadcTriggerMuxSel_57, IfxAdc_TmadcTriggerMuxSel_58, IfxAdc_TmadcTriggerMuxSel_59, IfxAdc_TmadcTriggerMuxSel_60, IfxAdc_TmadcTriggerMuxSel_61, IfxAdc_TmadcTriggerMuxSel_62, IfxAdc_TmadcTriggerMuxSel_63, IfxAdc_TmadcTriggerMuxSel_count } IfxAdc_TmadcTriggerMuxSel;

/* Function declarations used by mocks/tests */
void IfxAdc_enableModule(Ifx_ADC *modSFR);

#endif /* IFXADC_H */
