/* mock_egtm_atom_3_phase_inverter_pwm.h - Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float    float32;
typedef unsigned long uint32;
typedef signed long   sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short   sint16;
typedef unsigned char  boolean;
typedef unsigned int   Ifx_Priority;

typedef uint32 Ifx_UReg_32Bit; /* Used by driver register pointer fields */

/* Macros */
#ifndef TRUE
#define TRUE  ((boolean)1)
#endif
#ifndef FALSE
#define FALSE ((boolean)0)
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif
/* IFX_INTERRUPT must be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_none = 0,
    IfxSrc_Tos_cpu0 = 1,
    IfxSrc_Tos_cpu1 = 2,
    IfxSrc_Tos_cpu2 = 3,
    IfxSrc_Tos_cpu3 = 4,
    IfxSrc_Tos_cpu4 = 5,
    IfxSrc_Tos_cpu5 = 6,
    IfxSrc_Tos_dma  = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_none = 0
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* Minimal SFR block stubs: one per unique type */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_ASCLIN0;
typedef struct { uint32 reserved; } Ifx_ASCLIN1;
typedef struct { uint32 reserved; } Ifx_ASCLIN2;
typedef struct { uint32 reserved; } Ifx_ASCLIN3;
typedef struct { uint32 reserved; } Ifx_ASCLIN4;
typedef struct { uint32 reserved; } Ifx_ASCLIN5;
typedef struct { uint32 reserved; } Ifx_ASCLIN6;
typedef struct { uint32 reserved; } Ifx_ASCLIN7;
typedef struct { uint32 reserved; } Ifx_ASCLIN8;
typedef struct { uint32 reserved; } Ifx_ASCLIN9;
typedef struct { uint32 reserved; } Ifx_ASCLIN10;
typedef struct { uint32 reserved; } Ifx_ASCLIN11;
typedef struct { uint32 reserved; } Ifx_ASCLIN12;
typedef struct { uint32 reserved; } Ifx_ASCLIN13;
typedef struct { uint32 reserved; } Ifx_ASCLIN14;
typedef struct { uint32 reserved; } Ifx_ASCLIN15;
typedef struct { uint32 reserved; } Ifx_ASCLIN16;
typedef struct { uint32 reserved; } Ifx_ASCLIN17;
typedef struct { uint32 reserved; } Ifx_ASCLIN18;
typedef struct { uint32 reserved; } Ifx_ASCLIN19;
typedef struct { uint32 reserved; } Ifx_ASCLIN20;
typedef struct { uint32 reserved; } Ifx_ASCLIN21;
typedef struct { uint32 reserved; } Ifx_ASCLIN22;
typedef struct { uint32 reserved; } Ifx_ASCLIN23;
typedef struct { uint32 reserved; } Ifx_ASCLIN24;
typedef struct { uint32 reserved; } Ifx_ASCLIN25;
typedef struct { uint32 reserved; } Ifx_ASCLIN26;
typedef struct { uint32 reserved; } Ifx_ASCLIN27;
typedef struct { uint32 reserved; } Ifx_TBCU;
typedef struct { uint32 reserved; } Ifx_CSBCU;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_COMBCU;
typedef struct { uint32 reserved; } Ifx_CANXL0;
typedef struct { uint32 reserved; } Ifx_CANXL0_RAM;
typedef struct { uint32 reserved; } Ifx_CAN0;
typedef struct { uint32 reserved; } Ifx_CAN0_RAM;
typedef struct { uint32 reserved; } Ifx_CAN1;
typedef struct { uint32 reserved; } Ifx_CAN1_RAM;
typedef struct { uint32 reserved; } Ifx_CAN2;
typedef struct { uint32 reserved; } Ifx_CAN2_RAM;
typedef struct { uint32 reserved; } Ifx_CAN3;
typedef struct { uint32 reserved; } Ifx_CAN3_RAM;
typedef struct { uint32 reserved; } Ifx_CAN4;
typedef struct { uint32 reserved; } Ifx_CAN4_RAM;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CLOCK;
typedef struct { uint32 reserved; } Ifx_CPU_CFI0;
typedef struct { uint32 reserved; } Ifx_CPU_CFI1;
typedef struct { uint32 reserved; } Ifx_CPU_CFI2;
typedef struct { uint32 reserved; } Ifx_CPU_CFI3;
typedef struct { uint32 reserved; } Ifx_CPU_CFI4;
typedef struct { uint32 reserved; } Ifx_CPU_CFI5;
typedef struct { uint32 reserved; } Ifx_CPU_CFICS;
typedef struct { uint32 reserved; } Ifx_CPU0;
typedef struct { uint32 reserved; } Ifx_CPU1;
typedef struct { uint32 reserved; } Ifx_CPU2;
typedef struct { uint32 reserved; } Ifx_CPU3;
typedef struct { uint32 reserved; } Ifx_CPU4;
typedef struct { uint32 reserved; } Ifx_CPU5;
typedef struct { uint32 reserved; } Ifx_CPUCS;
typedef struct { uint32 reserved; } Ifx_DMA0;
typedef struct { uint32 reserved; } Ifx_DMA1;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM0;
typedef struct { uint32 reserved; } Ifx_DOM1;
typedef struct { uint32 reserved; } Ifx_DOM3;
typedef struct { uint32 reserved; } Ifx_DOM6;
typedef struct { uint32 reserved; } Ifx_DOM7;
typedef struct { uint32 reserved; } Ifx_DOM4;
typedef struct { uint32 reserved; } Ifx_DOM2;
typedef struct { uint32 reserved; } Ifx_DOM5;
typedef struct { uint32 reserved; } Ifx_DRE;
typedef struct { uint32 reserved; } Ifx_ERAY0;
typedef struct { uint32 reserved; } Ifx_ERAY1;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI_CSRM;
typedef struct { uint32 reserved; } Ifx_FSI_HOST;
typedef struct { uint32 reserved; } Ifx_GETH0;
typedef struct { uint32 reserved; } Ifx_HSCT0;
typedef struct { uint32 reserved; } Ifx_HSCT1;
typedef struct { uint32 reserved; } Ifx_HSPHY;
typedef struct { uint32 reserved; } Ifx_HSPHY_CRPARA;
typedef struct { uint32 reserved; } Ifx_HSSL0;
typedef struct { uint32 reserved; } Ifx_HSSL1;
typedef struct { uint32 reserved; } Ifx_I2C0;
typedef struct { uint32 reserved; } Ifx_I2C1;
typedef struct { uint32 reserved; } Ifx_I2C2;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_LETH0;
typedef struct { uint32 reserved; } Ifx_LLI0;
typedef struct { uint32 reserved; } Ifx_LMU0;
typedef struct { uint32 reserved; } Ifx_LMU1;
typedef struct { uint32 reserved; } Ifx_LMU2;
typedef struct { uint32 reserved; } Ifx_LMU3;
typedef struct { uint32 reserved; } Ifx_LMU4;
typedef struct { uint32 reserved; } Ifx_LMU5;
typedef struct { uint32 reserved; } Ifx_LMU6;
typedef struct { uint32 reserved; } Ifx_LMU7;
typedef struct { uint32 reserved; } Ifx_LMU8;
typedef struct { uint32 reserved; } Ifx_LMU9;
typedef struct { uint32 reserved; } Ifx_MCDS2P;
typedef struct { uint32 reserved; } Ifx_MCDS4P;
typedef struct { uint32 reserved; } Ifx_MSC0;
typedef struct { uint32 reserved; } Ifx_PCIE0_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE0_DSP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE1_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE1_DSP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE0_USP;
typedef struct { uint32 reserved; } Ifx_PCIE0_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE1_USP;
typedef struct { uint32 reserved; } Ifx_PCIE1_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PFRWB0A;
typedef struct { uint32 reserved; } Ifx_PFRWB0B;
typedef struct { uint32 reserved; } Ifx_PFRWB1A;
typedef struct { uint32 reserved; } Ifx_PFRWB1B;
typedef struct { uint32 reserved; } Ifx_PFRWB2A;
typedef struct { uint32 reserved; } Ifx_PFRWB2B;
typedef struct { uint32 reserved; } Ifx_PFRWB3A;
typedef struct { uint32 reserved; } Ifx_PFRWB3B;
typedef struct { uint32 reserved; } Ifx_PFRWB4A;
typedef struct { uint32 reserved; } Ifx_PFRWB4B;
typedef struct { uint32 reserved; } Ifx_PFRWB5A;
typedef struct { uint32 reserved; } Ifx_PFRWB5B;
typedef struct { uint32 reserved; } Ifx_PFRWBCS;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_P30;
typedef struct { uint32 reserved; } Ifx_P31;
typedef struct { uint32 reserved; } Ifx_P32;
typedef struct { uint32 reserved; } Ifx_P33;
typedef struct { uint32 reserved; } Ifx_P34;
typedef struct { uint32 reserved; } Ifx_P35;
typedef struct { uint32 reserved; } Ifx_PPU;
typedef struct { uint32 reserved; } Ifx_PPU_STUDMI;
typedef struct { uint32 reserved; } Ifx_PPU_DEBUG;
typedef struct { uint32 reserved; } Ifx_PPU_SM;
typedef struct { uint32 reserved; } Ifx_PPU_APU;
typedef struct { uint32 reserved; } Ifx_PPU_CSMAP;
typedef struct { uint32 reserved; } Ifx_PPU_VMEMAP;
typedef struct { uint32 reserved; } Ifx_PSI5S0;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI0;
typedef struct { uint32 reserved; } Ifx_QSPI1;
typedef struct { uint32 reserved; } Ifx_QSPI2;
typedef struct { uint32 reserved; } Ifx_QSPI3;
typedef struct { uint32 reserved; } Ifx_QSPI4;
typedef struct { uint32 reserved; } Ifx_QSPI5;
typedef struct { uint32 reserved; } Ifx_QSPI6;
typedef struct { uint32 reserved; } Ifx_QSPI7;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SDMMC0;
typedef struct { uint32 reserved; } Ifx_SENT0;
typedef struct { uint32 reserved; } Ifx_SENT1;
typedef struct { uint32 reserved; } Ifx_SMM;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SMUSTDBY;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_TRIF;
typedef struct { uint32 reserved; } Ifx_TRI;
typedef struct { uint32 reserved; } Ifx_VMT0;
typedef struct { uint32 reserved; } Ifx_VMT1;
typedef struct { uint32 reserved; } Ifx_VMT2;
typedef struct { uint32 reserved; } Ifx_VMT3;
typedef struct { uint32 reserved; } Ifx_VMT4;
typedef struct { uint32 reserved; } Ifx_VMT5;
typedef struct { uint32 reserved; } Ifx_VMT6;
typedef struct { uint32 reserved; } Ifx_VTMON;
typedef struct { uint32 reserved; } Ifx_WTU;
typedef struct { uint32 reserved; } Ifx_XSPI0;

/* Extern MODULE_* instances */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P MODULE_P00;
extern Ifx_ADC MODULE_ADC;
extern Ifx_ASCLIN0 MODULE_ASCLIN0;
extern Ifx_ASCLIN1 MODULE_ASCLIN1;
extern Ifx_ASCLIN2 MODULE_ASCLIN2;
extern Ifx_ASCLIN3 MODULE_ASCLIN3;
extern Ifx_ASCLIN4 MODULE_ASCLIN4;
extern Ifx_ASCLIN5 MODULE_ASCLIN5;
extern Ifx_ASCLIN6 MODULE_ASCLIN6;
extern Ifx_ASCLIN7 MODULE_ASCLIN7;
extern Ifx_ASCLIN8 MODULE_ASCLIN8;
extern Ifx_ASCLIN9 MODULE_ASCLIN9;
extern Ifx_ASCLIN10 MODULE_ASCLIN10;
extern Ifx_ASCLIN11 MODULE_ASCLIN11;
extern Ifx_ASCLIN12 MODULE_ASCLIN12;
extern Ifx_ASCLIN13 MODULE_ASCLIN13;
extern Ifx_ASCLIN14 MODULE_ASCLIN14;
extern Ifx_ASCLIN15 MODULE_ASCLIN15;
extern Ifx_ASCLIN16 MODULE_ASCLIN16;
extern Ifx_ASCLIN17 MODULE_ASCLIN17;
extern Ifx_ASCLIN18 MODULE_ASCLIN18;
extern Ifx_ASCLIN19 MODULE_ASCLIN19;
extern Ifx_ASCLIN20 MODULE_ASCLIN20;
extern Ifx_ASCLIN21 MODULE_ASCLIN21;
extern Ifx_ASCLIN22 MODULE_ASCLIN22;
extern Ifx_ASCLIN23 MODULE_ASCLIN23;
extern Ifx_ASCLIN24 MODULE_ASCLIN24;
extern Ifx_ASCLIN25 MODULE_ASCLIN25;
extern Ifx_ASCLIN26 MODULE_ASCLIN26;
extern Ifx_ASCLIN27 MODULE_ASCLIN27;
extern Ifx_TBCU MODULE_TBCU;
extern Ifx_CSBCU MODULE_CSBCU;
extern Ifx_SBCU MODULE_SBCU;
extern Ifx_COMBCU MODULE_COMBCU;
extern Ifx_CANXL0 MODULE_CANXL0;
extern Ifx_CANXL0_RAM MODULE_CANXL0_RAM;
extern Ifx_CAN0 MODULE_CAN0;
extern Ifx_CAN0_RAM MODULE_CAN0_RAM;
extern Ifx_CAN1 MODULE_CAN1;
extern Ifx_CAN1_RAM MODULE_CAN1_RAM;
extern Ifx_CAN2 MODULE_CAN2;
extern Ifx_CAN2_RAM MODULE_CAN2_RAM;
extern Ifx_CAN3 MODULE_CAN3;
extern Ifx_CAN3_RAM MODULE_CAN3_RAM;
extern Ifx_CAN4 MODULE_CAN4;
extern Ifx_CAN4_RAM MODULE_CAN4_RAM;
extern Ifx_CBS MODULE_CBS;
extern Ifx_CLOCK MODULE_CLOCK;
extern Ifx_CPU_CFI0 MODULE_CPU_CFI0;
extern Ifx_CPU_CFI1 MODULE_CPU_CFI1;
extern Ifx_CPU_CFI2 MODULE_CPU_CFI2;
extern Ifx_CPU_CFI3 MODULE_CPU_CFI3;
extern Ifx_CPU_CFI4 MODULE_CPU_CFI4;
extern Ifx_CPU_CFI5 MODULE_CPU_CFI5;
extern Ifx_CPU_CFICS MODULE_CPU_CFICS;
extern Ifx_CPU0 MODULE_CPU0;
extern Ifx_CPU1 MODULE_CPU1;
extern Ifx_CPU2 MODULE_CPU2;
extern Ifx_CPU3 MODULE_CPU3;
extern Ifx_CPU4 MODULE_CPU4;
extern Ifx_CPU5 MODULE_CPU5;
extern Ifx_CPUCS MODULE_CPUCS;
extern Ifx_DMA0 MODULE_DMA0;
extern Ifx_DMA1 MODULE_DMA1;
extern Ifx_DMU MODULE_DMU;
extern Ifx_DOM0 MODULE_DOM0;
extern Ifx_DOM1 MODULE_DOM1;
extern Ifx_DOM3 MODULE_DOM3;
extern Ifx_DOM6 MODULE_DOM6;
extern Ifx_DOM7 MODULE_DOM7;
extern Ifx_DOM4 MODULE_DOM4;
extern Ifx_DOM2 MODULE_DOM2;
extern Ifx_DOM5 MODULE_DOM5;
extern Ifx_DRE MODULE_DRE;
extern Ifx_ERAY0 MODULE_ERAY0;
extern Ifx_ERAY1 MODULE_ERAY1;
extern Ifx_FCE MODULE_FCE;
extern Ifx_FSI_CSRM MODULE_FSI_CSRM;
extern Ifx_FSI_HOST MODULE_FSI_HOST;
extern Ifx_GETH0 MODULE_GETH0;
extern Ifx_HSCT0 MODULE_HSCT0;
extern Ifx_HSCT1 MODULE_HSCT1;
extern Ifx_HSPHY MODULE_HSPHY;
extern Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA;
extern Ifx_HSSL0 MODULE_HSSL0;
extern Ifx_HSSL1 MODULE_HSSL1;
extern Ifx_I2C0 MODULE_I2C0;
extern Ifx_I2C1 MODULE_I2C1;
extern Ifx_I2C2 MODULE_I2C2;
extern Ifx_INT MODULE_INT;
extern Ifx_LETH0 MODULE_LETH0;
extern Ifx_LLI0 MODULE_LLI0;
extern Ifx_LMU0 MODULE_LMU0;
extern Ifx_LMU1 MODULE_LMU1;
extern Ifx_LMU2 MODULE_LMU2;
extern Ifx_LMU3 MODULE_LMU3;
extern Ifx_LMU4 MODULE_LMU4;
extern Ifx_LMU5 MODULE_LMU5;
extern Ifx_LMU6 MODULE_LMU6;
extern Ifx_LMU7 MODULE_LMU7;
extern Ifx_LMU8 MODULE_LMU8;
extern Ifx_LMU9 MODULE_LMU9;
extern Ifx_MCDS2P MODULE_MCDS2P;
extern Ifx_MCDS4P MODULE_MCDS4P;
extern Ifx_MSC0 MODULE_MSC0;
extern Ifx_PCIE0_DSP MODULE_PCIE0_DSP;
extern Ifx_PCIE0_DSP_SRI MODULE_PCIE0_DSP_SRI;
extern Ifx_PCIE1_DSP MODULE_PCIE1_DSP;
extern Ifx_PCIE1_DSP_SRI MODULE_PCIE1_DSP_SRI;
extern Ifx_PCIE0_USP MODULE_PCIE0_USP;
extern Ifx_PCIE0_USP_SRI MODULE_PCIE0_USP_SRI;
extern Ifx_PCIE1_USP MODULE_PCIE1_USP;
extern Ifx_PCIE1_USP_SRI MODULE_PCIE1_USP_SRI;
extern Ifx_PFRWB0A MODULE_PFRWB0A;
extern Ifx_PFRWB0B MODULE_PFRWB0B;
extern Ifx_PFRWB1A MODULE_PFRWB1A;
extern Ifx_PFRWB1B MODULE_PFRWB1B;
extern Ifx_PFRWB2A MODULE_PFRWB2A;
extern Ifx_PFRWB2B MODULE_PFRWB2B;
extern Ifx_PFRWB3A MODULE_PFRWB3A;
extern Ifx_PFRWB3B MODULE_PFRWB3B;
extern Ifx_PFRWB4A MODULE_PFRWB4A;
extern Ifx_PFRWB4B MODULE_PFRWB4B;
extern Ifx_PFRWB5A MODULE_PFRWB5A;
extern Ifx_PFRWB5B MODULE_PFRWB5B;
extern Ifx_PFRWBCS MODULE_PFRWBCS;
extern Ifx_PMS MODULE_PMS;
extern Ifx_P MODULE_P01;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P03;
extern Ifx_P MODULE_P04;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P16;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P30;
extern Ifx_P MODULE_P31;
extern Ifx_P MODULE_P32;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P35;
extern Ifx_P MODULE_P40;
extern Ifx_P MODULE_P41; /* required by port modules */
extern Ifx_PPU MODULE_PPU;
extern Ifx_PPU_STUDMI MODULE_PPU_STUDMI;
extern Ifx_PPU_DEBUG MODULE_PPU_DEBUG;
extern Ifx_PPU_SM MODULE_PPU_SM;
extern Ifx_PPU_APU MODULE_PPU_APU;
extern Ifx_PPU_CSMAP MODULE_PPU_CSMAP;
extern Ifx_PPU_VMEMAP MODULE_PPU_VMEMAP;
extern Ifx_PSI5S0 MODULE_PSI5S0;
extern Ifx_PSI5 MODULE_PSI5;
extern Ifx_QSPI0 MODULE_QSPI0;
extern Ifx_QSPI1 MODULE_QSPI1;
extern Ifx_QSPI2 MODULE_QSPI2;
extern Ifx_QSPI3 MODULE_QSPI3;
extern Ifx_QSPI4 MODULE_QSPI4;
extern Ifx_QSPI5 MODULE_QSPI5;
extern Ifx_QSPI6 MODULE_QSPI6;
extern Ifx_QSPI7 MODULE_QSPI7;
extern Ifx_SCU MODULE_SCU;
extern Ifx_SDMMC0 MODULE_SDMMC0;
extern Ifx_SENT0 MODULE_SENT0;
extern Ifx_SENT1 MODULE_SENT1;
extern Ifx_SMM MODULE_SMM;
extern Ifx_SMU MODULE_SMU;
extern Ifx_SMUSTDBY MODULE_SMUSTDBY;
extern Ifx_SRC MODULE_SRC;
extern Ifx_TRIF MODULE_TRIF;
extern Ifx_TRI MODULE_TRI;
extern Ifx_VMT0 MODULE_VMT0;
extern Ifx_VMT1 MODULE_VMT1;
extern Ifx_VMT2 MODULE_VMT2;
extern Ifx_VMT3 MODULE_VMT3;
extern Ifx_VMT4 MODULE_VMT4;
extern Ifx_VMT5 MODULE_VMT5;
extern Ifx_VMT6 MODULE_VMT6;
extern Ifx_VTMON MODULE_VTMON;
extern Ifx_WTU MODULE_WTU;
extern Ifx_XSPI0 MODULE_XSPI0;

/* Spy API state (extern) */
#define MOCK_MAX_CHANNELS 16

/* Call counters for stubbed functions */
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_setClkCount_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
/* Additional CMU helpers used by enable-guard pattern */
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;

/* Return-value control for non-void stubs */
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* Value-capture spy fields */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Getters for call counters */
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkCount_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
