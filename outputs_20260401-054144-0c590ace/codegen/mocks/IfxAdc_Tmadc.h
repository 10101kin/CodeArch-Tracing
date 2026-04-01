/* IfxAdc_Tmadc mock */
#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H
#include "mock_egtm_atom_tmadc_consolidated.h"
#include "IfxPort.h"

/* Tmadc-specific enums (single-owner here) */
typedef enum {
    IfxAdc_Tmadc_BufferType_linear = 0,
    IfxAdc_Tmadc_BufferType_linearWithTimestamp = 1,
    IfxAdc_Tmadc_BufferType_circular = 2
} IfxAdc_Tmadc_BufferType;

typedef enum {
    IfxAdc_Tmadc_channelState_uninitialized = 0,
    IfxAdc_Tmadc_channelState_initialized = 1
} IfxAdc_Tmadc_channelState;

typedef enum {
    IfxAdc_Tmadc_moduleState_unknown = 0,
    IfxAdc_Tmadc_moduleState_calibration = 1,
    IfxAdc_Tmadc_moduleState_calibrationError = 2,
    IfxAdc_Tmadc_moduleState_initialized = 3,
    IfxAdc_Tmadc_moduleState_run = 4,
    IfxAdc_Tmadc_moduleState_notPoweredOn = 5
} IfxAdc_Tmadc_moduleState;

/* A minimal subset of TMADC-related enums used in configs */
typedef enum { IfxAdc_TmadcResultReg_0 = 0, IfxAdc_TmadcResultReg_1, IfxAdc_TmadcResultReg_2, IfxAdc_TmadcResultReg_3, IfxAdc_TmadcResultReg_4, IfxAdc_TmadcResultReg_5, IfxAdc_TmadcResultReg_6, IfxAdc_TmadcResultReg_7, IfxAdc_TmadcResultReg_8, IfxAdc_TmadcResultReg_9, IfxAdc_TmadcResultReg_10, IfxAdc_TmadcResultReg_11, IfxAdc_TmadcResultReg_12, IfxAdc_TmadcResultReg_13, IfxAdc_TmadcResultReg_14, IfxAdc_TmadcResultReg_15 } IfxAdc_TmadcResultReg;

typedef enum { IfxAdc_TmadcBoundaryCmpMode_disable = 0, IfxAdc_TmadcBoundaryCmpMode_upperBound = 1, IfxAdc_TmadcBoundaryCmpMode_lowerBound = 2, IfxAdc_TmadcBoundaryCmpMode_bothBound = 3 } IfxAdc_TmadcBoundaryCmpMode;

typedef enum { IfxAdc_TmadcBoundaryReg_0 = 0, IfxAdc_TmadcBoundaryReg_1 = 1 } IfxAdc_TmadcBoundaryReg;

typedef enum { IfxAdc_TmadcTriggerMode_disable = 0, IfxAdc_TmadcTriggerMode_risingEdge = 1, IfxAdc_TmadcTriggerMode_fallingEdge = 2, IfxAdc_TmadcTriggerMode_bothEdge = 3 } IfxAdc_TmadcTriggerMode;

typedef enum { IfxAdc_TmadcTriggerMuxSel_0 = 0 } IfxAdc_TmadcTriggerMuxSel; /* compressed for mock */

typedef enum { IfxAdc_TmadcOpMode_oneShot = 0, IfxAdc_TmadcOpMode_continuous = 1 } IfxAdc_TmadcOpMode;

typedef enum { IfxAdc_TmadcGlobalServReq_0 = 0, IfxAdc_TmadcGlobalServReq_1 = 1, IfxAdc_TmadcGlobalServReq_2 = 2 } IfxAdc_TmadcGlobalServReq;

typedef enum { IfxAdc_TmadcServReq_none = -1, IfxAdc_TmadcServReq_0 = 0, IfxAdc_TmadcServReq_1 = 1, IfxAdc_TmadcServReq_2 = 2, IfxAdc_TmadcServReq_3 = 3, IfxAdc_TmadcServReq_4 = 4, IfxAdc_TmadcServReq_5 = 5 } IfxAdc_TmadcServReq;

typedef enum { IfxAdc_TmadcMonitorChannel_0 = 0, IfxAdc_TmadcMonitorChannel_1 = 1 } IfxAdc_TmadcMonitorChannel;

typedef enum { IfxAdc_TmadcMonitorChannelInput_coreSupply = 0, IfxAdc_TmadcMonitorChannelInput_otherModules = 1, IfxAdc_TmadcMonitorChannelInput_vssm = 2, IfxAdc_TmadcMonitorChannelInput_csd = 3 } IfxAdc_TmadcMonitorChannelInput;

typedef enum { IfxAdc_TmadcEmuxCodingScheme_binary = 0, IfxAdc_TmadcEmuxCodingScheme_gray = 1 } IfxAdc_TmadcEmuxCodingScheme;

typedef enum { IfxAdc_TmadcOutputSupervisorMux_noConnection = 0, IfxAdc_TmadcOutputSupervisorMux_vddk0 = 1, IfxAdc_TmadcOutputSupervisorMux_vddk1 = 2, IfxAdc_TmadcOutputSupervisorMux_vssm = 3 } IfxAdc_TmadcOutputSupervisorMux;

/* Basic TMADC structs (subset adequate for init/usage) */

typedef struct {
    uint32 timeStamp;
    uint32 results;
} typeName;

typedef struct {
    IfxAdc_TmadcTriggerMuxSel muxSel;
    IfxAdc_TmadcTriggerMode   edgeSel;
} IfxAdc_Tmadc_HwTriggerConfig;

typedef struct {
    IfxAdc_TmadcResultReg resultRegSel;
    boolean               enable;
} IfxAdc_Tmadc_BoundaryFlagCfg;

typedef struct {
    IfxAdc_Tmadc_HwTriggerConfig hwTrigger;
    float32                      delayNS;
} IfxAdc_Tmadc_TriggerConfig;

typedef struct {
    uint16                       upperBound;
    uint16                       lowerBound;
    IfxAdc_Tmadc_TriggerConfig *trigger;
    IfxAdc_Tmadc_BoundaryFlagCfg flagCfg;
} IfxAdc_Tmadc_BoundaryConfig;

typedef struct {
    IfxAdc_TmadcServReq errorNode;
    IfxAdc_TmadcServReq boundaryNode;
    IfxAdc_TmadcServReq resultNode;
} IfxAdc_Tmadc_ChannelServReqConfig;

typedef struct {
    uint32 *bufferPtr;
    void   *buffFullCallback; /* simplified */
    struct { uint32 dummy; } dma; /* simplified */
    IfxAdc_Tmadc_BufferType bufferType;
    uint16  writeIndex;
    uint16  readIndex;
    uint16  size;
    uint16  validResult;
    boolean bufferFull;
    boolean queueEnabled;
} IfxAdc_Tmadc_Queue;

typedef struct {
    void    *dmaSrvReqCfg;
    uint32   channelId;
    uint32   dmaId;
} IfxAdc_Tmadc_DmaConfig;

typedef struct {
    const void         *emuxCtrl0Pin;
    IfxPort_OutputMode  ctrl0PinMode;
    const void         *emuxCtrl1Pin;
    IfxPort_OutputMode  ctrl1PinMode;
    IfxPort_PadDriver   pinDriver;
} IfxAdc_Tmadc_EmuxPinConfig;

typedef struct {
    IfxAdc_TmadcServReq  node;
    uint32               eventOp;
    uint32               eventSel;
    IfxSrc_Tos           typeOfService;
    IfxSrc_VmId          vmId;
    Ifx_Priority         priority;
} IfxAdc_Tmadc_InterruptConfig;

typedef struct {
    const void        *tmadcInPin;
    IfxPort_InputMode  tmadcPinMode;
    IfxPort_PadDriver  pinDriver;
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
    uint8                              moduleId; /* simplified */
    IfxAdc_Tmadc_ChannelServReqConfig *grpSrvReq;
    IfxAdc_Tmadc_DmaConfig            *dmaCfg;
    void                              *groupResPtr;
    void                              *groupCallback;
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
    IfxAdc_Tmadc_InterruptConfig *intConfig[8];
    uint8                          numServReqNodes;
} IfxAdc_Tmadc_ServRequestConfig;

typedef struct {
    uint8                  id;               
    void                  *modSFR;           
    IfxAdc_Tmadc_moduleState state;          
    boolean                calEnabled;       
} IfxAdc_Tmadc;

/* Minimal channel/group structs */
typedef struct {
    void                      *modSFR;
    void                      *chSFR;
    volatile void             *rsltSFR;
    volatile void             *tmstmpSFR;
    IfxAdc_TmadcResultReg      resultRegNum;
    uint8                       id;
    IfxAdc_Tmadc_channelState   state;
    IfxAdc_TmadcBoundaryReg     boundaryReg;
    uint8                        moduleId;
    IfxAdc_Tmadc_Queue          queue;
    boolean                      timeStampEnabled;
} IfxAdc_Tmadc_Ch;

typedef struct {
    uint8                              id;
    uint8                              moduleId;
    void                              *adcSFR;
    float32                            samplingTimeNS;
    IfxAdc_TmadcOpMode                 mode;
    uint8                              core;
    boolean                            enableEmux;
    IfxAdc_Tmadc_ResultConfig          resultCfg;
    IfxAdc_Tmadc_ChannelTriggerConfig *trigger;
    IfxAdc_Tmadc_ChannelServReqConfig *channelSrvReq;
    IfxAdc_Tmadc_ChannelPinConfig     *channelPin;
    IfxAdc_Tmadc_Queue               *queueCfg;
    IfxAdc_Tmadc_GroupConfig         *groupCfg;
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
    IfxAdc_Tmadc_GlobalServRequestConfig *glbsrvReqCfg;
    IfxAdc_Tmadc_ServRequestConfig      *srvReqCfg;
} IfxAdc_Tmadc_Config;

typedef struct {
    uint8   numChannels;
    uint16  channelset;
    IfxAdc_TmadcResultReg baseResultReg;
    void   *tmSFR;
    uint32 *sourceAddress;
    uint32 *groupResPtr;
    void   *groupCallback;
    struct { uint32 dummy; } dma;
    boolean timestampEnabled;
} IfxAdc_Tmadc_Group;

typedef struct {
    IfxAdc_TmadcMonitorChannel      id;
    uint8                           moduleId;
    void                           *modSFR;
    void                           *chSFR;
    volatile void                  *rsltSFR;
} IfxAdc_Tmadc_MonitorCh;

typedef struct {
    IfxAdc_TmadcMonitorChannel      id;
    uint8                           moduleId;
    Ifx_ADC                        *adcSFR;
    float32                         samplingTimeNS;
    IfxAdc_TmadcOpMode              mode;
    IfxAdc_TmadcMonitorChannelInput input;
    boolean                         enableChannel;
    IfxAdc_Tmadc_MonitorChResultConfig resultCfg;
    IfxAdc_Tmadc_ChannelServReqConfig  channelSrvReq;
} IfxAdc_Tmadc_MonitorChannelConfig;

typedef struct {
    uint16 timeStamp;
    uint16 result;
} IfxAdc_Tmadc_Result;

/* Functions used by the module */
void IfxAdc_Tmadc_initModule(IfxAdc_Tmadc *tmadc, const IfxAdc_Tmadc_Config *config);
void IfxAdc_Tmadc_initModuleConfig(IfxAdc_Tmadc_Config *config, Ifx_ADC *adc);

#endif /* IFXADC_TMADC_H */
