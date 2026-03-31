/* IfxPort.h - per-driver mock header */
#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"  /* For IfxApApu_ApuConfig, IfxApProt_ProtConfig */

/* ====== Configuration constants required by types ====== */
#ifndef IFXPORT_NUM_APU
# define IFXPORT_NUM_APU 1
#endif
#ifndef IFXPORT_NUM_PINS
# define IFXPORT_NUM_PINS 16
#endif

/* ====== Enums (define ALL required values) ====== */
typedef enum
{
    IfxPort_ControlledBy_port = 0,  
    IfxPort_ControlledBy_hsct = 1   
} IfxPort_ControlledBy;{
    IfxPort_InputMode_undefined    = -1,
    IfxPort_InputMode_noPullDevice = ((0U << 4U) | (0U << 1U) | 0U),
    IfxPort_InputMode_pullDown     = ((1U << 4U) | (0U << 1U) | 0U),
    IfxPort_InputMode_pullUp       = ((2U << 4U) | (0U << 1U) | 0U)
} IfxPort_InputMode;{ IfxPort_LvdsMode_high = 0, IfxPort_LvdsMode_medium = 1 } IfxPort_LvdsMode;{ IfxPort_LvdsTerm_external = 0, IfxPort_LvdsTerm_internal = 1 } IfxPort_LvdsTerm;{
    IfxPort_Mode_inputNoPullDevice      = ((0U << 4U) | (0U << 1U) | 0U),
    IfxPort_Mode_inputPullDown          = ((1U << 4U) | (0U << 1U) | 0U),
    IfxPort_Mode_inputPullUp            = ((2U << 4U) | (0U << 1U) | 0U),
    IfxPort_Mode_outputPushPullGeneral  = ((0U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt1     = ((1U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt2     = ((2U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt3     = ((3U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt4     = ((4U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt5     = ((5U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt6     = ((6U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt7     = ((7U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt8     = ((8U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt9     = ((9U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt10    = ((10U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt11    = ((11U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt12    = ((12U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt13    = ((13U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt14    = ((14U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputPushPullAlt15    = ((15U << 4U) | (0U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainGeneral = ((0U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt1    = ((1U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt2    = ((2U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt3    = ((3U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt4    = ((4U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt5    = ((5U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt6    = ((6U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt7    = ((7U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt8    = ((8U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt9    = ((9U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt10   = ((10U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt11   = ((11U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt12   = ((12U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt13   = ((13U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt14   = ((14U << 4U) | (1U << 1U) | 1U),
    IfxPort_Mode_outputOpenDrainAlt15   = ((15U << 4U) | (1U << 1U) | 1U)
} IfxPort_Mode;{
    IfxPort_Modex_differentialXspiGpio = (0 << 16),
    IfxPort_Modex_singleEndedXspiGpio  = (1 << 16),
    IfxPort_Modex_gpioMode             = (2 << 16),
    IfxPort_Modex_xspiRgmiiMode        = (3 << 16)
} IfxPort_Modex;{
    IfxPort_OutputIdx_general = ((0U << 4U) | 1U),
    IfxPort_OutputIdx_alt1    = ((1U << 4U) | 1U),
    IfxPort_OutputIdx_alt2    = ((2U << 4U) | 1U),
    IfxPort_OutputIdx_alt3    = ((3U << 4U) | 1U),
    IfxPort_OutputIdx_alt4    = ((4U << 4U) | 1U),
    IfxPort_OutputIdx_alt5    = ((5U << 4U) | 1U),
    IfxPort_OutputIdx_alt6    = ((6U << 4U) | 1U),
    IfxPort_OutputIdx_alt7    = ((7U << 4U) | 1U),
    IfxPort_OutputIdx_alt8    = ((8U << 4U) | 1U),
    IfxPort_OutputIdx_alt9    = ((9U << 4U) | 1U),
    IfxPort_OutputIdx_alt10   = ((10U << 4U) | 1U),
    IfxPort_OutputIdx_alt11   = ((11U << 4U) | 1U),
    IfxPort_OutputIdx_alt12   = ((12U << 4U) | 1U),
    IfxPort_OutputIdx_alt13   = ((13U << 4U) | 1U),
    IfxPort_OutputIdx_alt14   = ((14U << 4U) | 1U),
    IfxPort_OutputIdx_alt15   = ((15U << 4U) | 1U)
} IfxPort_OutputIdx;{ IfxPort_OutputMode_pushPull = ((0U << 1U) | 1U), IfxPort_OutputMode_openDrain = ((1U << 1U) | 1U) } IfxPort_OutputMode;{ IfxPort_PadSupply_3v = 0, IfxPort_PadSupply_5v = 1 } IfxPort_PadSupply;{ IfxPort_PinFunctionMode_digital = 0, IfxPort_PinFunctionMode_analog = 1 } IfxPort_PinFunctionMode;{
    IfxPort_State_notChanged = (0 << 16) | (0 << 0),
    IfxPort_State_high       = (0 << 16) | (1U << 0),
    IfxPort_State_low        = (1 << 16) | (0 << 0),
    IfxPort_State_toggled    = (1 << 16) | (1 << 0)
} IfxPort_State;{
    IfxPort_BandgapTrimConfig_1P199V = 0,
    IfxPort_BandgapTrimConfig_1P184V = 1,
    IfxPort_BandgapTrimConfig_1P168V = 2,
    IfxPort_BandgapTrimConfig_1P153V = 3,
    IfxPort_BandgapTrimConfig_1P139V = 4,
    IfxPort_BandgapTrimConfig_1P125V = 5,
    IfxPort_BandgapTrimConfig_1P112V = 6,
    IfxPort_BandgapTrimConfig_1P098V = 7,
    IfxPort_BandgapTrimConfig_1P340V = 8,
    IfxPort_BandgapTrimConfig_1P321V = 9,
    IfxPort_BandgapTrimConfig_1P302V = 10,
    IfxPort_BandgapTrimConfig_1P283V = 11,
    IfxPort_BandgapTrimConfig_1P266V = 12,
    IfxPort_BandgapTrimConfig_1P248V = 13,
    IfxPort_BandgapTrimConfig_1P231V = 14,
    IfxPort_BandgapTrimConfig_1P215V = 15
} IfxPort_BandgapTrimConfig;{
    IfxPort_BlankingTimerConfig_0ms = 0,
    IfxPort_BlankingTimerConfig_2ms = 1,
    IfxPort_BlankingTimerConfig_4ms = 2,
    IfxPort_BlankingTimerConfig_7ms = 3
} IfxPort_BlankingTimerConfig;{ IfxPort_EsrLevel_0 = 0, IfxPort_EsrLevel_1 } IfxPort_EsrLevel;{ IfxPort_EsrPadCfg_PP = 0, IfxPort_EsrPadCfg_TPU = 1, IfxPort_EsrPadCfg_TPD = 2 } IfxPort_EsrPadCfg;{ IfxPort_LvdsDirection_rx = 0, IfxPort_LvdsDirection_tx = 1 } IfxPort_LvdsDirection;{ IfxPort_LvdsPath_enable = 0, IfxPort_LvdsPath_disable = 1 } IfxPort_LvdsPath;{ IfxPort_LvdsPullDown_disable = 0, IfxPort_LvdsPullDown_enable = 1 } IfxPort_LvdsPullDown;{ IfxPort_LvdsTerminationMode_external = 0, IfxPort_LvdsTerminationMode_internal = 1 } IfxPort_LvdsTerminationMode;{
    IfxPort_PadAccessGroup_PadAccessGroup0 = 0,
    IfxPort_PadAccessGroup_PadAccessGroup1 = 1,
    IfxPort_PadAccessGroup_PadAccessGroup2 = 2,
    IfxPort_PadAccessGroup_PadAccessGroup3 = 3,
    IfxPort_PadAccessGroup_PadAccessGroup4 = 4,
    IfxPort_PadAccessGroup_PadAccessGroup5 = 5,
    IfxPort_PadAccessGroup_PadAccessGroup6 = 6,
    IfxPort_PadAccessGroup_PadAccessGroup7 = 7
} IfxPort_PadAccessGroup;{
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = (0 << 3) | (0 << 0),
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = (0 << 3) | (1 << 0),
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = (0 << 3) | (2 << 0),
    IfxPort_PadDriver_ttlSpeed1            = (2 << 3) | (0 << 0),
    IfxPort_PadDriver_ttlSpeed2            = (2 << 3) | (1 << 0),
    IfxPort_PadDriver_ttlSpeed3            = (2 << 3) | (2 << 0),
    IfxPort_PadDriver_ttl3v3Speed1         = (3 << 3) | (0 << 0),
    IfxPort_PadDriver_ttl3v3Speed2         = (3 << 3) | (1 << 0),
    IfxPort_PadDriver_ttl3v3Speed3         = (3 << 3) | (2 << 0)
} IfxPort_PadDriver;

/* ====== Structs ====== */
typedef struct
{
    uint8 pinIndex;       
    uint8 grpNum;         
} IfxPort_Pin_ApuConfig;{
    IfxPort_LvdsMode     lvdsMode;
    IfxPort_ControlledBy enablePortControlled;
    IfxPort_PadSupply    padSupply;
    IfxPort_LvdsTerm     lvdsTerm;
} IfxPort_LvdsConfig;{
    Ifx_P *port;
    uint8  pinIndex;
} IfxPort_Pin;{
    Ifx_P            *port;
    uint8             pinIndex;
    IfxPort_OutputIdx mode;
    IfxPort_PadDriver padDriver;
} IfxPort_Pin_Config;{
    IfxApApu_ApuConfig apuConfig;
    uint8              grpNum;
} IfxPort_ApuConfig;{
    IfxApApu_ApuConfig    apuConfig[IFXPORT_NUM_APU];
    IfxPort_Pin_ApuConfig pinConfig[IFXPORT_NUM_PINS];
} IfxPort_ApuGroupConfig;{
    IfxApProt_ProtConfig protseConfig;
} IfxPort_ProtConfig;

/* ====== Function declarations (subset used by module) ====== */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
