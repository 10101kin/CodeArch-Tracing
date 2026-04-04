/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTMATOMPWM_H
#define MOCK_EGTMATOMPWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned long uint32;
typedef signed long sint32;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char uint8;
typedef unsigned char boolean;
typedef uint32 Ifx_Priority;
typedef uint32 Ifx_UReg_32Bit;

/* Macros */
#ifndef TRUE
#define TRUE ((boolean)1)
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
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_cpu6 = 6,
    IfxSrc_Tos_cpu7 = 7,
    IfxSrc_Tos_dma  = 8
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;

/* Common SFR module typedef stubs (minimal) */
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_DRE;
typedef struct { uint32 reserved; } Ifx_PCIE_USP;
typedef struct { uint32 reserved; } Ifx_PCIE_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE_DSP_SRI;
typedef struct { uint32 reserved; } Ifx_PFRWB;
typedef struct { uint32 reserved; } Ifx_PFRWBCS;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_TRIF;
typedef struct { uint32 reserved; } Ifx_VTMON;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_HSPHY;
typedef struct { uint32 reserved; } Ifx_HSPHY_CRPARA;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CAN_RAM;
typedef struct { uint32 reserved; } Ifx_HSCT;
typedef struct { uint32 reserved; } Ifx_SMUSTDBY;
typedef struct { uint32 reserved; } Ifx_WTU;
typedef struct { uint32 reserved; } Ifx_MSC;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_LETH;
typedef struct { uint32 reserved; } Ifx_MCDS4P2P;
typedef struct { uint32 reserved; } Ifx_XSPI;
typedef struct { uint32 reserved; } Ifx_CANXL;
typedef struct { uint32 reserved; } Ifx_CANXL_RAM;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_TRI;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_QSPI;
typedef struct { uint32 reserved; } Ifx_DOM;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_FSI_CSRM;
typedef struct { uint32 reserved; } Ifx_FSI_HOST;
typedef struct { uint32 reserved; } Ifx_CPU;
typedef struct { uint32 reserved; } Ifx_CPUCS;
typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_CLOCK;
typedef struct { uint32 reserved; } Ifx_I2C;
typedef struct { uint32 reserved; } Ifx_SDMMC;
typedef struct { uint32 reserved; } Ifx_ERAY;
typedef struct { uint32 reserved; } Ifx_HSSL;
typedef struct { uint32 reserved; } Ifx_SMM;
typedef struct { uint32 reserved; } Ifx_EGTM_REG;
typedef struct { uint32 reserved; } Ifx_PPU;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_LLI;
typedef struct { uint32 reserved; } Ifx_CPU_CFI;
typedef struct { uint32 reserved; } Ifx_CPU_CFICS;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_BCU;
typedef struct { uint32 reserved; } Ifx_LMU;
typedef struct { uint32 reserved; } Ifx_PROT;
typedef struct { uint32 reserved; } Ifx_ACCEN;
typedef struct { uint32 reserved; } Ifx_VMT;

/* Extern MODULE_* instances (declare here; defined in .c) */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P MODULE_P00;
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

/* Additional externs for SFR modules (subset covering listed modules) */
extern Ifx_ADC MODULE_ADC;
extern Ifx_DRE MODULE_DRE;
extern Ifx_PCIE_USP MODULE_PCIE0_USP;
extern Ifx_PCIE_USP_SRI MODULE_PCIE0_USP_SRI;
extern Ifx_PCIE_USP MODULE_PCIE1_USP;
extern Ifx_PCIE_USP_SRI MODULE_PCIE1_USP_SRI;
extern Ifx_PCIE_DSP MODULE_PCIE0_DSP;
extern Ifx_PCIE_DSP_SRI MODULE_PCIE0_DSP_SRI;
extern Ifx_PCIE_DSP MODULE_PCIE1_DSP;
extern Ifx_PCIE_DSP_SRI MODULE_PCIE1_DSP_SRI;
extern Ifx_PFRWB MODULE_PFRWB0A;
extern Ifx_PFRWB MODULE_PFRWB0B;
extern Ifx_PFRWB MODULE_PFRWB1A;
extern Ifx_PFRWB MODULE_PFRWB1B;
extern Ifx_PFRWB MODULE_PFRWB2A;
extern Ifx_PFRWB MODULE_PFRWB2B;
extern Ifx_PFRWB MODULE_PFRWB3A;
extern Ifx_PFRWB MODULE_PFRWB3B;
extern Ifx_PFRWB MODULE_PFRWB4A;
extern Ifx_PFRWB MODULE_PFRWB4B;
extern Ifx_PFRWB MODULE_PFRWB5A;
extern Ifx_PFRWB MODULE_PFRWB5B;
extern Ifx_PFRWBCS MODULE_PFRWBCS;
extern Ifx_INT MODULE_INT;
extern Ifx_TRIF MODULE_TRIF;
extern Ifx_VTMON MODULE_VTMON;
extern Ifx_SRC MODULE_SRC;
extern Ifx_HSPHY MODULE_HSPHY;
extern Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA;
extern Ifx_SCU MODULE_SCU;
extern Ifx_SENT MODULE_SENT0;
extern Ifx_SENT MODULE_SENT1;
extern Ifx_CAN MODULE_CAN0;
extern Ifx_CAN_RAM MODULE_CAN0_RAM;
extern Ifx_CAN MODULE_CAN1;
extern Ifx_CAN_RAM MODULE_CAN1_RAM;
extern Ifx_CAN MODULE_CAN2;
extern Ifx_CAN_RAM MODULE_CAN2_RAM;
extern Ifx_CAN MODULE_CAN3;
extern Ifx_CAN_RAM MODULE_CAN3_RAM;
extern Ifx_CAN MODULE_CAN4;
extern Ifx_CAN_RAM MODULE_CAN4_RAM;
extern Ifx_HSCT MODULE_HSCT0;
extern Ifx_HSCT MODULE_HSCT1;
extern Ifx_SMUSTDBY MODULE_SMUSTDBY;
extern Ifx_WTU MODULE_WTU;
extern Ifx_MSC MODULE_MSC0;
extern Ifx_FCE MODULE_FCE;
extern Ifx_LETH MODULE_LETH0;
extern Ifx_MCDS4P2P MODULE_MCDS2P;
extern Ifx_MCDS4P2P MODULE_MCDS4P;
extern Ifx_XSPI MODULE_XSPI0;
extern Ifx_CANXL MODULE_CANXL0;
extern Ifx_CANXL_RAM MODULE_CANXL0_RAM;
extern Ifx_CBS MODULE_CBS;
extern Ifx_TRI MODULE_TRI;
extern Ifx_SMU MODULE_SMU;
extern Ifx_QSPI MODULE_QSPI0;
extern Ifx_QSPI MODULE_QSPI1;
extern Ifx_QSPI MODULE_QSPI2;
extern Ifx_QSPI MODULE_QSPI3;
extern Ifx_QSPI MODULE_QSPI4;
extern Ifx_QSPI MODULE_QSPI5;
extern Ifx_QSPI MODULE_QSPI6;
extern Ifx_QSPI MODULE_QSPI7;
extern Ifx_DOM MODULE_DOM0;
extern Ifx_DOM MODULE_DOM1;
extern Ifx_DOM MODULE_DOM3;
extern Ifx_DOM MODULE_DOM6;
extern Ifx_DOM MODULE_DOM7;
extern Ifx_DOM MODULE_DOM4;
extern Ifx_DOM MODULE_DOM2;
extern Ifx_DOM MODULE_DOM5;
extern Ifx_PSI5 MODULE_PSI5;
extern Ifx_FSI_CSRM MODULE_FSI_CSRM;
extern Ifx_FSI_HOST MODULE_FSI_HOST;
extern Ifx_CPU MODULE_CPU0;
extern Ifx_CPU MODULE_CPU1;
extern Ifx_CPU MODULE_CPU2;
extern Ifx_CPU MODULE_CPU3;
extern Ifx_CPU MODULE_CPU4;
extern Ifx_CPU MODULE_CPU5;
extern Ifx_CPUCS MODULE_CPUCS;
extern Ifx_ASCLIN MODULE_ASCLIN0;
extern Ifx_ASCLIN MODULE_ASCLIN1;
extern Ifx_ASCLIN MODULE_ASCLIN2;
extern Ifx_ASCLIN MODULE_ASCLIN3;
extern Ifx_ASCLIN MODULE_ASCLIN4;
extern Ifx_ASCLIN MODULE_ASCLIN5;
extern Ifx_ASCLIN MODULE_ASCLIN6;
extern Ifx_ASCLIN MODULE_ASCLIN7;
extern Ifx_ASCLIN MODULE_ASCLIN8;
extern Ifx_ASCLIN MODULE_ASCLIN9;
extern Ifx_ASCLIN MODULE_ASCLIN10;
extern Ifx_ASCLIN MODULE_ASCLIN11;
extern Ifx_ASCLIN MODULE_ASCLIN12;
extern Ifx_ASCLIN MODULE_ASCLIN13;
extern Ifx_ASCLIN MODULE_ASCLIN14;
extern Ifx_ASCLIN MODULE_ASCLIN15;
extern Ifx_ASCLIN MODULE_ASCLIN16;
extern Ifx_ASCLIN MODULE_ASCLIN17;
extern Ifx_ASCLIN MODULE_ASCLIN18;
extern Ifx_ASCLIN MODULE_ASCLIN19;
extern Ifx_ASCLIN MODULE_ASCLIN20;
extern Ifx_ASCLIN MODULE_ASCLIN21;
extern Ifx_ASCLIN MODULE_ASCLIN22;
extern Ifx_ASCLIN MODULE_ASCLIN23;
extern Ifx_ASCLIN MODULE_ASCLIN24;
extern Ifx_ASCLIN MODULE_ASCLIN25;
extern Ifx_ASCLIN MODULE_ASCLIN26;
extern Ifx_ASCLIN MODULE_ASCLIN27;
extern Ifx_CLOCK MODULE_CLOCK;
extern Ifx_I2C MODULE_I2C0;
extern Ifx_I2C MODULE_I2C1;
extern Ifx_I2C MODULE_I2C2;
extern Ifx_SDMMC MODULE_SDMMC0;
extern Ifx_ERAY MODULE_ERAY0;
extern Ifx_ERAY MODULE_ERAY1;
extern Ifx_HSSL MODULE_HSSL0;
extern Ifx_HSSL MODULE_HSSL1;
extern Ifx_SMM MODULE_SMM;
extern Ifx_EGTM_REG MODULE_EGTM; /* already above */
extern Ifx_PPU MODULE_PPU;
extern Ifx_PPU MODULE_PPU_STUDMI;
extern Ifx_PPU MODULE_PPU_DEBUG;
extern Ifx_PPU MODULE_PPU_SM;
extern Ifx_PPU MODULE_PPU_APU;
extern Ifx_PPU MODULE_PPU_CSMAP;
extern Ifx_PPU MODULE_PPU_VMEMAP;
extern Ifx_DMU MODULE_DMU;
extern Ifx_PMS MODULE_PMS;
extern Ifx_LLI MODULE_LLI0;
extern Ifx_CPU_CFI MODULE_CPU_CFI0;
extern Ifx_CPU_CFI MODULE_CPU_CFI1;
extern Ifx_CPU_CFI MODULE_CPU_CFI2;
extern Ifx_CPU_CFI MODULE_CPU_CFI3;
extern Ifx_CPU_CFI MODULE_CPU_CFI4;
extern Ifx_CPU_CFI MODULE_CPU_CFI5;
extern Ifx_CPU_CFICS MODULE_CPU_CFICS;
extern Ifx_DMA MODULE_DMA0;
extern Ifx_DMA MODULE_DMA1;
extern Ifx_GETH MODULE_GETH0;
extern Ifx_PSI5S MODULE_PSI5S0;
extern Ifx_BCU MODULE_TBCU;
extern Ifx_BCU MODULE_CSBCU;
extern Ifx_BCU MODULE_SBCU;
extern Ifx_BCU MODULE_COMBCU;
extern Ifx_VMT MODULE_VMT0;
extern Ifx_VMT MODULE_VMT1;
extern Ifx_VMT MODULE_VMT2;
extern Ifx_VMT MODULE_VMT3;
extern Ifx_VMT MODULE_VMT4;
extern Ifx_VMT MODULE_VMT5;
extern Ifx_VMT MODULE_VMT6;
extern Ifx_LMU MODULE_LMU0;
extern Ifx_LMU MODULE_LMU1;
extern Ifx_LMU MODULE_LMU2;
extern Ifx_LMU MODULE_LMU3;
extern Ifx_LMU MODULE_LMU4;
extern Ifx_LMU MODULE_LMU5;
extern Ifx_LMU MODULE_LMU6;
extern Ifx_LMU MODULE_LMU7;
extern Ifx_LMU MODULE_LMU8;
extern Ifx_LMU MODULE_LMU9;
extern Ifx_PROT MODULE_PROT;
extern Ifx_ACCEN MODULE_ACCEN;

/* Spy API */
#define MOCK_MAX_CHANNELS 16

/* Call counters (extern) */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

/* Return-value control (extern) */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* Value-capture spy fields */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_egtmatompwm_reset(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTMATOMPWM_H */
