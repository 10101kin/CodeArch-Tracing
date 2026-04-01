/* IfxAdc_Tmadc.h */
#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxAdc.h"
#include "IfxPort.h"

#ifndef IFX_CONST
#define IFX_CONST const
#endif
#ifndef IFXADC_TMADC_MAX_SERV_REQ_NODE
#define IFXADC_TMADC_MAX_SERV_REQ_NODE (7)
#endif

/* Minimal pin-type placeholders */
typedef struct { uint32 id; } IfxAdc_Emuxctrl_Out;
typedef struct { uint32 id; } IfxAdc_TmadcCh_In;

/* SFR placeholders */
typedef struct { uint32 r; } Ifx_ADC_TMADC;
typedef struct { uint32 r; } Ifx_ADC_TMADC_CH;
typedef struct { uint32 r; } Ifx_ADC_RES;
typedef struct { uint32 r; } Ifx_ADC_TS;
typedef struct { uint32 r; } Ifx_ADC_TMADC_MCH;
typedef struct { uint32 r; } Ifx_ADC_TMADC_MRES;

/* Callback */
typedef void (*IfxAdc_Tmadc_Callback)(void* context);

/* typeName struct (per spec) */
typedef struct { uint32 timeStamp; uint32 results; } typeName;

/* Config blocks */
typedef struct {
    IfxAdc_TmadcTriggerMuxSel muxSel;
    IfxAdc_TmadcTriggerMode   edgeSel;
} IfxAdc_Tmadc_HwTriggerConfig;

typedef struct {
    IfxAdc_TmadcResultReg resultRegSel;
    boolean               enable;
} IfxAdc_Tmadc_BoundaryFlagCfg;

/* DMA placeholders */
typedef uint32 IfxDma_Dma_Channel;
typedef uint32 IfxDma_Index;
typedef uint32 IfxDma_ChannelId;

typedef struct {
    Ifx_Priority priority;
    IfxSrc_Tos   typeOfService;
    IfxSrc_VmId  vmId;
} IfxAdc_Tmadc_DmaSrvReq;

typedef struct {
    IfxAdc_Tmadc_HwTriggerConfig hwTrigger;
    float32                      delayNS;
} IfxAdc_Tmadc_TriggerConfig;

typedef struct {
    uint16                       upperBound;
    uint16                       lowerBound;
    IfxAdc_Tmadc_TriggerConfig  *trigger;
    IfxAdc_Tmadc_BoundaryFlagCfg flagCfg;
} IfxAdc_Tmadc_BoundaryConfig;

typedef struct {
    IfxAdc_TmadcServReq errorNode;
    IfxAdc_TmadcServReq boundaryNode;
    IfxAdc_TmadcServReq resultNode;
} IfxAdc_Tmadc_ChannelServReqConfig;

typedef struct {
    IfxAdc_Tmadc_DmaSrvReq *dmaSrvReqCfg;
    IfxDma_ChannelId        channelId;
    IfxDma_Index            dmaId;
} IfxAdc_Tmadc_DmaConfig;

typedef struct {
    IFX_CONST IfxAdc_Emuxctrl_Out *emuxCtrl0Pin;
    IfxPort_OutputMode             ctrl0PinMode;
    IFX_CONST IfxAdc_Emuxctrl_Out *emuxCtrl1Pin;
    IfxPort_OutputMode             ctrl1PinMode;
    IfxPort_PadDriver              pinDriver;
} IfxAdc_Tmadc_EmuxPinConfig;

typedef struct {
    IfxAdc_TmadcServReq  node;
    IfxAdc_TmadcEventOp  eventOp;
    IfxAdc_TmadcEventSel eventSel;
    IfxSrc_Tos           typeOfService;
    IfxSrc_VmId          vmId;
    Ifx_Priority         priority;
} IfxAdc_Tmadc_InterruptConfig;

typedef struct {
    IFX_CONST IfxAdc_TmadcCh_In *tmadcInPin;
    IfxPort_InputMode            tmadcPinMode;
    IfxPort_PadDriver            pinDriver;
} IfxAdc_Tmadc_ChannelPinConfig;

typedef struct {
    IfxAdc_Tmadc_HwTriggerConfig hwTrigger1;
    IfxAdc_Tmadc_HwTriggerConfig hwTrigger2;
    float32                      delayNS;
} IfxAdc_Tmadc_ChannelTriggerConfig;

typedef struct {
    uint8                        count;
    boolean                      emuxCi0;
    boolean                      emuxCi1;
    IfxAdc_TmadcEmuxCodingScheme exmuxCodeScheme;
    IfxAdc_Tmadc_EmuxPinConfig  *emuxPins;
} IfxAdc_Tmadc_EmuxConfig;

typedef struct {
    IfxAdc_TmadcGlobalServReq servReq;
    boolean                   enableEvent;
    boolean                   eventLogic;
    IfxSrc_Tos                typeOfService;
    IfxSrc_VmId               vmId;
    Ifx_Priority              priority;
} IfxAdc_Tmadc_GlobalServRequestConfig;

typedef struct {
    uint16                             channels;
    IfxAdc_TmadcResultReg              baseResultReg;
    IfxAdc_Tmadc_ChannelTriggerConfig *trigger;
    IfxAdc_TmadcOpMode                 mode;
    boolean                            waitForRead;
    boolean                            enableTimestamp;
    uint8                              numChannels;
    uint8                              moduleId; /* IfxAdc_TmadcModule placeholder */
    IfxAdc_Tmadc_ChannelServReqConfig *grpSrvReq;
    IfxAdc_Tmadc_DmaConfig            *dmaCfg;
    void                              *groupResPtr;
    IfxAdc_Tmadc_Callback              groupCallback;
} IfxAdc_Tmadc_GroupConfig;

typedef struct {
    IfxAdc_Tmadc_BoundaryConfig *boundCfg1;
    IfxAdc_Tmadc_BoundaryConfig *boundCfg2;
} IfxAdc_Tmadc_ModuleBoundConfig;

typedef struct {
    boolean                     waitForRead;
    boolean                     boundMode;
    IfxAdc_TmadcBoundaryCmpMode boundCmpMode;
    IfxAdc_TmadcBoundaryReg     boundRegSel;
} IfxAdc_Tmadc_MonitorChResultConfig;

typedef enum {
    IfxAdc_Tmadc_BufferType_linear = 0,
    IfxAdc_Tmadc_BufferType_linearWithTimestamp = 1,
    IfxAdc_Tmadc_BufferType_circular = 2
} IfxAdc_Tmadc_BufferType;

typedef struct {
    uint32                 *bufferPtr;
    IfxAdc_Tmadc_Callback   buffFullCallback;
    struct { IfxDma_Dma_Channel channel; IfxDma_Index dmaId; boolean useDma; } dma; /* Inline DMA handle */
    IfxAdc_Tmadc_BufferType bufferType;
    uint16                  writeIndex;
    uint16                  readIndex;
    uint16                  size;
    uint16                  validResult;
    boolean                 bufferFull;
    boolean                 queueEnabled;
} IfxAdc_Tmadc_Queue;

typedef struct {
    IfxAdc_Tmadc_DmaConfig *dmaCfg;
    void                   *bufferPtr;
    IfxAdc_Tmadc_Callback   bufferFullCallback;
    uint16                  size;
    IfxAdc_Tmadc_BufferType bufferType;
} IfxAdc_Tmadc_QueueConfig;

typedef struct {
    IfxAdc_TmadcResultReg       resultReg;
    boolean                     waitForRead;
    boolean                     hysteresisEn;
    boolean                     boundMode;
    boolean                     enableTimestamp;
    IfxAdc_TmadcBoundaryCmpMode boundCmpMode;
    IfxAdc_TmadcBoundaryReg     boundRegSel;
} IfxAdc_Tmadc_ResultConfig;

typedef struct {
    IfxAdc_Tmadc_InterruptConfig *intConfig[IFXADC_TMADC_MAX_SERV_REQ_NODE];
    uint8                         numServReqNodes;
} IfxAdc_Tmadc_ServRequestConfig;

typedef struct {
    uint8                 id;      /* module ID */
    Ifx_ADC_TMADC        *modSFR;  /* pointer to module SFR */
    uint8                 state;   /* module state */
    boolean               calEnabled;
} IfxAdc_Tmadc;

typedef struct {
    Ifx_ADC_TMADC    *modSFR;
    Ifx_ADC_TMADC_CH *chSFR;
    volatile Ifx_ADC_RES *rsltSFR;
    volatile Ifx_ADC_TS  *tmstmpSFR;
    IfxAdc_TmadcResultReg resultRegNum;
    uint8                  id; /* channel id */
    uint8                  state; /* channel state */
    IfxAdc_TmadcBoundaryReg boundaryReg;
    uint8                  moduleId;
    IfxAdc_Tmadc_Queue    queue;
    boolean               timeStampEnabled;
} IfxAdc_Tmadc_Ch;

typedef struct {
    uint8                              id;
    uint8                              moduleId;
    Ifx_ADC                           *adcSFR;
    float32                            samplingTimeNS;
    IfxAdc_TmadcOpMode                 mode;
    IfxAdc_TmadcSarCore                core;
    boolean                            enableEmux;
    IfxAdc_Tmadc_ResultConfig          resultCfg;
    IfxAdc_Tmadc_ChannelTriggerConfig *trigger;
    IfxAdc_Tmadc_ChannelServReqConfig *channelSrvReq;
    IfxAdc_Tmadc_ChannelPinConfig     *channelPin;
    IfxAdc_Tmadc_QueueConfig          *queueCfg;
    IfxAdc_Tmadc_GroupConfig          *groupCfg;
} IfxAdc_Tmadc_ChConfig;

typedef struct {
    uint8                                id;
    Ifx_ADC                             *adcSFR;
    boolean                              shadowBnd0Update;
    boolean                              shadowBnd1Update;
    boolean                              calEnable;
    IfxAdc_TmadcOutputSupervisorMux      outputSignalSel;
    IfxAdc_Tmadc_EmuxConfig             *emuxCfg;
    IfxAdc_Tmadc_ModuleBoundConfig      *bndryConfig;
    IfxAdc_Tmadc_GlobalServRequestConfig*glbsrvReqCfg;
    IfxAdc_Tmadc_ServRequestConfig      *srvReqCfg;
} IfxAdc_Tmadc_Config;

typedef struct {
    uint8           numChannels;
    uint16          channelset;
    IfxAdc_TmadcResultReg baseResultReg;
    Ifx_ADC_TMADC  *tmSFR;
    uint32         *sourceAddress;
    uint32         *groupResPtr;
    IfxAdc_Tmadc_Callback groupCallback;
    struct { IfxDma_Dma_Channel channel; IfxDma_Index dmaId; boolean useDma; } dma;
    boolean         timestampEnabled;
} IfxAdc_Tmadc_Group;

typedef struct {
    IfxAdc_TmadcMonitorChannel         id;
    uint8                               moduleId;
    Ifx_ADC_TMADC                      *modSFR;
    Ifx_ADC_TMADC_MCH                  *chSFR;
    volatile Ifx_ADC_TMADC_MRES        *rsltSFR;
} IfxAdc_Tmadc_MonitorCh;

typedef struct {
    IfxAdc_TmadcMonitorChannel         id;
    uint8                               moduleId;
    Ifx_ADC                            *adcSFR;
    float32                             samplingTimeNS;
    IfxAdc_TmadcOpMode                  mode;
    IfxAdc_TmadcMonitorChannelInput     input;
    boolean                             enableChannel;
    IfxAdc_Tmadc_MonitorChResultConfig  resultCfg;
    IfxAdc_Tmadc_ChannelServReqConfig   channelSrvReq;
} IfxAdc_Tmadc_MonitorChannelConfig;

typedef struct {
    uint16 timeStamp;
    uint16 result;
} IfxAdc_Tmadc_Result;

/* API to mock */
void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config);
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc);

#endif /* IFXADC_TMADC_H */
