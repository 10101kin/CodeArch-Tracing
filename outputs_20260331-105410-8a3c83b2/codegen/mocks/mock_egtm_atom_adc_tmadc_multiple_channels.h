/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef float            float32;
typedef unsigned char    uint8;
typedef signed char      sint8;
typedef unsigned short   uint16;
typedef signed short     sint16;
typedef unsigned int     uint32;
typedef signed int       sint32;
typedef unsigned char    boolean;
typedef uint32           Ifx_Priority;

typedef uint32           Ifx_UReg_32Bit; /* SFR 32-bit register type used by iLLD */

/* Macros */
#ifndef TRUE
# define TRUE  ((boolean)1)
#endif
#ifndef FALSE
# define FALSE ((boolean)0)
#endif
#ifndef NULL_PTR
# define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
# define IFX_STATIC static
#endif
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_dma  = 2
} IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* EGTM / Port core types */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;

extern Ifx_P MODULE_P00;  extern Ifx_P MODULE_P01;  extern Ifx_P MODULE_P02;  extern Ifx_P MODULE_P03;  extern Ifx_P MODULE_P04;
extern Ifx_P MODULE_P10;  extern Ifx_P MODULE_P13;  extern Ifx_P MODULE_P14;  extern Ifx_P MODULE_P15;  extern Ifx_P MODULE_P16;
extern Ifx_P MODULE_P20;  extern Ifx_P MODULE_P21;  extern Ifx_P MODULE_P22;  extern Ifx_P MODULE_P23;  extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P30;  extern Ifx_P MODULE_P31;  extern Ifx_P MODULE_P32;  extern Ifx_P MODULE_P33;  extern Ifx_P MODULE_P34;  extern Ifx_P MODULE_P35;  extern Ifx_P MODULE_P40;

/* SFR Register Modules (stubs) */
typedef struct { uint32 reserved; } Ifx_ADC;               extern Ifx_ADC MODULE_ADC;
typedef struct { uint32 reserved; } Ifx_DRE;               extern Ifx_DRE MODULE_DRE;

/* PCIE */
typedef struct { uint32 reserved; } Ifx_PCIE0_USP;         extern Ifx_PCIE0_USP MODULE_PCIE0_USP;
typedef struct { uint32 reserved; } Ifx_PCIE0_USP_SRI;     extern Ifx_PCIE0_USP_SRI MODULE_PCIE0_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE1_USP;         extern Ifx_PCIE1_USP MODULE_PCIE1_USP;
typedef struct { uint32 reserved; } Ifx_PCIE1_USP_SRI;     extern Ifx_PCIE1_USP_SRI MODULE_PCIE1_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE0_DSP;         extern Ifx_PCIE0_DSP MODULE_PCIE0_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE0_DSP_SRI;     extern Ifx_PCIE0_DSP_SRI MODULE_PCIE0_DSP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE1_DSP;         extern Ifx_PCIE1_DSP MODULE_PCIE1_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE1_DSP_SRI;     extern Ifx_PCIE1_DSP_SRI MODULE_PCIE1_DSP_SRI;

/* PFRWB */
typedef struct { uint32 reserved; } Ifx_PFRWB;             extern Ifx_PFRWB MODULE_PFRWB0A; extern Ifx_PFRWB MODULE_PFRWB0B; extern Ifx_PFRWB MODULE_PFRWB1A; extern Ifx_PFRWB MODULE_PFRWB1B; extern Ifx_PFRWB MODULE_PFRWB2A; extern Ifx_PFRWB MODULE_PFRWB2B; extern Ifx_PFRWB MODULE_PFRWB3A; extern Ifx_PFRWB MODULE_PFRWB3B; extern Ifx_PFRWB MODULE_PFRWB4A; extern Ifx_PFRWB MODULE_PFRWB4B; extern Ifx_PFRWB MODULE_PFRWB5A; extern Ifx_PFRWB MODULE_PFRWB5B;
typedef struct { uint32 reserved; } Ifx_PFRWBCS;           extern Ifx_PFRWBCS MODULE_PFRWBCS;

/* Others */
typedef struct { uint32 reserved; } Ifx_INT;               extern Ifx_INT MODULE_INT;
typedef struct { uint32 reserved; } Ifx_TRIF;              extern Ifx_TRIF MODULE_TRIF;
typedef struct { uint32 reserved; } Ifx_VTMON;             extern Ifx_VTMON MODULE_VTMON;
typedef struct { uint32 reserved; } Ifx_SRC;               extern Ifx_SRC MODULE_SRC;
typedef struct { uint32 reserved; } Ifx_HSPHY;             extern Ifx_HSPHY MODULE_HSPHY;
typedef struct { uint32 reserved; } Ifx_HSPHY_CRPARA;      extern Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA;
typedef struct { uint32 reserved; } Ifx_SCU;               extern Ifx_SCU MODULE_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;              extern Ifx_SENT MODULE_SENT0; extern Ifx_SENT MODULE_SENT1;
typedef struct { uint32 reserved; } Ifx_CAN;               extern Ifx_CAN MODULE_CAN0; extern Ifx_CAN MODULE_CAN1; extern Ifx_CAN MODULE_CAN2; extern Ifx_CAN MODULE_CAN3; extern Ifx_CAN MODULE_CAN4;
typedef struct { uint32 reserved; } Ifx_CAN_RAM;           extern Ifx_CAN_RAM MODULE_CAN0_RAM; extern Ifx_CAN_RAM MODULE_CAN1_RAM; extern Ifx_CAN_RAM MODULE_CAN2_RAM; extern Ifx_CAN_RAM MODULE_CAN3_RAM; extern Ifx_CAN_RAM MODULE_CAN4_RAM;
typedef struct { uint32 reserved; } Ifx_HSCT;              extern Ifx_HSCT MODULE_HSCT0; extern Ifx_HSCT MODULE_HSCT1;
typedef struct { uint32 reserved; } Ifx_SMUSTDBY;          extern Ifx_SMUSTDBY MODULE_SMUSTDBY;
typedef struct { uint32 reserved; } Ifx_WTU;               extern Ifx_WTU MODULE_WTU;
typedef struct { uint32 reserved; } Ifx_MSC;               extern Ifx_MSC MODULE_MSC0;
typedef struct { uint32 reserved; } Ifx_FCE;               extern Ifx_FCE MODULE_FCE;
typedef struct { uint32 reserved; } Ifx_LETH;              extern Ifx_LETH MODULE_LETH0;
typedef struct { uint32 reserved; } Ifx_MCDS2P;            extern Ifx_MCDS2P MODULE_MCDS2P;
typedef struct { uint32 reserved; } Ifx_MCDS4P;            extern Ifx_MCDS4P MODULE_MCDS4P;
typedef struct { uint32 reserved; } Ifx_XSPI;              extern Ifx_XSPI MODULE_XSPI0;
typedef struct { uint32 reserved; } Ifx_CANXL;             extern Ifx_CANXL MODULE_CANXL0;
typedef struct { uint32 reserved; } Ifx_CANXL_RAM;         extern Ifx_CANXL_RAM MODULE_CANXL0_RAM;
typedef struct { uint32 reserved; } Ifx_CBS;               extern Ifx_CBS MODULE_CBS;
typedef struct { uint32 reserved; } Ifx_TRI;               extern Ifx_TRI MODULE_TRI;
typedef struct { uint32 reserved; } Ifx_SMU;               extern Ifx_SMU MODULE_SMU;
typedef struct { uint32 reserved; } Ifx_QSPI;              extern Ifx_QSPI MODULE_QSPI0; extern Ifx_QSPI MODULE_QSPI1; extern Ifx_QSPI MODULE_QSPI2; extern Ifx_QSPI MODULE_QSPI3; extern Ifx_QSPI MODULE_QSPI4; extern Ifx_QSPI MODULE_QSPI5; extern Ifx_QSPI MODULE_QSPI6; extern Ifx_QSPI MODULE_QSPI7;
typedef struct { uint32 reserved; } Ifx_DOM;               extern Ifx_DOM MODULE_DOM0; extern Ifx_DOM MODULE_DOM1; extern Ifx_DOM MODULE_DOM2; extern Ifx_DOM MODULE_DOM3; extern Ifx_DOM MODULE_DOM4; extern Ifx_DOM MODULE_DOM5; extern Ifx_DOM MODULE_DOM6; extern Ifx_DOM MODULE_DOM7;
typedef struct { uint32 reserved; } Ifx_PSI5;              extern Ifx_PSI5 MODULE_PSI5;
typedef struct { uint32 reserved; } Ifx_FSI_CSRM;          extern Ifx_FSI_CSRM MODULE_FSI_CSRM;
typedef struct { uint32 reserved; } Ifx_FSI_HOST;          extern Ifx_FSI_HOST MODULE_FSI_HOST;
typedef struct { uint32 reserved; } Ifx_CPU;               extern Ifx_CPU MODULE_CPU0; extern Ifx_CPU MODULE_CPU1; extern Ifx_CPU MODULE_CPU2; extern Ifx_CPU MODULE_CPU3; extern Ifx_CPU MODULE_CPU4; extern Ifx_CPU MODULE_CPU5;
typedef struct { uint32 reserved; } Ifx_CPUCS;             extern Ifx_CPUCS MODULE_CPUCS;
typedef struct { uint32 reserved; } Ifx_ASCLIN;            extern Ifx_ASCLIN MODULE_ASCLIN0; extern Ifx_ASCLIN MODULE_ASCLIN1; extern Ifx_ASCLIN MODULE_ASCLIN2; extern Ifx_ASCLIN MODULE_ASCLIN3; extern Ifx_ASCLIN MODULE_ASCLIN4; extern Ifx_ASCLIN MODULE_ASCLIN5; extern Ifx_ASCLIN MODULE_ASCLIN6; extern Ifx_ASCLIN MODULE_ASCLIN7; extern Ifx_ASCLIN MODULE_ASCLIN8; extern Ifx_ASCLIN MODULE_ASCLIN9; extern Ifx_ASCLIN MODULE_ASCLIN10; extern Ifx_ASCLIN MODULE_ASCLIN11; extern Ifx_ASCLIN MODULE_ASCLIN12; extern Ifx_ASCLIN MODULE_ASCLIN13; extern Ifx_ASCLIN MODULE_ASCLIN14; extern Ifx_ASCLIN MODULE_ASCLIN15; extern Ifx_ASCLIN MODULE_ASCLIN16; extern Ifx_ASCLIN MODULE_ASCLIN17; extern Ifx_ASCLIN MODULE_ASCLIN18; extern Ifx_ASCLIN MODULE_ASCLIN19; extern Ifx_ASCLIN MODULE_ASCLIN20; extern Ifx_ASCLIN MODULE_ASCLIN21; extern Ifx_ASCLIN MODULE_ASCLIN22; extern Ifx_ASCLIN MODULE_ASCLIN23; extern Ifx_ASCLIN MODULE_ASCLIN24; extern Ifx_ASCLIN MODULE_ASCLIN25; extern Ifx_ASCLIN MODULE_ASCLIN26; extern Ifx_ASCLIN MODULE_ASCLIN27;
typedef struct { uint32 reserved; } Ifx_CLOCK;             extern Ifx_CLOCK MODULE_CLOCK;
typedef struct { uint32 reserved; } Ifx_I2C;               extern Ifx_I2C MODULE_I2C0; extern Ifx_I2C MODULE_I2C1; extern Ifx_I2C MODULE_I2C2;
typedef struct { uint32 reserved; } Ifx_SDMMC;             extern Ifx_SDMMC MODULE_SDMMC0;
typedef struct { uint32 reserved; } Ifx_ERAY;              extern Ifx_ERAY MODULE_ERAY0; extern Ifx_ERAY MODULE_ERAY1;
typedef struct { uint32 reserved; } Ifx_HSSL;              extern Ifx_HSSL MODULE_HSSL0; extern Ifx_HSSL MODULE_HSSL1;
typedef struct { uint32 reserved; } Ifx_SMM;               extern Ifx_SMM MODULE_SMM;
/* EGTM already defined above */
typedef struct { uint32 reserved; } Ifx_PPU;               extern Ifx_PPU MODULE_PPU; extern Ifx_PPU MODULE_PPU_STUDMI; extern Ifx_PPU MODULE_PPU_DEBUG; extern Ifx_PPU MODULE_PPU_SM; extern Ifx_PPU MODULE_PPU_APU; extern Ifx_PPU MODULE_PPU_CSMAP; extern Ifx_PPU MODULE_PPU_VMEMAP;
typedef struct { uint32 reserved; } Ifx_DMU;               extern Ifx_DMU MODULE_DMU;
typedef struct { uint32 reserved; } Ifx_PMS;               extern Ifx_PMS MODULE_PMS;
typedef struct { uint32 reserved; } Ifx_LLI;               extern Ifx_LLI MODULE_LLI0;
typedef struct { uint32 reserved; } Ifx_CPU_CFI;           extern Ifx_CPU_CFI MODULE_CPU_CFI0; extern Ifx_CPU_CFI MODULE_CPU_CFI1; extern Ifx_CPU_CFI MODULE_CPU_CFI2; extern Ifx_CPU_CFI MODULE_CPU_CFI3; extern Ifx_CPU_CFI MODULE_CPU_CFI4; extern Ifx_CPU_CFI MODULE_CPU_CFI5; extern Ifx_CPU_CFI MODULE_CPU_CFICS;
typedef struct { uint32 reserved; } Ifx_DMA;               extern Ifx_DMA MODULE_DMA0; extern Ifx_DMA MODULE_DMA1;
typedef struct { uint32 reserved; } Ifx_GETH;              extern Ifx_GETH MODULE_GETH0;
typedef struct { uint32 reserved; } Ifx_PSI5S;             extern Ifx_PSI5S MODULE_PSI5S0;
typedef struct { uint32 reserved; } Ifx_TBCU;              extern Ifx_TBCU MODULE_TBCU; extern Ifx_TBCU MODULE_CSBCU; extern Ifx_TBCU MODULE_SBCU; extern Ifx_TBCU MODULE_COMBCU;
/* Port modules already defined individually above */
typedef struct { uint32 reserved; } Ifx_VMT;               extern Ifx_VMT MODULE_VMT0; extern Ifx_VMT MODULE_VMT1; extern Ifx_VMT MODULE_VMT2; extern Ifx_VMT MODULE_VMT3; extern Ifx_VMT MODULE_VMT4; extern Ifx_VMT MODULE_VMT5; extern Ifx_VMT MODULE_VMT6;
typedef struct { uint32 reserved; } Ifx_LMU;               extern Ifx_LMU MODULE_LMU0; extern Ifx_LMU MODULE_LMU1; extern Ifx_LMU MODULE_LMU2; extern Ifx_LMU MODULE_LMU3; extern Ifx_LMU MODULE_LMU4; extern Ifx_LMU MODULE_LMU5; extern Ifx_LMU MODULE_LMU6; extern Ifx_LMU MODULE_LMU7; extern Ifx_LMU MODULE_LMU8; extern Ifx_LMU MODULE_LMU9;
typedef struct { uint32 reserved; } Ifx_ACCEN;             extern Ifx_ACCEN MODULE_ACCEN;
typedef struct { uint32 reserved; } Ifx_PROT;              extern Ifx_PROT MODULE_PROT;

/* Spy API: counters, return-value controls, captured values */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm_Pwm */
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxEgtm */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;

/* IfxPort */
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);

int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_togglePin_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
