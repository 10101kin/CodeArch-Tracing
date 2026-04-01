#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H

#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxPort.h"

/* SFR-like minimal types used by TMADC driver */
typedef struct { uint32 reserved; } Ifx_ADC_TMADC;
typedef struct { uint32 reserved; } Ifx_ADC_TMADC_CH;
typedef struct { uint32 reserved; } Ifx_ADC_RES;
typedef struct { uint32 reserved; } Ifx_ADC_TS;
typedef struct { uint32 reserved; } Ifx_ADC_TMADC_MRES;

/* DMA and callback placeholders */
typedef void (*IfxAdc_Tmadc_Callback)(void *);
typedef uint32 IfxDma_Dma_Channel;
typedef uint32 IfxDma_Index;
typedef uint32 IfxDma_ChannelId;

typedef struct { uint32 reserved; } IfxAdc_Emuxctrl_Out;
typedef struct { uint32 reserved; } IfxAdc_TmadcCh_In;

#ifndef IFXADC_TMADC_MAX_SERV_REQ_NODE
#define IFXADC_TMADC_MAX_SERV_REQ_NODE 7
#endif

/* Basic enums used by TMADC API subset */
typedef enum { IfxAdc_Tmadc_BufferType_linear = 0, IfxAdc_Tmadc_BufferType_linearWithTimestamp = 1, IfxAdc_Tmadc_BufferType_circular = 2 } IfxAdc_Tmadc_BufferType;

typedef enum { IfxAdc_Tmadc_channelState_uninitialized = 0, IfxAdc_Tmadc_channelState_initialized = 1 } IfxAdc_Tmadc_channelState;

typedef enum {
    IfxAdc_Tmadc_moduleState_unknown = 0,
    IfxAdc_Tmadc_moduleState_calibration = 1,
    IfxAdc_Tmadc_moduleState_calibrationError = 2,
    IfxAdc_Tmadc_moduleState_initialized = 3,
    IfxAdc_Tmadc_moduleState_run = 4,
    IfxAdc_Tmadc_moduleState_notPoweredOn = 5
} IfxAdc_Tmadc_moduleState;

/* Extra enums/ids to support structs */
typedef enum { IfxAdc_TmadcTriggerMode_disable = 0, IfxAdc_TmadcTriggerMode_risingEdge = 1, IfxAdc_TmadcTriggerMode_fallingEdge = 2, IfxAdc_TmadcTriggerMode_bothEdge = 3 } IfxAdc_TmadcTriggerMode;

typedef enum { IfxAdc_TmadcTriggerMuxSel_0 = 0 } IfxAdc_TmadcTriggerMuxSel;

typedef enum { IfxAdc_TmadcBoundaryCmpMode_disable = 0, IfxAdc_TmadcBoundaryCmpMode_upperBound = 1, IfxAdc_TmadcBoundaryCmpMode_lowerBound = 2, IfxAdc_TmadcBoundaryCmpMode_bothBound = 3 } IfxAdc_TmadcBoundaryCmpMode;

typedef enum { IfxAdc_TmadcBoundaryReg_0 = 0, IfxAdc_TmadcBoundaryReg_1 = 1 } IfxAdc_TmadcBoundaryReg;

typedef enum { IfxAdc_TmadcServReq_none = -1, IfxAdc_TmadcServReq_0 = 0, IfxAdc_TmadcServReq_1, IfxAdc_TmadcServReq_2, IfxAdc_TmadcServReq_3, IfxAdc_TmadcServReq_4, IfxAdc_TmadcServReq_5, IfxAdc_TmadcServReq_6 } IfxAdc_TmadcServReq;

typedef enum { IfxAdc_TmadcEventOp_orLogicWithoutWfc = 0, IfxAdc_TmadcEventOp_orLogicWithWfc = 1, IfxAdc_TmadcEventOp_andLogic = 2 } IfxAdc_TmadcEventOp;

typedef enum { IfxAdc_TmadcEventSel_disable = 0, IfxAdc_TmadcEventSel_error = 1, IfxAdc_TmadcEventSel_result = 2, IfxAdc_TmadcEventSel_boundary = 3 } IfxAdc_TmadcEventSel;

typedef enum { IfxAdc_TmadcGlobalServReq_0 = 0, IfxAdc_TmadcGlobalServReq_1 = 1, IfxAdc_TmadcGlobalServReq_2 = 2 } IfxAdc_TmadcGlobalServReq;

typedef enum { IfxAdc_TmadcOpMode_oneShot = 0, IfxAdc_TmadcOpMode_continuous = 1 } IfxAdc_TmadcOpMode;

typedef enum { IfxAdc_TmadcMonitorChannel_0 = 0, IfxAdc_TmadcMonitorChannel_1 = 1 } IfxAdc_TmadcMonitorChannel;

typedef enum { IfxAdc_TmadcMonitorChannelInput_coreSupply = 0, IfxAdc_TmadcMonitorChannelInput_otherModules = 1, IfxAdc_TmadcMonitorChannelInput_vssm = 2, IfxAdc_TmadcMonitorChannelInput_csd = 3 } IfxAdc_TmadcMonitorChannelInput;

typedef enum {
    IfxAdc_TmadcResultReg_0 = 0, IfxAdc_TmadcResultReg_1, IfxAdc_TmadcResultReg_2, IfxAdc_TmadcResultReg_3,
    IfxAdc_TmadcResultReg_4, IfxAdc_TmadcResultReg_5, IfxAdc_TmadcResultReg_6, IfxAdc_TmadcResultReg_7,
    IfxAdc_TmadcResultReg_8, IfxAdc_TmadcResultReg_9, IfxAdc_TmadcResultReg_10, IfxAdc_TmadcResultReg_11,
    IfxAdc_TmadcResultReg_12, IfxAdc_TmadcResultReg_13, IfxAdc_TmadcResultReg_14, IfxAdc_TmadcResultReg_15
} IfxAdc_TmadcResultReg;

typedef enum { IfxAdc_TmadcSarCore_0 = 0 } IfxAdc_TmadcSarCore;

typedef enum { IfxAdc_TmadcModule_0 = 0 } IfxAdc_TmadcModule;

typedef enum { IfxAdc_TmadcChannel_0 = 0 } IfxAdc_TmadcChannel;

/* Small struct used in template */
typedef struct { uint32 timeStamp; uint32 results; } typeName;

typedef struct { IfxAdc_TmadcTriggerMuxSel muxSel; IfxAdc_TmadcTriggerMode edgeSel; } IfxAdc_Tmadc_HwTriggerConfig;

typedef struct { IfxAdc_TmadcResultReg resultRegSel; boolean enable; } IfxAdc_Tmadc_BoundaryFlagCfg;

typedef struct { Ifx_Priority priority; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; } IfxAdc_Tmadc_DmaSrvReq;

typedef struct { IfxAdc_Tmadc_HwTriggerConfig hwTrigger; float32 delayNS; } IfxAdc_Tmadc_TriggerConfig;

typedef struct { uint16 upperBound; uint16 lowerBound; IfxAdc_Tmadc_TriggerConfig *trigger; IfxAdc_Tmadc_BoundaryFlagCfg flagCfg; } IfxAdc_Tmadc_BoundaryConfig;

typedef struct { IfxAdc_TmadcServReq errorNode; IfxAdc_TmadcServReq boundaryNode; IfxAdc_TmadcServReq resultNode; } IfxAdc_Tmadc_ChannelServReqConfig;

typedef struct { IfxDma_Dma_Channel channel; IfxDma_Index dmaId; boolean useDma; } IfxAdc_Tmadc_Dma;

typedef struct { IfxAdc_Tmadc_DmaSrvReq *dmaSrvReqCfg; IfxDma_ChannelId channelId; IfxDma_Index dmaId; } IfxAdc_Tmadc_DmaConfig;

typedef struct { IFX_CONST IfxAdc_Emuxctrl_Out *emuxCtrl0Pin; IfxPort_OutputMode ctrl0PinMode; IFX_CONST IfxAdc_Emuxctrl_Out *emuxCtrl1Pin; IfxPort_OutputMode ctrl1PinMode; IfxPort_PadDriver pinDriver; } IfxAdc_Tmadc_EmuxPinConfig;

typedef struct { IfxAdc_TmadcServReq node; IfxAdc_TmadcEventOp eventOp; IfxAdc_TmadcEventSel eventSel; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; Ifx_Priority priority; } IfxAdc_Tmadc_InterruptConfig;

typedef struct { IFX_CONST IfxAdc_TmadcCh_In *tmadcInPin; IfxPort_InputMode tmadcPinMode; IfxPort_PadDriver pinDriver; } IfxAdc_Tmadc_ChannelPinConfig;

typedef struct { IfxAdc_Tmadc_HwTriggerConfig hwTrigger1; IfxAdc_Tmadc_HwTriggerConfig hwTrigger2; float32 delayNS; } IfxAdc_Tmadc_ChannelTriggerConfig;

typedef struct {
    uint8 count; boolean emuxCi0; boolean emuxCi1; uint8 exmuxCodeScheme; IfxAdc_Tmadc_EmuxPinConfig *emuxPins;
} IfxAdc_Tmadc_EmuxConfig;

typedef struct { IfxAdc_TmadcGlobalServReq servReq; boolean enableEvent; boolean eventLogic; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; Ifx_Priority priority; } IfxAdc_Tmadc_GlobalServRequestConfig;

typedef struct {
    uint16 channels; IfxAdc_TmadcResultReg baseResultReg; IfxAdc_Tmadc_ChannelTriggerConfig *trigger; IfxAdc_TmadcOpMode mode; boolean waitForRead; boolean enableTimestamp; uint8 numChannels; IfxAdc_TmadcModule moduleId; IfxAdc_Tmadc_ChannelServReqConfig *grpSrvReq; IfxAdc_Tmadc_DmaConfig *dmaCfg; void *groupResPtr; IfxAdc_Tmadc_Callback groupCallback;
} IfxAdc_Tmadc_GroupConfig;

typedef struct { IfxAdc_Tmadc_BoundaryConfig *boundCfg1; IfxAdc_Tmadc_BoundaryConfig *boundCfg2; } IfxAdc_Tmadc_ModuleBoundConfig;

typedef struct { boolean waitForRead; boolean boundMode; IfxAdc_TmadcBoundaryCmpMode boundCmpMode; IfxAdc_TmadcBoundaryReg boundRegSel; } IfxAdc_Tmadc_MonitorChResultConfig;

typedef struct {
    uint32 *bufferPtr; IfxAdc_Tmadc_Callback buffFullCallback; IfxAdc_Tmadc_Dma dma; IfxAdc_Tmadc_BufferType bufferType; uint16 writeIndex; uint16 readIndex; uint16 size; uint16 validResult; boolean bufferFull; boolean queueEnabled;
} IfxAdc_Tmadc_Queue;

typedef struct { IfxAdc_Tmadc_DmaConfig *dmaCfg; void *bufferPtr; IfxAdc_Tmadc_Callback bufferFullCallback; uint16 size; IfxAdc_Tmadc_BufferType bufferType; } IfxAdc_Tmadc_QueueConfig;

typedef struct {
    IfxAdc_TmadcResultReg       resultReg; boolean waitForRead; boolean hysteresisEn; boolean boundMode; boolean enableTimestamp; IfxAdc_TmadcBoundaryCmpMode boundCmpMode; IfxAdc_TmadcBoundaryReg boundRegSel;
} IfxAdc_Tmadc_ResultConfig;

typedef struct { IfxAdc_Tmadc_InterruptConfig *intConfig[IFXADC_TMADC_MAX_SERV_REQ_NODE]; uint8 numServReqNodes; } IfxAdc_Tmadc_ServRequestConfig;

typedef struct { IfxAdc_TmadcModule id; Ifx_ADC_TMADC *modSFR; IfxAdc_Tmadc_moduleState state; boolean calEnabled; } IfxAdc_Tmadc;

typedef struct {
    Ifx_ADC_TMADC      *modSFR; Ifx_ADC_TMADC_CH *chSFR; volatile Ifx_ADC_RES *rsltSFR; volatile Ifx_ADC_TS *tmstmpSFR; IfxAdc_TmadcResultReg resultRegNum; IfxAdc_TmadcChannel id; IfxAdc_Tmadc_channelState state; IfxAdc_TmadcBoundaryReg boundaryReg; IfxAdc_TmadcModule moduleId; struct _IfxAdc_Tmadc_Queue { uint32 *dummy; } queue; boolean timeStampEnabled;
} IfxAdc_Tmadc_Ch;

typedef struct {
    IfxAdc_TmadcChannel id; IfxAdc_TmadcModule moduleId; Ifx_ADC *adcSFR; float32 samplingTimeNS; IfxAdc_TmadcOpMode mode; IfxAdc_TmadcSarCore core; boolean enableEmux; IfxAdc_Tmadc_ResultConfig resultCfg; IfxAdc_Tmadc_ChannelTriggerConfig *trigger; IfxAdc_Tmadc_ChannelServReqConfig *channelSrvReq; IfxAdc_Tmadc_ChannelPinConfig *channelPin; IfxAdc_Tmadc_QueueConfig *queueCfg; IfxAdc_Tmadc_GroupConfig *groupCfg;
} IfxAdc_Tmadc_ChConfig;

typedef struct {
    IfxAdc_TmadcModule                    id; Ifx_ADC *adcSFR; boolean shadowBnd0Update; boolean shadowBnd1Update; boolean calEnable; uint8 outputSignalSel; IfxAdc_Tmadc_EmuxConfig *emuxCfg; IfxAdc_Tmadc_ModuleBoundConfig *bndryConfig; IfxAdc_Tmadc_GlobalServRequestConfig *glbsrvReqCfg; IfxAdc_Tmadc_ServRequestConfig *srvReqCfg;
} IfxAdc_Tmadc_Config;

typedef struct {
    uint8 numChannels; uint16 channelset; IfxAdc_TmadcResultReg baseResultReg; Ifx_ADC_TMADC *tmSFR; uint32 *sourceAddress; uint32 *groupResPtr; IfxAdc_Tmadc_Callback groupCallback; IfxAdc_Tmadc_Dma dma; boolean timestampEnabled;
} IfxAdc_Tmadc_Group;

typedef struct { IfxAdc_TmadcMonitorChannel id; IfxAdc_TmadcModule moduleId; Ifx_ADC_TMADC *modSFR; void *chSFR; volatile Ifx_ADC_TMADC_MRES *rsltSFR; } IfxAdc_Tmadc_MonitorCh;

typedef struct { IfxAdc_TmadcMonitorChannel id; IfxAdc_TmadcModule moduleId; Ifx_ADC *adcSFR; float32 samplingTimeNS; IfxAdc_TmadcOpMode mode; IfxAdc_TmadcMonitorChannelInput input; boolean enableChannel; IfxAdc_Tmadc_MonitorChResultConfig resultCfg; IfxAdc_Tmadc_ChannelServReqConfig channelSrvReq; } IfxAdc_Tmadc_MonitorChannelConfig;

typedef struct { uint16 timeStamp; uint16 result; } IfxAdc_Tmadc_Result;

/* Minimal API subset used by production */
void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc);

#endif /* IFXADC_TMADC_H */
