#ifndef IFXPORT_H
#define IFXPORT_H


typedef enum {
    IfxPort_ControlledBy_port,
    IfxPort_ControlledBy_hsct
} IfxPort_ControlledBy;

typedef enum {
    IfxPort_InputMode_undefined,
    IfxPort_InputMode_noPullDevice,
    IfxPort_InputMode_pullDown,
    IfxPort_InputMode_pullUp
} IfxPort_InputMode;

typedef enum {
    IfxPort_LvdsMode_high,
    IfxPort_LvdsMode_medium
} IfxPort_LvdsMode;

typedef enum {
    IfxPort_LvdsTerm_external,
    IfxPort_LvdsTerm_internal
} IfxPort_LvdsTerm;

typedef enum {
    IfxPort_Mode_inputNoPullDevice,
    IfxPort_Mode_inputPullDown,
    IfxPort_Mode_inputPullUp,
    IfxPort_Mode_outputPushPullGeneral,
    IfxPort_Mode_outputPushPullAlt1,
    IfxPort_Mode_outputPushPullAlt2,
    IfxPort_Mode_outputPushPullAlt3,
    IfxPort_Mode_outputPushPullAlt4,
    IfxPort_Mode_outputPushPullAlt5,
    IfxPort_Mode_outputPushPullAlt6,
    IfxPort_Mode_outputPushPullAlt7,
    IfxPort_Mode_outputPushPullAlt8,
    IfxPort_Mode_outputPushPullAlt9,
    IfxPort_Mode_outputPushPullAlt10,
    IfxPort_Mode_outputPushPullAlt11,
    IfxPort_Mode_outputPushPullAlt12,
    IfxPort_Mode_outputPushPullAlt13,
    IfxPort_Mode_outputPushPullAlt14,
    IfxPort_Mode_outputPushPullAlt15,
    IfxPort_Mode_outputOpenDrainGeneral,
    IfxPort_Mode_outputOpenDrainAlt1,
    IfxPort_Mode_outputOpenDrainAlt2,
    IfxPort_Mode_outputOpenDrainAlt3,
    IfxPort_Mode_outputOpenDrainAlt4,
    IfxPort_Mode_outputOpenDrainAlt5,
    IfxPort_Mode_outputOpenDrainAlt6,
    IfxPort_Mode_outputOpenDrainAlt7,
    IfxPort_Mode_outputOpenDrainAlt8,
    IfxPort_Mode_outputOpenDrainAlt9,
    IfxPort_Mode_outputOpenDrainAlt10,
    IfxPort_Mode_outputOpenDrainAlt11,
    IfxPort_Mode_outputOpenDrainAlt12,
    IfxPort_Mode_outputOpenDrainAlt13,
    IfxPort_Mode_outputOpenDrainAlt14,
    IfxPort_Mode_outputOpenDrainAlt15
} IfxPort_Mode;

typedef enum {
    IfxPort_Modex_differentialXspiGpio,
    IfxPort_Modex_singleEndedXspiGpio,
    IfxPort_Modex_gpioMode,
    IfxPort_Modex_xspiRgmiiMode
} IfxPort_Modex;

typedef enum {
    IfxPort_OutputIdx_general,
    IfxPort_OutputIdx_alt1,
    IfxPort_OutputIdx_alt2,
    IfxPort_OutputIdx_alt3,
    IfxPort_OutputIdx_alt4,
    IfxPort_OutputIdx_alt5,
    IfxPort_OutputIdx_alt6,
    IfxPort_OutputIdx_alt7,
    IfxPort_OutputIdx_alt8,
    IfxPort_OutputIdx_alt9,
    IfxPort_OutputIdx_alt10,
    IfxPort_OutputIdx_alt11,
    IfxPort_OutputIdx_alt12,
    IfxPort_OutputIdx_alt13,
    IfxPort_OutputIdx_alt14,
    IfxPort_OutputIdx_alt15
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_OutputMode_pushPull,
    IfxPort_OutputMode_openDrain
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadSupply_3v,
    IfxPort_PadSupply_5v
} IfxPort_PadSupply;

typedef enum {
    IfxPort_PinFunctionMode_digital,
    IfxPort_PinFunctionMode_analog
} IfxPort_PinFunctionMode;

typedef enum {
    IfxPort_State_notChanged,
    IfxPort_State_high,
    IfxPort_State_low,
    IfxPort_State_toggled
} IfxPort_State;

typedef enum {
    IfxPort_BandgapTrimConfig_1P199V,
    IfxPort_BandgapTrimConfig_1P184V,
    IfxPort_BandgapTrimConfig_1P168V,
    IfxPort_BandgapTrimConfig_1P153V,
    IfxPort_BandgapTrimConfig_1P139V,
    IfxPort_BandgapTrimConfig_1P125V,
    IfxPort_BandgapTrimConfig_1P112V,
    IfxPort_BandgapTrimConfig_1P098V,
    IfxPort_BandgapTrimConfig_1P340V,
    IfxPort_BandgapTrimConfig_1P321V,
    IfxPort_BandgapTrimConfig_1P302V,
    IfxPort_BandgapTrimConfig_1P283V,
    IfxPort_BandgapTrimConfig_1P266V,
    IfxPort_BandgapTrimConfig_1P248V,
    IfxPort_BandgapTrimConfig_1P231V,
    IfxPort_BandgapTrimConfig_1P215V
} IfxPort_BandgapTrimConfig;

typedef enum {
    IfxPort_BlankingTimerConfig_0ms,
    IfxPort_BlankingTimerConfig_2ms,
    IfxPort_BlankingTimerConfig_4ms,
    IfxPort_BlankingTimerConfig_7ms
} IfxPort_BlankingTimerConfig;

typedef enum {
    IfxPort_EsrLevel_0
} IfxPort_EsrLevel;

typedef enum {
    IfxPort_EsrPadCfg_PP,
    IfxPort_EsrPadCfg_TPU,
    IfxPort_EsrPadCfg_TPD
} IfxPort_EsrPadCfg;

typedef enum {
    IfxPort_LvdsDirection_rx,
    IfxPort_LvdsDirection_tx
} IfxPort_LvdsDirection;

typedef enum {
    IfxPort_LvdsPath_enable,
    IfxPort_LvdsPath_disable
} IfxPort_LvdsPath;

typedef enum {
    IfxPort_LvdsPullDown_disable,
    IfxPort_LvdsPullDown_enable
} IfxPort_LvdsPullDown;

typedef enum {
    IfxPort_LvdsTerminationMode_external,
    IfxPort_LvdsTerminationMode_internal
} IfxPort_LvdsTerminationMode;

typedef enum {
    IfxPort_PadAccessGroup_PadAccessGroup0,
    IfxPort_PadAccessGroup_PadAccessGroup1,
    IfxPort_PadAccessGroup_PadAccessGroup2,
    IfxPort_PadAccessGroup_PadAccessGroup3,
    IfxPort_PadAccessGroup_PadAccessGroup4,
    IfxPort_PadAccessGroup_PadAccessGroup5,
    IfxPort_PadAccessGroup_PadAccessGroup6,
    IfxPort_PadAccessGroup_PadAccessGroup7
} IfxPort_PadAccessGroup;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1,
    IfxPort_PadDriver_cmosAutomotiveSpeed2,
    IfxPort_PadDriver_cmosAutomotiveSpeed3,
    IfxPort_PadDriver_ttlSpeed1,
    IfxPort_PadDriver_ttlSpeed2,
    IfxPort_PadDriver_ttlSpeed3,
    IfxPort_PadDriver_ttl3v3Speed1,
    IfxPort_PadDriver_ttl3v3Speed2,
    IfxPort_PadDriver_ttl3v3Speed3
} IfxPort_PadDriver;

typedef struct
{
    uint8 pinIndex;       
    uint8 grpNum;         
} IfxPort_Pin_ApuConfig;

typedef struct
{
    IfxPort_LvdsMode     lvdsMode;                   
    IfxPort_ControlledBy enablePortControlled;       
    IfxPort_PadSupply    padSupply;                  
    IfxPort_LvdsTerm     lvdsTerm;                   
} IfxPort_LvdsConfig;

typedef struct
{
    Ifx_P *port;
    uint8  pinIndex;
} IfxPort_Pin;

typedef struct
{
    Ifx_P            *port;
    uint8             pinIndex;
    IfxPort_OutputIdx mode;
    IfxPort_PadDriver padDriver;
} IfxPort_Pin_Config;

typedef struct
{
    IfxApApu_ApuConfig apuConfig;       /**< \brief APU Configurations */
    uint8              grpNum;          /**< \brief The APU group number to which the pin is mapped to. */
} IfxPort_ApuConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;        /**< \brief PROT SE Configurations */
} IfxPort_ProtConfig;

void IfxPort_setPinModeOutput(Ifx_P * port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);
void IfxPort_togglePin(Ifx_P * port, uint8 pinIndex);

#endif /* IFXPORT_H */
