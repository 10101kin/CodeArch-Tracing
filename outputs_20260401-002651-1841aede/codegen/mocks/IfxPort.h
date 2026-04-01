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
    IfxApApu_ApuConfig    apuConfig[IFXPORT_NUM_APU];       /**< \brief APU Configurations for all the APUs */
    IfxPort_Pin_ApuConfig pinConfig[IFXPORT_NUM_PINS];      /**< \brief Configuration to map a specific pin to a APU group */
} IfxPort_ApuGroupConfig;

typedef struct
{
    IfxApProt_ProtConfig protseConfig;        /**< \brief PROT SE Configurations */
} IfxPort_ProtConfig;

typedef enum
{
    IfxPort_ControlledBy_port = 0,  /**< \brief port controlled by PORT Module */
    IfxPort_ControlledBy_hsct = 1   /**< \brief Port controlled by HSCT Module */
} IfxPort_ControlledBy;

typedef enum
{
    IfxPort_InputMode_undefined    = -1,
    IfxPort_InputMode_noPullDevice = ((0U << 4U) | (0U << 1U) | 0U),
    IfxPort_InputMode_pullDown     = ((1U << 4U) | (0U << 1U) | 0U),
    IfxPort_InputMode_pullUp       = ((2U << 4U) | (0U << 1U) | 0U)
} IfxPort_InputMode;

typedef enum
{
    IfxPort_LvdsMode_high   = 0, /**< \brief LVDS-H Mode */
    IfxPort_LvdsMode_medium = 1  /**< \brief LVDS-M Mode */
} IfxPort_LvdsMode;

typedef enum
{
    IfxPort_LvdsTerm_external = 0,  /**< \brief external termination */
    IfxPort_LvdsTerm_internal = 1   /**< \brief 100 Ohm internal termination */
} IfxPort_LvdsTerm;

typedef enum
{
    IfxPort_Mode_inputNoPullDevice      = ((0U << 4U) | (0U << 1U) | 0U),  /**< \brief Input, No pull device connected. */
    IfxPort_Mode_inputPullDown          = ((1U << 4U) | (0U << 1U) | 0U),  /**< \brief Input, pull-down device connected. */
    IfxPort_Mode_inputPullUp            = ((2U << 4U) | (0U << 1U) | 0U),  /**< \brief Input, pull-up device connected. */
    IfxPort_Mode_outputPushPullGeneral  = ((0U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, General-purpose output */
    IfxPort_Mode_outputPushPullAlt1     = ((1U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 1. */
    IfxPort_Mode_outputPushPullAlt2     = ((2U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 2. */
    IfxPort_Mode_outputPushPullAlt3     = ((3U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 3. */
    IfxPort_Mode_outputPushPullAlt4     = ((4U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 4. */
    IfxPort_Mode_outputPushPullAlt5     = ((5U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 5. */
    IfxPort_Mode_outputPushPullAlt6     = ((6U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 6. */
    IfxPort_Mode_outputPushPullAlt7     = ((7U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 7. */
    IfxPort_Mode_outputPushPullAlt8     = ((8U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 8. */
    IfxPort_Mode_outputPushPullAlt9     = ((9U << 4U) | (0U << 1U) | 1U),  /**< \brief Push-pull, Alternate output function 9. */
    IfxPort_Mode_outputPushPullAlt10    = ((10U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 10. */
    IfxPort_Mode_outputPushPullAlt11    = ((11U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 11. */
    IfxPort_Mode_outputPushPullAlt12    = ((12U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 12. */
    IfxPort_Mode_outputPushPullAlt13    = ((13U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 13. */
    IfxPort_Mode_outputPushPullAlt14    = ((14U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 14. */
    IfxPort_Mode_outputPushPullAlt15    = ((15U << 4U) | (0U << 1U) | 1U), /**< \brief Push-pull, Alternate output function 15. */
    IfxPort_Mode_outputOpenDrainGeneral = ((0U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, General-purpose output. */
    IfxPort_Mode_outputOpenDrainAlt1    = ((1U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 1. */
    IfxPort_Mode_outputOpenDrainAlt2    = ((2U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 2. */
    IfxPort_Mode_outputOpenDrainAlt3    = ((3U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 3. */
    IfxPort_Mode_outputOpenDrainAlt4    = ((4U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 4. */
    IfxPort_Mode_outputOpenDrainAlt5    = ((5U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 5. */
    IfxPort_Mode_outputOpenDrainAlt6    = ((6U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 6. */
    IfxPort_Mode_outputOpenDrainAlt7    = ((7U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 7. */
    IfxPort_Mode_outputOpenDrainAlt8    = ((8U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 8. */
    IfxPort_Mode_outputOpenDrainAlt9    = ((9U << 4U) | (1U << 1U) | 1U),  /**< \brief Open-drain, Alternate output function 9. */
    IfxPort_Mode_outputOpenDrainAlt10   = ((10U << 4U) | (1U << 1U) | 1U), /**< \brief Open-drain, Alternate output function 10. */
    IfxPort_Mode_outputOpenDrainAlt11   = ((11U << 4U) | (1U << 1U) | 1U), /**< \brief Open-drain, Alternate output function 11. */
    IfxPort_Mode_outputOpenDrainAlt12   = ((12U << 4U) | (1U << 1U) | 1U), /**< \brief Open-drain, Alternate output function 12. */
    IfxPort_Mode_outputOpenDrainAlt13   = ((13U << 4U) | (1U << 1U) | 1U), /**< \brief Open-drain, Alternate output function 13. */
    IfxPort_Mode_outputOpenDrainAlt14   = ((14U << 4U) | (1U << 1U) | 1U), /**< \brief Open-drain, Alternate output function 14. */
    IfxPort_Mode_outputOpenDrainAlt15   = ((15U << 4U) | (1U << 1U) | 1U)  /**< \brief Open-drain, Alternate output function 15. */
} IfxPort_Mode;

typedef enum
{
    IfxPort_Modex_differentialXspiGpio = (0 << 16),  /**< \brief Differential xspi, Rgmii or GPIO */
    IfxPort_Modex_singleEndedXspiGpio  = (1 << 16),  /**< \brief Single ended xspi or gpio */
    IfxPort_Modex_gpioMode             = (2 << 16),  /**< \brief Gpio Mode */
    IfxPort_Modex_xspiRgmiiMode        = (3 << 16)   /**< \brief Xspi or Rgmii mode */
} IfxPort_Modex;

typedef enum
{
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
} IfxPort_OutputIdx;

typedef enum
{
    IfxPort_OutputMode_pushPull  = ((0U << 1U) | 1U),
    IfxPort_OutputMode_openDrain = ((1U << 1U) | 1U)
} IfxPort_OutputMode;

typedef enum
{
    IfxPort_PadSupply_3v = 0,  /**< \brief select,3.3v */
    IfxPort_PadSupply_5v = 1   /**< \brief select,5V */
} IfxPort_PadSupply;

typedef enum
{
    IfxPort_PinFunctionMode_digital = 0,  /**< \brief Pad Pn.x is enabled and can be selected for digital function */
    IfxPort_PinFunctionMode_analog  = 1   /**< \brief Pad Pn.x is enabled and can be selected for analog function */
} IfxPort_PinFunctionMode;

typedef enum
{
    IfxPort_State_notChanged = (0 << 16) | (0 << 0),  /**< \brief Ifx_P pin is left unchanged. */
    IfxPort_State_high       = (0 << 16) | (1U << 0), /**< \brief Ifx_P pin is set to high. */
    IfxPort_State_low        = (1U << 16) | (0 << 0), /**< \brief Ifx_P pin is set to low. */
    IfxPort_State_toggled    = (1U << 16) | (1U << 0) /**< \brief Ifx_P pin is toggled. */
} IfxPort_State;

typedef enum
{
    IfxPort_BandgapTrimConfig_1P199V = 0,   /**< \brief The trimming value is zero. */
    IfxPort_BandgapTrimConfig_1P184V = 1,   /**< \brief The trimming value is one. */
    IfxPort_BandgapTrimConfig_1P168V = 2,   /**< \brief The trimming value is two. */
    IfxPort_BandgapTrimConfig_1P153V = 3,   /**< \brief The trimming value is three. */
    IfxPort_BandgapTrimConfig_1P139V = 4,   /**< \brief The trimming value is four. */
    IfxPort_BandgapTrimConfig_1P125V = 5,   /**< \brief The trimming value is five. */
    IfxPort_BandgapTrimConfig_1P112V = 6,   /**< \brief The trimming value is six. */
    IfxPort_BandgapTrimConfig_1P098V = 7,   /**< \brief The trimming value is seven. */
    IfxPort_BandgapTrimConfig_1P340V = 8,   /**< \brief The trimming value is eight. */
    IfxPort_BandgapTrimConfig_1P321V = 9,   /**< \brief The trimming value is nine. */
    IfxPort_BandgapTrimConfig_1P302V = 10,  /**< \brief The trimming value is ten. */
    IfxPort_BandgapTrimConfig_1P283V = 11,  /**< \brief The trimming value is eleven. */
    IfxPort_BandgapTrimConfig_1P266V = 12,  /**< \brief The trimming value is twelve. */
    IfxPort_BandgapTrimConfig_1P248V = 13,  /**< \brief The trimming value is thirteen */
    IfxPort_BandgapTrimConfig_1P231V = 14,  /**< \brief The trimming value is fourteen. */
    IfxPort_BandgapTrimConfig_1P215V = 15   /**< \brief The trimming value is fifteen. */
} IfxPort_BandgapTrimConfig;

typedef enum
{
    IfxPort_BlankingTimerConfig_0ms = 0,  /**< \brief Corresponds to blanking time of 0 ms */
    IfxPort_BlankingTimerConfig_2ms = 1,  /**< \brief Corresponds to blanking time of 2 ms */
    IfxPort_BlankingTimerConfig_4ms = 2,  /**< \brief Corresponds to blanking time of 4 ms */
    IfxPort_BlankingTimerConfig_7ms = 3   /**< \brief Corresponds to blanking time of 7 ms */
} IfxPort_BlankingTimerConfig;

typedef enum
{
    IfxPort_EsrLevel_0 = 0,      /**< \brief ESR Level0  */
    IfxPort_EsrLevel_1           /**< \brief ESR Level1  */
} IfxPort_EsrLevel;

typedef enum
{
    IfxPort_EsrPadCfg_PP  = 0,  /**< \brief PAD in Push-PULL */
    IfxPort_EsrPadCfg_TPU = 1,  /**< \brief Tristate, weak pull-up */
    IfxPort_EsrPadCfg_TPD = 2   /**< \brief Tristate, weak pull-down */
} IfxPort_EsrPadCfg;

typedef enum
{
    IfxPort_LvdsDirection_rx = 0,  /**< \brief LVDS direction RX */
    IfxPort_LvdsDirection_tx = 1   /**< \brief LVDS direction TX */
} IfxPort_LvdsDirection;

typedef enum
{
    IfxPort_LvdsPath_enable  = 0, /**< \brief LVDS enabled */
    IfxPort_LvdsPath_disable = 1  /**< \brief LVDS disabled */
} IfxPort_LvdsPath;

typedef enum
{
    IfxPort_LvdsPullDown_disable = 0,  /**< \brief Disable Pull Down resistor */
    IfxPort_LvdsPullDown_enable  = 1   /**< \brief Enable Pull Down resistor */
} IfxPort_LvdsPullDown;

typedef enum
{
    IfxPort_LvdsTerminationMode_external = 0,  /**< \brief Termination Mode External */
    IfxPort_LvdsTerminationMode_internal = 1   /**< \brief Termination Mode Internal */
} IfxPort_LvdsTerminationMode;

typedef enum
{
    IfxPort_PadAccessGroup_PadAccessGroup0 = 0,  /**< \brief Pad Access Group 0 */
    IfxPort_PadAccessGroup_PadAccessGroup1 = 1,  /**< \brief Pad Access Group 1 */
    IfxPort_PadAccessGroup_PadAccessGroup2 = 2,  /**< \brief Pad Access Group 2 */
    IfxPort_PadAccessGroup_PadAccessGroup3 = 3,  /**< \brief Pad Access Group 3 */
    IfxPort_PadAccessGroup_PadAccessGroup4 = 4,  /**< \brief Pad Access Group 4 */
    IfxPort_PadAccessGroup_PadAccessGroup5 = 5,  /**< \brief Pad Access Group 5 */
    IfxPort_PadAccessGroup_PadAccessGroup6 = 6,  /**< \brief Pad Access Group 6 */
    IfxPort_PadAccessGroup_PadAccessGroup7 = 7   /**< \brief Pad Access Group 7 */
} IfxPort_PadAccessGroup;

typedef enum
{
    IfxPort_PadDriver_cmosAutomotiveSpeed1 = (0 << 3) | (0 << 0),  /**< \brief Speed grade 1. */
    IfxPort_PadDriver_cmosAutomotiveSpeed2 = (0 << 3) | (1 << 0),  /**< \brief Speed grade 2. */
    IfxPort_PadDriver_cmosAutomotiveSpeed3 = (0 << 3) | (2 << 0),  /**< \brief Speed grade 3. */
    IfxPort_PadDriver_ttlSpeed1            = (2 << 3) | (0 << 0),  /**< \brief Speed grade 1. */
    IfxPort_PadDriver_ttlSpeed2            = (2 << 3) | (1 << 0),  /**< \brief Speed grade 2. */
    IfxPort_PadDriver_ttlSpeed3            = (2 << 3) | (2 << 0),  /**< \brief Speed grade 3. */
    IfxPort_PadDriver_ttl3v3Speed1         = (3 << 3) | (0 << 0),  /**< \brief ttl3v3Speed1. */
    IfxPort_PadDriver_ttl3v3Speed2         = (3 << 3) | (1 << 0),  /**< \brief ttl3v3Speed2. */
    IfxPort_PadDriver_ttl3v3Speed3         = (3 << 3) | (2 << 0)   /**< \brief ttl3v3Speed3. */
} IfxPort_PadDriver;

void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex);
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index);

#endif /* IFXPORT_H */
