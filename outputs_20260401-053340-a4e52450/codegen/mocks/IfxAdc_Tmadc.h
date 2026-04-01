#ifndef IFXADC_TMADC_H
#define IFXADC_TMADC_H
#include "mock_egtm_atom_adc_tmadc_multiple_channels.h"
#include "IfxPort.h"
#include "IfxAdc.h"

/* Minimal SFR stub types used */
typedef struct { uint32 reserved; } Ifx_ADC_TMADC;
typedef struct { uint32 reserved; } Ifx_ADC_TMADC_CH;
typedef struct { uint32 reserved; } Ifx_ADC_RES;
typedef struct { uint32 reserved; } Ifx_ADC_TS;
typedef struct { uint32 reserved; } Ifx_ADC_TMADC_MCH;

/* Basic enums needed */
typedef enum { IfxAdc_Tmadc_BufferType_linear = 0, IfxAdc_Tmadc_BufferType_linearWithTimestamp = 1, IfxAdc_Tmadc_BufferType_circular = 2 } IfxAdc_Tmadc_BufferType;

typedef enum { IfxAdc_Tmadc_channelState_uninitialized = 0, IfxAdc_Tmadc_channelState_initialized = 1 } IfxAdc_Tmadc_channelState;

typedef enum { IfxAdc_Tmadc_moduleState_unknown = 0, IfxAdc_Tmadc_moduleState_calibration = 1, IfxAdc_Tmadc_moduleState_calibrationError = 2, IfxAdc_Tmadc_moduleState_initialized = 3, IfxAdc_Tmadc_moduleState_run = 4, IfxAdc_Tmadc_moduleState_notPoweredOn = 5 } IfxAdc_Tmadc_moduleState;

typedef enum { IfxAdc_TmadcModule_0 = 0 } IfxAdc_TmadcModule;
typedef enum { IfxAdc_TmadcChannel_0 = 0 } IfxAdc_TmadcChannel;

/* typeName as specified */
typedef struct { uint32 timeStamp; uint32 results; } typeName;

/* Selected config/result structs minimal (only those referenced in main structs) */
typedef struct { boolean waitForRead; boolean boundMode; uint8 boundCmpMode; uint8 boundRegSel; } IfxAdc_Tmadc_MonitorChResultConfig;

typedef struct { uint32 *bufferPtr; void (*buffFullCallback)(void); struct { uint32 channel; uint32 dmaId; boolean useDma; } dma; IfxAdc_Tmadc_BufferType bufferType; uint16 writeIndex; uint16 readIndex; uint16 size; uint16 validResult; boolean bufferFull; boolean queueEnabled; } IfxAdc_Tmadc_Queue;

/* Main handles */
typedef struct {
    IfxAdc_TmadcModule       id;
    Ifx_ADC_TMADC           *modSFR;
    IfxAdc_Tmadc_moduleState state;
    boolean                  calEnabled;
} IfxAdc_Tmadc;

/* Minimal channel */
typedef struct {
    Ifx_ADC_TMADC    *modSFR;
    Ifx_ADC_TMADC_CH *chSFR;
    volatile Ifx_ADC_RES *rsltSFR;
    volatile Ifx_ADC_TS  *tmstmpSFR;
    IfxAdc_TmadcResultReg resultRegNum;
    IfxAdc_TmadcChannel   id;
    IfxAdc_Tmadc_channelState state;
    uint8                    boundaryReg;
    IfxAdc_TmadcModule      moduleId;
    IfxAdc_Tmadc_Queue      queue;
    boolean                 timeStampEnabled;
} IfxAdc_Tmadc_Ch;

/* Minimal config */
typedef struct {
    IfxAdc_TmadcModule id;
    Ifx_ADC           *adcSFR;
    boolean            shadowBnd0Update;
    boolean            shadowBnd1Update;
    boolean            calEnable;
    uint8              outputSignalSel;
    void              *emuxCfg;
    void              *bndryConfig;
    void              *glbsrvReqCfg;
    void              *srvReqCfg;
} IfxAdc_Tmadc_Config;

/* Group and monitor channel minimal for completeness */
typedef struct {
    uint8                 numChannels;
    uint16                channelset;
    IfxAdc_TmadcResultReg baseResultReg;
    Ifx_ADC_TMADC        *tmSFR;
    uint32               *sourceAddress;
    uint32               *groupResPtr;
    void                 *groupCallback;
    struct { uint32 channel; uint32 dmaId; boolean useDma; } dma;
    boolean               timestampEnabled;
} IfxAdc_Tmadc_Group;

typedef struct {
    uint8                   id;
    IfxAdc_TmadcModule      moduleId;
    Ifx_ADC                *adcSFR;
    float32                 samplingTimeNS;
    uint8                   mode;
    uint8                   input;
    boolean                 enableChannel;
    IfxAdc_Tmadc_MonitorChResultConfig resultCfg;
    struct { uint8 errorNode; uint8 boundaryNode; uint8 resultNode; } channelSrvReq;
} IfxAdc_Tmadc_MonitorChannelConfig;

/* Function used by production */
void IfxAdc_Tmadc_runModule(IfxAdc_Tmadc *tmadc);

#endif /* IFXADC_TMADC_H */
