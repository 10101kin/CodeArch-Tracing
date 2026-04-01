#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxAdc.h"

/* Basic SFR placeholders used by struct pointers */
typedef struct { uint32 r; } Ifx_ADC_TMADC;
typedef struct { uint32 r; } Ifx_ADC_TMADC_CH;
typedef struct { uint32 r; } Ifx_ADC_RES;
typedef struct { uint32 r; } Ifx_ADC_TS;
typedef struct { uint32 r; } Ifx_ADC_TMADC_MCH;
typedef struct { uint32 r; } Ifx_ADC_TMADC_MRES;

/* Additional IDs used by TMADC (minimal) */
typedef enum { IfxAdc_TmadcModule_0 = 0 } IfxAdc_TmadcModule;
typedef enum { IfxAdc_TmadcChannel_0 = 0 } IfxAdc_TmadcChannel;

typedef void (*IfxAdc_Tmadc_Callback)(void);

typedef struct { uint32 timeStamp; uint32 results; } typeName;

typedef struct { IfxAdc_TmadcTriggerMuxSel muxSel; IfxAdc_TmadcTriggerMode edgeSel; } IfxAdc_Tmadc_HwTriggerConfig;

typedef struct { IfxAdc_TmadcResultReg resultRegSel; boolean enable; } IfxAdc_Tmadc_BoundaryFlagCfg;

typedef struct { Ifx_Priority priority; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; } IfxAdc_Tmadc_DmaSrvReq;

typedef struct { IfxAdc_Tmadc_HwTriggerConfig hwTrigger; float32 delayNS; } IfxAdc_Tmadc_TriggerConfig;

typedef struct { uint16 upperBound; uint16 lowerBound; IfxAdc_Tmadc_TriggerConfig *trigger; IfxAdc_Tmadc_BoundaryFlagCfg flagCfg; } IfxAdc_Tmadc_BoundaryConfig;

typedef struct { IfxAdc_TmadcServReq errorNode; IfxAdc_TmadcServReq boundaryNode; IfxAdc_TmadcServReq resultNode; } IfxAdc_Tmadc_ChannelServReqConfig;

/* DMA placeholders */
typedef struct { uint32 dummy; } IfxDma_Dma_Channel; 
typedef uint8 IfxDma_Index; 
typedef uint8 IfxDma_ChannelId;

typedef struct { IfxAdc_Tmadc_DmaSrvReq *dmaSrvReqCfg; IfxDma_ChannelId channelId; IfxDma_Index dmaId; } IfxAdc_Tmadc_DmaConfig;

typedef struct { IfxDma_Dma_Channel channel; IfxDma_Index dmaId; boolean useDma; } IfxAdc_Tmadc_Dma;

typedef struct { const void *emuxCtrl0Pin; IfxPort_OutputMode ctrl0PinMode; const void *emuxCtrl1Pin; IfxPort_OutputMode ctrl1PinMode; IfxPort_PadDriver pinDriver; } IfxAdc_Tmadc_EmuxPinConfig;

typedef struct { IfxAdc_TmadcServReq node; IfxAdc_TmadcEventOp eventOp; IfxAdc_TmadcEventSel eventSel; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; Ifx_Priority priority; } IfxAdc_Tmadc_InterruptConfig;

typedef struct { const void *tmadcInPin; IfxPort_InputMode tmadcPinMode; IfxPort_PadDriver pinDriver; } IfxAdc_Tmadc_ChannelPinConfig;

typedef struct { IfxAdc_Tmadc_HwTriggerConfig hwTrigger1; IfxAdc_Tmadc_HwTriggerConfig hwTrigger2; float32 delayNS; } IfxAdc_Tmadc_ChannelTriggerConfig;

typedef struct {
    uint8 count; boolean emuxCi0; boolean emuxCi1; IfxAdc_TmadcEmuxCodingScheme exmuxCodeScheme; IfxAdc_Tmadc_EmuxPinConfig *emuxPins;
} IfxAdc_Tmadc_EmuxConfig;

typedef struct { IfxAdc_TmadcGlobalServReq servReq; boolean enableEvent; boolean eventLogic; IfxSrc_Tos typeOfService; IfxSrc_VmId vmId; Ifx_Priority priority; } IfxAdc_Tmadc_GlobalServRequestConfig;

typedef struct {
    uint16 channels; IfxAdc_TmadcResultReg baseResultReg; IfxAdc_Tmadc_ChannelTriggerConfig *trigger; IfxAdc_TmadcOpMode mode; boolean waitForRead; boolean enableTimestamp; uint8 numChannels; IfxAdc_TmadcModule moduleId; IfxAdc_Tmadc_ChannelServReqConfig *grpSrvReq; IfxAdc_Tmadc_DmaConfig *dmaCfg; void *groupResPtr; IfxAdc_Tmadc_Callback groupCallback;
} IfxAdc_Tmadc_GroupConfig;

typedef struct { IfxAdc_Tmadc_BoundaryConfig *boundCfg1; IfxAdc_Tmadc_BoundaryConfig *boundCfg2; } IfxAdc_Tmadc_ModuleBoundConfig;

typedef struct { boolean waitForRead; boolean boundMode; IfxAdc_TmadcBoundaryCmpMode boundCmpMode; IfxAdc_TmadcBoundaryReg boundRegSel; } IfxAdc_Tmadc_MonitorChResultConfig;

typedef enum { IfxAdc_Tmadc_BufferType_linear = 0, IfxAdc_Tmadc_BufferType_linearWithTimestamp = 1, IfxAdc_Tmadc_BufferType_circular = 2 } IfxAdc_Tmadc_BufferType;

typedef struct { uint32 *bufferPtr; IfxAdc_Tmadc_Callback buffFullCallback; IfxAdc_Tmadc_Dma dma; IfxAdc_Tmadc_BufferType bufferType; uint16 writeIndex; uint16 readIndex; uint16 size; uint16 validResult; boolean bufferFull; boolean queueEnabled; } IfxAdc_Tmadc_Queue;

typedef struct { IfxAdc_Tmadc_DmaConfig *dmaCfg; void *bufferPtr; IfxAdc_Tmadc_Callback bufferFullCallback; uint16 size; IfxAdc_Tmadc_BufferType bufferType; } IfxAdc_Tmadc_QueueConfig;

typedef struct { IfxAdc_TmadcResultReg resultReg; boolean waitForRead; boolean hysteresisEn; boolean boundMode; boolean enableTimestamp; IfxAdc_TmadcBoundaryCmpMode boundCmpMode; IfxAdc_TmadcBoundaryReg boundRegSel; } IfxAdc_Tmadc_ResultConfig;

typedef struct { IfxAdc_Tmadc_InterruptConfig *intConfig[7]; uint8 numServReqNodes; } IfxAdc_Tmadc_ServRequestConfig;

typedef enum { IfxAdc_Tmadc_channelState_uninitialized = 0, IfxAdc_Tmadc_channelState_initialized = 1 } IfxAdc_Tmadc_channelState;

typedef enum { IfxAdc_Tmadc_moduleState_unknown = 0, IfxAdc_Tmadc_moduleState_calibration = 1, IfxAdc_Tmadc_moduleState_calibrationError = 2, IfxAdc_Tmadc_moduleState_initialized = 3, IfxAdc_Tmadc_moduleState_run = 4, IfxAdc_Tmadc_moduleState_notPoweredOn = 5 } IfxAdc_Tmadc_moduleState;

typedef struct { IfxAdc_TmadcModule id; Ifx_ADC_TMADC *modSFR; IfxAdc_Tmadc_moduleState state; boolean calEnabled; } IfxAdc_Tmadc;

typedef struct {
    Ifx_ADC_TMADC *modSFR; Ifx_ADC_TMADC_CH *chSFR; volatile Ifx_ADC_RES *rsltSFR; volatile Ifx_ADC_TS *tmstmpSFR; IfxAdc_TmadcResultReg resultRegNum; IfxAdc_TmadcChannel id; IfxAdc_Tmadc_channelState state; IfxAdc_TmadcBoundaryReg boundaryReg; IfxAdc_TmadcModule moduleId; IfxAdc_Tmadc_Queue queue; boolean timeStampEnabled;
} IfxAdc_Tmadc_Ch;

typedef struct {
    IfxAdc_TmadcChannel id; IfxAdc_TmadcModule moduleId; Ifx_ADC *adcSFR; float32 samplingTimeNS; IfxAdc_TmadcOpMode mode; IfxAdc_TmadcSarCore core; boolean enableEmux; IfxAdc_Tmadc_ResultConfig resultCfg; IfxAdc_Tmadc_ChannelTriggerConfig *trigger; IfxAdc_Tmadc_ChannelServReqConfig *channelSrvReq; IfxAdc_Tmadc_ChannelPinConfig *channelPin; IfxAdc_Tmadc_QueueConfig *queueCfg; IfxAdc_Tmadc_GroupConfig *groupCfg;
} IfxAdc_Tmadc_ChConfig;

typedef struct {
    IfxAdc_TmadcModule                    id;                     
    Ifx_ADC                              *adcSFR;                 
    boolean                               shadowBnd0Update;       
    boolean                               shadowBnd1Update;       
    boolean                               calEnable;              
    IfxAdc_TmadcOutputSupervisorMux       outputSignalSel;        
    IfxAdc_Tmadc_EmuxConfig              *emuxCfg;                
    IfxAdc_Tmadc_ModuleBoundConfig       *bndryConfig;            
    IfxAdc_Tmadc_GlobalServRequestConfig *glbsrvReqCfg;           
    IfxAdc_Tmadc_ServRequestConfig       *srvReqCfg;              
} IfxAdc_Tmadc_Config;

/* API subset used in tests */
void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config);
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc);

#endif
