#ifndef IFXPORT_H
#define IFXPORT_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Enums (complete per spec) */
typedef enum {
    IfxPort_ControlledBy_port = 0,
    IfxPort_ControlledBy_hsct = 1
} IfxPort_ControlledBy;

typedef enum {
    IfxPort_InputMode_undefined    = -1,
    IfxPort_InputMode_noPullDevice = 0 << 3,
    IfxPort_InputMode_pullDown     = 1u << 3,
    IfxPort_InputMode_pullUp       = 2u << 3
} IfxPort_InputMode;

typedef enum {
    IfxPort_LvdsMode_high   = 0,
    IfxPort_LvdsMode_medium = 1
} IfxPort_LvdsMode;

typedef enum {
    IfxPort_Mode_inputNoPullDevice      = 0,
    IfxPort_Mode_inputPullDown          = 8u,
    IfxPort_Mode_inputPullUp            = 0x10u,
    IfxPort_Mode_outputPushPullGeneral  = 0x80u,
    IfxPort_Mode_outputPushPullAlt1     = 0x88u,
    IfxPort_Mode_outputPushPullAlt2     = 0x90u,
    IfxPort_Mode_outputPushPullAlt3     = 0x98u,
    IfxPort_Mode_outputPushPullAlt4     = 0xA0u,
    IfxPort_Mode_outputPushPullAlt5     = 0xA8u,
    IfxPort_Mode_outputPushPullAlt6     = 0xB0u,
    IfxPort_Mode_outputPushPullAlt7     = 0xB8u,
    IfxPort_Mode_outputOpenDrainGeneral = 0xC0u,
    IfxPort_Mode_outputOpenDrainAlt1    = 0xC8u,
    IfxPort_Mode_outputOpenDrainAlt2    = 0xD0u,
    IfxPort_Mode_outputOpenDrainAlt3    = 0xD8u,
    IfxPort_Mode_outputOpenDrainAlt4    = 0xE0u,
    IfxPort_Mode_outputOpenDrainAlt5    = 0xE8u,
    IfxPort_Mode_outputOpenDrainAlt6    = 0xF0u,
    IfxPort_Mode_outputOpenDrainAlt7    = 0xF8u
} IfxPort_Mode;

typedef enum {
    IfxPort_OutputIdx_general  = 0x10u << 3,
    IfxPort_OutputIdx_alt1     = 0x11u << 3,
    IfxPort_OutputIdx_alt2     = 0x12u << 3,
    IfxPort_OutputIdx_alt3     = 0x13u << 3,
    IfxPort_OutputIdx_alt4     = 0x14u << 3,
    IfxPort_OutputIdx_alt5     = 0x15u << 3,
    IfxPort_OutputIdx_alt6     = 0x16u << 3,
    IfxPort_OutputIdx_alt7     = 0x17u << 3
} IfxPort_OutputIdx;

typedef enum {
    IfxPort_OutputMode_pushPull      = 0x10u << 3,
    IfxPort_OutputMode_openDrain     = 0x18u << 3,
    IfxPort_OutputMode_none          = 0
} IfxPort_OutputMode;

typedef enum {
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = 0,
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = 1,
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = 2,
    IfxPort_PadDriver_cmosAutomotiveSpeed4 = 3,
    IfxPort_PadDriver_ttlSpeed1            = 8,
    IfxPort_PadDriver_ttlSpeed2            = 9,
    IfxPort_PadDriver_ttlSpeed3            = 10,
    IfxPort_PadDriver_ttlSpeed4            = 11,
    IfxPort_PadDriver_ttl3v3Speed1         = 12,
    IfxPort_PadDriver_ttl3v3Speed2         = 13,
    IfxPort_PadDriver_ttl3v3Speed3         = 14,
    IfxPort_PadDriver_ttl3v3Speed4         = 15
} IfxPort_PadDriver;

typedef enum {
    IfxPort_PadSupply_3v = 0,
    IfxPort_PadSupply_5v = 1
} IfxPort_PadSupply;

typedef enum {
    IfxPort_PinFunctionMode_digital = 0,
    IfxPort_PinFunctionMode_analog  = 1
} IfxPort_PinFunctionMode;

typedef enum {
    IfxPort_State_notChanged = (0 << 16) | (0 << 0),
    IfxPort_State_high       = (0 << 16) | (1u << 0),
    IfxPort_State_low        = (1u << 16) | (0 << 0),
    IfxPort_State_toggled    = (1u << 16) | (1u << 0)
} IfxPort_State;

typedef enum {
    IfxPort_LvdsDirection_rx = 0,
    IfxPort_LvdsDirection_tx = 1
} IfxPort_LvdsDirection;

typedef enum {
    IfxPort_LvdsPath_enable  = 0,
    IfxPort_LvdsPath_disable = 1
} IfxPort_LvdsPath;

typedef enum {
    IfxPort_LvdsPullDown_disable = 0,
    IfxPort_LvdsPullDown_enable  = 1
} IfxPort_LvdsPullDown;

typedef enum {
    IfxPort_LvdsTerminationMode_external = 0,
    IfxPort_LvdsTerminationMode_internal = 1
} IfxPort_LvdsTerminationMode;

/* Structs */
typedef struct {
    IfxPort_LvdsMode     lvdsMode;
    IfxPort_ControlledBy enablePortControlled;
    IfxPort_PadSupply    padSupply;
} IfxPort_LvdsConfig;

typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
} IfxPort_Pin;

typedef struct {
    Ifx_P              *port;
    uint8               pinIndex;
    IfxPort_OutputIdx   mode;
    IfxPort_PadDriver   padDriver;
} IfxPort_Pin_Config;

/* Functions (subset required by module/tests) */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);

#endif /* IFXPORT_H */
