#ifndef IFXPORT_H
#define IFXPORT_H
#include "mock_egtm_atom_3_phase_inverter_pwm.h"

/* Basic port types */
typedef enum { IfxPort_ControlledBy_port = 0, IfxPort_ControlledBy_hsct = 1 } IfxPort_ControlledBy;
typedef enum { IfxPort_InputMode_undefined = -1, IfxPort_InputMode_noPullDevice = 0, IfxPort_InputMode_pullDown = 16, IfxPort_InputMode_pullUp = 32 } IfxPort_InputMode;
typedef enum { IfxPort_LvdsMode_high = 0, IfxPort_LvdsMode_medium = 1 } IfxPort_LvdsMode;
typedef enum { IfxPort_LvdsTerm_external = 0, IfxPort_LvdsTerm_internal = 1 } IfxPort_LvdsTerm;
typedef enum {
    IfxPort_Mode_inputNoPullDevice      = 0,
    IfxPort_Mode_inputPullDown          = 16,
    IfxPort_Mode_inputPullUp            = 32,
    IfxPort_Mode_outputPushPullGeneral  = 1,
    IfxPort_Mode_outputPushPullAlt1     = 17,
    IfxPort_Mode_outputPushPullAlt2     = 33,
    IfxPort_Mode_outputPushPullAlt3     = 49,
    IfxPort_Mode_outputPushPullAlt4     = 65,
    IfxPort_Mode_outputPushPullAlt5     = 81,
    IfxPort_Mode_outputPushPullAlt6     = 97,
    IfxPort_Mode_outputPushPullAlt7     = 113,
    IfxPort_Mode_outputPushPullAlt8     = 129,
    IfxPort_Mode_outputPushPullAlt9     = 145,
    IfxPort_Mode_outputPushPullAlt10    = 161,
    IfxPort_Mode_outputPushPullAlt11    = 177,
    IfxPort_Mode_outputPushPullAlt12    = 193,
    IfxPort_Mode_outputPushPullAlt13    = 209,
    IfxPort_Mode_outputPushPullAlt14    = 225,
    IfxPort_Mode_outputPushPullAlt15    = 241,
    IfxPort_Mode_outputOpenDrainGeneral = 3,
    IfxPort_Mode_outputOpenDrainAlt1    = 19,
    IfxPort_Mode_outputOpenDrainAlt2    = 35,
    IfxPort_Mode_outputOpenDrainAlt3    = 51,
    IfxPort_Mode_outputOpenDrainAlt4    = 67,
    IfxPort_Mode_outputOpenDrainAlt5    = 83,
    IfxPort_Mode_outputOpenDrainAlt6    = 99,
    IfxPort_Mode_outputOpenDrainAlt7    = 115,
    IfxPort_Mode_outputOpenDrainAlt8    = 131,
    IfxPort_Mode_outputOpenDrainAlt9    = 147,
    IfxPort_Mode_outputOpenDrainAlt10   = 163,
    IfxPort_Mode_outputOpenDrainAlt11   = 179,
    IfxPort_Mode_outputOpenDrainAlt12   = 195,
    IfxPort_Mode_outputOpenDrainAlt13   = 211,
    IfxPort_Mode_outputOpenDrainAlt14   = 227,
    IfxPort_Mode_outputOpenDrainAlt15   = 243
} IfxPort_Mode;

typedef enum { IfxPort_Modex_differentialXspiGpio = (0<<16), IfxPort_Modex_singleEndedXspiGpio = (1<<16), IfxPort_Modex_gpioMode = (2<<16), IfxPort_Modex_xspiRgmiiMode = (3<<16) } IfxPort_Modex;

typedef enum {
    IfxPort_OutputIdx_general = 1,
    IfxPort_OutputIdx_alt1    = 17,
    IfxPort_OutputIdx_alt2    = 33,
    IfxPort_OutputIdx_alt3    = 49,
    IfxPort_OutputIdx_alt4    = 65,
    IfxPort_OutputIdx_alt5    = 81,
    IfxPort_OutputIdx_alt6    = 97,
    IfxPort_OutputIdx_alt7    = 113,
    IfxPort_OutputIdx_alt8    = 129,
    IfxPort_OutputIdx_alt9    = 145,
    IfxPort_OutputIdx_alt10   = 161,
    IfxPort_OutputIdx_alt11   = 177,
    IfxPort_OutputIdx_alt12   = 193,
    IfxPort_OutputIdx_alt13   = 209,
    IfxPort_OutputIdx_alt14   = 225,
    IfxPort_OutputIdx_alt15   = 241
} IfxPort_OutputIdx;

typedef enum { IfxPort_OutputMode_pushPull = 1, IfxPort_OutputMode_openDrain = 3 } IfxPort_OutputMode;

typedef enum { IfxPort_PadSupply_3v = 0, IfxPort_PadSupply_5v = 1 } IfxPort_PadSupply;

typedef enum { IfxPort_PinFunctionMode_digital = 0, IfxPort_PinFunctionMode_analog = 1 } IfxPort_PinFunctionMode;

typedef enum { IfxPort_State_notChanged = 0, IfxPort_State_high = 1, IfxPort_State_low = 65536, IfxPort_State_toggled = 65537 } IfxPort_State;

typedef enum { IfxPort_BandgapTrimConfig_1P199V = 0, IfxPort_BandgapTrimConfig_1P184V = 1, IfxPort_BandgapTrimConfig_1P168V = 2, IfxPort_BandgapTrimConfig_1P153V = 3, IfxPort_BandgapTrimConfig_1P139V = 4, IfxPort_BandgapTrimConfig_1P125V = 5, IfxPort_BandgapTrimConfig_1P112V = 6, IfxPort_BandgapTrimConfig_1P098V = 7, IfxPort_BandgapTrimConfig_1P340V = 8, IfxPort_BandgapTrimConfig_1P321V = 9, IfxPort_BandgapTrimConfig_1P302V = 10, IfxPort_BandgapTrimConfig_1P283V = 11, IfxPort_BandgapTrimConfig_1P266V = 12, IfxPort_BandgapTrimConfig_1P248V = 13, IfxPort_BandgapTrimConfig_1P231V = 14, IfxPort_BandgapTrimConfig_1P215V = 15 } IfxPort_BandgapTrimConfig;

typedef enum { IfxPort_BlankingTimerConfig_0ms = 0, IfxPort_BlankingTimerConfig_2ms = 1, IfxPort_BlankingTimerConfig_4ms = 2, IfxPort_BlankingTimerConfig_7ms = 3 } IfxPort_BlankingTimerConfig;

typedef enum { IfxPort_EsrLevel_0 = 0 } IfxPort_EsrLevel;

typedef enum { IfxPort_EsrPadCfg_PP = 0, IfxPort_EsrPadCfg_TPU = 1, IfxPort_EsrPadCfg_TPD = 2 } IfxPort_EsrPadCfg;

typedef enum { IfxPort_LvdsDirection_rx = 0, IfxPort_LvdsDirection_tx = 1 } IfxPort_LvdsDirection;

typedef enum { IfxPort_LvdsPath_enable = 0, IfxPort_LvdsPath_disable = 1 } IfxPort_LvdsPath;

typedef enum { IfxPort_LvdsPullDown_disable = 0, IfxPort_LvdsPullDown_enable = 1 } IfxPort_LvdsPullDown;

typedef enum { IfxPort_LvdsTerminationMode_external = 0, IfxPort_LvdsTerminationMode_internal = 1 } IfxPort_LvdsTerminationMode;

typedef enum { IfxPort_PadAccessGroup_PadAccessGroup0 = 0, IfxPort_PadAccessGroup_PadAccessGroup1 = 1, IfxPort_PadAccessGroup_PadAccessGroup2 = 2, IfxPort_PadAccessGroup_PadAccessGroup3 = 3, IfxPort_PadAccessGroup_PadAccessGroup4 = 4, IfxPort_PadAccessGroup_PadAccessGroup5 = 5, IfxPort_PadAccessGroup_PadAccessGroup6 = 6, IfxPort_PadAccessGroup_PadAccessGroup7 = 7 } IfxPort_PadAccessGroup;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_ttlSpeed1            = 16,
    IfxPort_PadDriver_ttlSpeed2            = 17,
    IfxPort_PadDriver_ttlSpeed3            = 18,
    IfxPort_PadDriver_ttl3v3Speed1         = 24,
    IfxPort_PadDriver_ttl3v3Speed2         = 25,
    IfxPort_PadDriver_ttl3v3Speed3         = 26
} IfxPort_PadDriver;

typedef struct { Ifx_P *port; uint8 pinIndex; } IfxPort_Pin;

typedef struct { Ifx_P *port; uint8 pinIndex; IfxPort_OutputIdx mode; IfxPort_PadDriver padDriver; } IfxPort_Pin_Config;

/* Functions used by production */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);

#endif /* IFXPORT_H */
