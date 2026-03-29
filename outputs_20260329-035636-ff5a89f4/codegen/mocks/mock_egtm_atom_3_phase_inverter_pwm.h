/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + spy API only
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float        float32;
typedef unsigned int uint32;
typedef int          sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short  sint16;
typedef unsigned char boolean;
typedef uint8         Ifx_Priority;

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
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum
{
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5
} IfxSrc_Tos;

/* MODULE_* register-block stubs (typedef + extern) */
/* Types */
typedef struct { uint32 reserved; } Ifx_EGTM;
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
typedef struct { uint32 reserved; } Ifx_P25;
typedef struct { uint32 reserved; } Ifx_P30;
typedef struct { uint32 reserved; } Ifx_P31;
typedef struct { uint32 reserved; } Ifx_P32;
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

/* Extern module instances */
extern Ifx_EGTM MODULE_EGTM;
/* Port modules needed as extern Ifx_P instances */
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P40;
extern Ifx_P MODULE_P41;

/* Spy API declarations */
#define MOCK_MAX_CHANNELS 16

/* Call counters */
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_setClkCount_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;

/* Return value controls */
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
extern float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Mock control API (getters + reset) */
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkCount_getCallCount(void);
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);

void mock_egtm_atom_3_phase_inverter_pwm_reset(void);


/* ── Auto-injected missing declarations ── */
/* Missing: MODULE_ADC of type Ifx_ADC */
extern Ifx_ADC MODULE_ADC;
/* Missing: MODULE_ASCLIN0 of type Ifx_ASCLIN0 */
extern Ifx_ASCLIN0 MODULE_ASCLIN0;
/* Missing: MODULE_ASCLIN1 of type Ifx_ASCLIN1 */
extern Ifx_ASCLIN1 MODULE_ASCLIN1;
/* Missing: MODULE_ASCLIN2 of type Ifx_ASCLIN2 */
extern Ifx_ASCLIN2 MODULE_ASCLIN2;
/* Missing: MODULE_ASCLIN3 of type Ifx_ASCLIN3 */
extern Ifx_ASCLIN3 MODULE_ASCLIN3;
/* Missing: MODULE_ASCLIN4 of type Ifx_ASCLIN4 */
extern Ifx_ASCLIN4 MODULE_ASCLIN4;
/* Missing: MODULE_ASCLIN5 of type Ifx_ASCLIN5 */
extern Ifx_ASCLIN5 MODULE_ASCLIN5;
/* Missing: MODULE_ASCLIN6 of type Ifx_ASCLIN6 */
extern Ifx_ASCLIN6 MODULE_ASCLIN6;
/* Missing: MODULE_ASCLIN7 of type Ifx_ASCLIN7 */
extern Ifx_ASCLIN7 MODULE_ASCLIN7;
/* Missing: MODULE_ASCLIN8 of type Ifx_ASCLIN8 */
extern Ifx_ASCLIN8 MODULE_ASCLIN8;
/* Missing: MODULE_ASCLIN9 of type Ifx_ASCLIN9 */
extern Ifx_ASCLIN9 MODULE_ASCLIN9;
/* Missing: MODULE_ASCLIN10 of type Ifx_ASCLIN10 */
extern Ifx_ASCLIN10 MODULE_ASCLIN10;
/* Missing: MODULE_ASCLIN11 of type Ifx_ASCLIN11 */
extern Ifx_ASCLIN11 MODULE_ASCLIN11;
/* Missing: MODULE_ASCLIN12 of type Ifx_ASCLIN12 */
extern Ifx_ASCLIN12 MODULE_ASCLIN12;
/* Missing: MODULE_ASCLIN13 of type Ifx_ASCLIN13 */
extern Ifx_ASCLIN13 MODULE_ASCLIN13;
/* Missing: MODULE_ASCLIN14 of type Ifx_ASCLIN14 */
extern Ifx_ASCLIN14 MODULE_ASCLIN14;
/* Missing: MODULE_ASCLIN15 of type Ifx_ASCLIN15 */
extern Ifx_ASCLIN15 MODULE_ASCLIN15;
/* Missing: MODULE_ASCLIN16 of type Ifx_ASCLIN16 */
extern Ifx_ASCLIN16 MODULE_ASCLIN16;
/* Missing: MODULE_ASCLIN17 of type Ifx_ASCLIN17 */
extern Ifx_ASCLIN17 MODULE_ASCLIN17;
/* Missing: MODULE_ASCLIN18 of type Ifx_ASCLIN18 */
extern Ifx_ASCLIN18 MODULE_ASCLIN18;
/* Missing: MODULE_ASCLIN19 of type Ifx_ASCLIN19 */
extern Ifx_ASCLIN19 MODULE_ASCLIN19;
/* Missing: MODULE_ASCLIN20 of type Ifx_ASCLIN20 */
extern Ifx_ASCLIN20 MODULE_ASCLIN20;
/* Missing: MODULE_ASCLIN21 of type Ifx_ASCLIN21 */
extern Ifx_ASCLIN21 MODULE_ASCLIN21;
/* Missing: MODULE_ASCLIN22 of type Ifx_ASCLIN22 */
extern Ifx_ASCLIN22 MODULE_ASCLIN22;
/* Missing: MODULE_ASCLIN23 of type Ifx_ASCLIN23 */
extern Ifx_ASCLIN23 MODULE_ASCLIN23;
/* Missing: MODULE_ASCLIN24 of type Ifx_ASCLIN24 */
extern Ifx_ASCLIN24 MODULE_ASCLIN24;
/* Missing: MODULE_ASCLIN25 of type Ifx_ASCLIN25 */
extern Ifx_ASCLIN25 MODULE_ASCLIN25;
/* Missing: MODULE_ASCLIN26 of type Ifx_ASCLIN26 */
extern Ifx_ASCLIN26 MODULE_ASCLIN26;
/* Missing: MODULE_ASCLIN27 of type Ifx_ASCLIN27 */
extern Ifx_ASCLIN27 MODULE_ASCLIN27;
/* Missing: MODULE_TBCU of type Ifx_TBCU */
extern Ifx_TBCU MODULE_TBCU;
/* Missing: MODULE_CSBCU of type Ifx_CSBCU */
extern Ifx_CSBCU MODULE_CSBCU;
/* Missing: MODULE_SBCU of type Ifx_SBCU */
extern Ifx_SBCU MODULE_SBCU;
/* Missing: MODULE_COMBCU of type Ifx_COMBCU */
extern Ifx_COMBCU MODULE_COMBCU;
/* Missing: MODULE_CANXL0 of type Ifx_CANXL0 */
extern Ifx_CANXL0 MODULE_CANXL0;
/* Missing: MODULE_CANXL0_RAM of type Ifx_CANXL0_RAM */
extern Ifx_CANXL0_RAM MODULE_CANXL0_RAM;
/* Missing: MODULE_CAN0 of type Ifx_CAN0 */
extern Ifx_CAN0 MODULE_CAN0;
/* Missing: MODULE_CAN0_RAM of type Ifx_CAN0_RAM */
extern Ifx_CAN0_RAM MODULE_CAN0_RAM;
/* Missing: MODULE_CAN1 of type Ifx_CAN1 */
extern Ifx_CAN1 MODULE_CAN1;
/* Missing: MODULE_CAN1_RAM of type Ifx_CAN1_RAM */
extern Ifx_CAN1_RAM MODULE_CAN1_RAM;
/* Missing: MODULE_CAN2 of type Ifx_CAN2 */
extern Ifx_CAN2 MODULE_CAN2;
/* Missing: MODULE_CAN2_RAM of type Ifx_CAN2_RAM */
extern Ifx_CAN2_RAM MODULE_CAN2_RAM;
/* Missing: MODULE_CAN3 of type Ifx_CAN3 */
extern Ifx_CAN3 MODULE_CAN3;
/* Missing: MODULE_CAN3_RAM of type Ifx_CAN3_RAM */
extern Ifx_CAN3_RAM MODULE_CAN3_RAM;
/* Missing: MODULE_CAN4 of type Ifx_CAN4 */
extern Ifx_CAN4 MODULE_CAN4;
/* Missing: MODULE_CAN4_RAM of type Ifx_CAN4_RAM */
extern Ifx_CAN4_RAM MODULE_CAN4_RAM;
/* Missing: MODULE_CBS of type Ifx_CBS */
extern Ifx_CBS MODULE_CBS;
/* Missing: MODULE_CLOCK of type Ifx_CLOCK */
extern Ifx_CLOCK MODULE_CLOCK;
/* Missing: MODULE_CPU_CFI0 of type Ifx_CPU_CFI0 */
extern Ifx_CPU_CFI0 MODULE_CPU_CFI0;
/* Missing: MODULE_CPU_CFI1 of type Ifx_CPU_CFI1 */
extern Ifx_CPU_CFI1 MODULE_CPU_CFI1;
/* Missing: MODULE_CPU_CFI2 of type Ifx_CPU_CFI2 */
extern Ifx_CPU_CFI2 MODULE_CPU_CFI2;
/* Missing: MODULE_CPU_CFI3 of type Ifx_CPU_CFI3 */
extern Ifx_CPU_CFI3 MODULE_CPU_CFI3;
/* Missing: MODULE_CPU_CFI4 of type Ifx_CPU_CFI4 */
extern Ifx_CPU_CFI4 MODULE_CPU_CFI4;
/* Missing: MODULE_CPU_CFI5 of type Ifx_CPU_CFI5 */
extern Ifx_CPU_CFI5 MODULE_CPU_CFI5;
/* Missing: MODULE_CPU_CFICS of type Ifx_CPU_CFICS */
extern Ifx_CPU_CFICS MODULE_CPU_CFICS;
/* Missing: MODULE_CPU0 of type Ifx_CPU0 */
extern Ifx_CPU0 MODULE_CPU0;
/* Missing: MODULE_CPU1 of type Ifx_CPU1 */
extern Ifx_CPU1 MODULE_CPU1;
/* Missing: MODULE_CPU2 of type Ifx_CPU2 */
extern Ifx_CPU2 MODULE_CPU2;
/* Missing: MODULE_CPU3 of type Ifx_CPU3 */
extern Ifx_CPU3 MODULE_CPU3;
/* Missing: MODULE_CPU4 of type Ifx_CPU4 */
extern Ifx_CPU4 MODULE_CPU4;
/* Missing: MODULE_CPU5 of type Ifx_CPU5 */
extern Ifx_CPU5 MODULE_CPU5;
/* Missing: MODULE_CPUCS of type Ifx_CPUCS */
extern Ifx_CPUCS MODULE_CPUCS;
/* Missing: MODULE_DMA0 of type Ifx_DMA0 */
extern Ifx_DMA0 MODULE_DMA0;
/* Missing: MODULE_DMA1 of type Ifx_DMA1 */
extern Ifx_DMA1 MODULE_DMA1;
/* Missing: MODULE_DMU of type Ifx_DMU */
extern Ifx_DMU MODULE_DMU;
/* Missing: MODULE_DOM0 of type Ifx_DOM0 */
extern Ifx_DOM0 MODULE_DOM0;
/* Missing: MODULE_DOM1 of type Ifx_DOM1 */
extern Ifx_DOM1 MODULE_DOM1;
/* Missing: MODULE_DOM3 of type Ifx_DOM3 */
extern Ifx_DOM3 MODULE_DOM3;
/* Missing: MODULE_DOM6 of type Ifx_DOM6 */
extern Ifx_DOM6 MODULE_DOM6;
/* Missing: MODULE_DOM7 of type Ifx_DOM7 */
extern Ifx_DOM7 MODULE_DOM7;
/* Missing: MODULE_DOM4 of type Ifx_DOM4 */
extern Ifx_DOM4 MODULE_DOM4;
/* Missing: MODULE_DOM2 of type Ifx_DOM2 */
extern Ifx_DOM2 MODULE_DOM2;
/* Missing: MODULE_DOM5 of type Ifx_DOM5 */
extern Ifx_DOM5 MODULE_DOM5;
/* Missing: MODULE_DRE of type Ifx_DRE */
extern Ifx_DRE MODULE_DRE;
/* Missing: MODULE_ERAY0 of type Ifx_ERAY0 */
extern Ifx_ERAY0 MODULE_ERAY0;
/* Missing: MODULE_ERAY1 of type Ifx_ERAY1 */
extern Ifx_ERAY1 MODULE_ERAY1;
/* Missing: MODULE_FCE of type Ifx_FCE */
extern Ifx_FCE MODULE_FCE;
/* Missing: MODULE_FSI_CSRM of type Ifx_FSI_CSRM */
extern Ifx_FSI_CSRM MODULE_FSI_CSRM;
/* Missing: MODULE_FSI_HOST of type Ifx_FSI_HOST */
extern Ifx_FSI_HOST MODULE_FSI_HOST;
/* Missing: MODULE_GETH0 of type Ifx_GETH0 */
extern Ifx_GETH0 MODULE_GETH0;
/* Missing: MODULE_HSCT0 of type Ifx_HSCT0 */
extern Ifx_HSCT0 MODULE_HSCT0;
/* Missing: MODULE_HSCT1 of type Ifx_HSCT1 */
extern Ifx_HSCT1 MODULE_HSCT1;
/* Missing: MODULE_HSPHY of type Ifx_HSPHY */
extern Ifx_HSPHY MODULE_HSPHY;
/* Missing: MODULE_HSPHY_CRPARA of type Ifx_HSPHY_CRPARA */
extern Ifx_HSPHY_CRPARA MODULE_HSPHY_CRPARA;
/* Missing: MODULE_HSSL0 of type Ifx_HSSL0 */
extern Ifx_HSSL0 MODULE_HSSL0;
/* Missing: MODULE_HSSL1 of type Ifx_HSSL1 */
extern Ifx_HSSL1 MODULE_HSSL1;
/* Missing: MODULE_I2C0 of type Ifx_I2C0 */
extern Ifx_I2C0 MODULE_I2C0;
/* Missing: MODULE_I2C1 of type Ifx_I2C1 */
extern Ifx_I2C1 MODULE_I2C1;
/* Missing: MODULE_I2C2 of type Ifx_I2C2 */
extern Ifx_I2C2 MODULE_I2C2;
/* Missing: MODULE_INT of type Ifx_INT */
extern Ifx_INT MODULE_INT;
/* Missing: MODULE_LETH0 of type Ifx_LETH0 */
extern Ifx_LETH0 MODULE_LETH0;
/* Missing: MODULE_LLI0 of type Ifx_LLI0 */
extern Ifx_LLI0 MODULE_LLI0;
/* Missing: MODULE_LMU0 of type Ifx_LMU0 */
extern Ifx_LMU0 MODULE_LMU0;
/* Missing: MODULE_LMU1 of type Ifx_LMU1 */
extern Ifx_LMU1 MODULE_LMU1;
/* Missing: MODULE_LMU2 of type Ifx_LMU2 */
extern Ifx_LMU2 MODULE_LMU2;
/* Missing: MODULE_LMU3 of type Ifx_LMU3 */
extern Ifx_LMU3 MODULE_LMU3;
/* Missing: MODULE_LMU4 of type Ifx_LMU4 */
extern Ifx_LMU4 MODULE_LMU4;
/* Missing: MODULE_LMU5 of type Ifx_LMU5 */
extern Ifx_LMU5 MODULE_LMU5;
/* Missing: MODULE_LMU6 of type Ifx_LMU6 */
extern Ifx_LMU6 MODULE_LMU6;
/* Missing: MODULE_LMU7 of type Ifx_LMU7 */
extern Ifx_LMU7 MODULE_LMU7;
/* Missing: MODULE_LMU8 of type Ifx_LMU8 */
extern Ifx_LMU8 MODULE_LMU8;
/* Missing: MODULE_LMU9 of type Ifx_LMU9 */
extern Ifx_LMU9 MODULE_LMU9;
/* Missing: MODULE_MCDS2P of type Ifx_MCDS2P */
extern Ifx_MCDS2P MODULE_MCDS2P;
/* Missing: MODULE_MCDS4P of type Ifx_MCDS4P */
extern Ifx_MCDS4P MODULE_MCDS4P;
/* Missing: MODULE_MSC0 of type Ifx_MSC0 */
extern Ifx_MSC0 MODULE_MSC0;
/* Missing: MODULE_PCIE0_DSP of type Ifx_PCIE0_DSP */
extern Ifx_PCIE0_DSP MODULE_PCIE0_DSP;
/* Missing: MODULE_PCIE0_DSP_SRI of type Ifx_PCIE0_DSP_SRI */
extern Ifx_PCIE0_DSP_SRI MODULE_PCIE0_DSP_SRI;
/* Missing: MODULE_PCIE1_DSP of type Ifx_PCIE1_DSP */
extern Ifx_PCIE1_DSP MODULE_PCIE1_DSP;
/* Missing: MODULE_PCIE1_DSP_SRI of type Ifx_PCIE1_DSP_SRI */
extern Ifx_PCIE1_DSP_SRI MODULE_PCIE1_DSP_SRI;
/* Missing: MODULE_PCIE0_USP of type Ifx_PCIE0_USP */
extern Ifx_PCIE0_USP MODULE_PCIE0_USP;
/* Missing: MODULE_PCIE0_USP_SRI of type Ifx_PCIE0_USP_SRI */
extern Ifx_PCIE0_USP_SRI MODULE_PCIE0_USP_SRI;
/* Missing: MODULE_PCIE1_USP of type Ifx_PCIE1_USP */
extern Ifx_PCIE1_USP MODULE_PCIE1_USP;
/* Missing: MODULE_PCIE1_USP_SRI of type Ifx_PCIE1_USP_SRI */
extern Ifx_PCIE1_USP_SRI MODULE_PCIE1_USP_SRI;
/* Missing: MODULE_PFRWB0A of type Ifx_PFRWB0A */
extern Ifx_PFRWB0A MODULE_PFRWB0A;
/* Missing: MODULE_PFRWB0B of type Ifx_PFRWB0B */
extern Ifx_PFRWB0B MODULE_PFRWB0B;
/* Missing: MODULE_PFRWB1A of type Ifx_PFRWB1A */
extern Ifx_PFRWB1A MODULE_PFRWB1A;
/* Missing: MODULE_PFRWB1B of type Ifx_PFRWB1B */
extern Ifx_PFRWB1B MODULE_PFRWB1B;
/* Missing: MODULE_PFRWB2A of type Ifx_PFRWB2A */
extern Ifx_PFRWB2A MODULE_PFRWB2A;
/* Missing: MODULE_PFRWB2B of type Ifx_PFRWB2B */
extern Ifx_PFRWB2B MODULE_PFRWB2B;
/* Missing: MODULE_PFRWB3A of type Ifx_PFRWB3A */
extern Ifx_PFRWB3A MODULE_PFRWB3A;
/* Missing: MODULE_PFRWB3B of type Ifx_PFRWB3B */
extern Ifx_PFRWB3B MODULE_PFRWB3B;
/* Missing: MODULE_PFRWB4A of type Ifx_PFRWB4A */
extern Ifx_PFRWB4A MODULE_PFRWB4A;
/* Missing: MODULE_PFRWB4B of type Ifx_PFRWB4B */
extern Ifx_PFRWB4B MODULE_PFRWB4B;
/* Missing: MODULE_PFRWB5A of type Ifx_PFRWB5A */
extern Ifx_PFRWB5A MODULE_PFRWB5A;
/* Missing: MODULE_PFRWB5B of type Ifx_PFRWB5B */
extern Ifx_PFRWB5B MODULE_PFRWB5B;
/* Missing: MODULE_PFRWBCS of type Ifx_PFRWBCS */
extern Ifx_PFRWBCS MODULE_PFRWBCS;
/* Missing: MODULE_PMS of type Ifx_PMS */
extern Ifx_PMS MODULE_PMS;
/* Missing: MODULE_P01 of type Ifx_P */
extern Ifx_P MODULE_P01;
/* Missing: MODULE_P03 of type Ifx_P */
extern Ifx_P MODULE_P03;
/* Missing: MODULE_P04 of type Ifx_P */
extern Ifx_P MODULE_P04;
/* Missing: MODULE_P16 of type Ifx_P */
extern Ifx_P MODULE_P16;
/* Missing: MODULE_P25 of type Ifx_P */
extern Ifx_P MODULE_P25;
/* Missing: MODULE_P30 of type Ifx_P */
extern Ifx_P MODULE_P30;
/* Missing: MODULE_P31 of type Ifx_P */
extern Ifx_P MODULE_P31;
/* Missing: MODULE_P32 of type Ifx_P */
extern Ifx_P MODULE_P32;
/* Missing: MODULE_P35 of type Ifx_P */
extern Ifx_P MODULE_P35;
/* Missing: MODULE_PPU of type Ifx_PPU */
extern Ifx_PPU MODULE_PPU;
/* Missing: MODULE_PPU_STUDMI of type Ifx_PPU_STUDMI */
extern Ifx_PPU_STUDMI MODULE_PPU_STUDMI;
/* Missing: MODULE_PPU_DEBUG of type Ifx_PPU_DEBUG */
extern Ifx_PPU_DEBUG MODULE_PPU_DEBUG;
/* Missing: MODULE_PPU_SM of type Ifx_PPU_SM */
extern Ifx_PPU_SM MODULE_PPU_SM;
/* Missing: MODULE_PPU_APU of type Ifx_PPU_APU */
extern Ifx_PPU_APU MODULE_PPU_APU;
/* Missing: MODULE_PPU_CSMAP of type Ifx_PPU_CSMAP */
extern Ifx_PPU_CSMAP MODULE_PPU_CSMAP;
/* Missing: MODULE_PPU_VMEMAP of type Ifx_PPU_VMEMAP */
extern Ifx_PPU_VMEMAP MODULE_PPU_VMEMAP;
/* Missing: MODULE_PSI5S0 of type Ifx_PSI5S0 */
extern Ifx_PSI5S0 MODULE_PSI5S0;
/* Missing: MODULE_PSI5 of type Ifx_PSI5 */
extern Ifx_PSI5 MODULE_PSI5;
/* Missing: MODULE_QSPI0 of type Ifx_QSPI0 */
extern Ifx_QSPI0 MODULE_QSPI0;
/* Missing: MODULE_QSPI1 of type Ifx_QSPI1 */
extern Ifx_QSPI1 MODULE_QSPI1;
/* Missing: MODULE_QSPI2 of type Ifx_QSPI2 */
extern Ifx_QSPI2 MODULE_QSPI2;
/* Missing: MODULE_QSPI3 of type Ifx_QSPI3 */
extern Ifx_QSPI3 MODULE_QSPI3;
/* Missing: MODULE_QSPI4 of type Ifx_QSPI4 */
extern Ifx_QSPI4 MODULE_QSPI4;
/* Missing: MODULE_QSPI5 of type Ifx_QSPI5 */
extern Ifx_QSPI5 MODULE_QSPI5;
/* Missing: MODULE_QSPI6 of type Ifx_QSPI6 */
extern Ifx_QSPI6 MODULE_QSPI6;
/* Missing: MODULE_QSPI7 of type Ifx_QSPI7 */
extern Ifx_QSPI7 MODULE_QSPI7;
/* Missing: MODULE_SCU of type Ifx_SCU */
extern Ifx_SCU MODULE_SCU;
/* Missing: MODULE_SDMMC0 of type Ifx_SDMMC0 */
extern Ifx_SDMMC0 MODULE_SDMMC0;
/* Missing: MODULE_SENT0 of type Ifx_SENT0 */
extern Ifx_SENT0 MODULE_SENT0;
/* Missing: MODULE_SENT1 of type Ifx_SENT1 */
extern Ifx_SENT1 MODULE_SENT1;
/* Missing: MODULE_SMM of type Ifx_SMM */
extern Ifx_SMM MODULE_SMM;
/* Missing: MODULE_SMU of type Ifx_SMU */
extern Ifx_SMU MODULE_SMU;
/* Missing: MODULE_SMUSTDBY of type Ifx_SMUSTDBY */
extern Ifx_SMUSTDBY MODULE_SMUSTDBY;
/* Missing: MODULE_SRC of type Ifx_SRC */
extern Ifx_SRC MODULE_SRC;
/* Missing: MODULE_TRIF of type Ifx_TRIF */
extern Ifx_TRIF MODULE_TRIF;
/* Missing: MODULE_TRI of type Ifx_TRI */
extern Ifx_TRI MODULE_TRI;
/* Missing: MODULE_VMT0 of type Ifx_VMT0 */
extern Ifx_VMT0 MODULE_VMT0;
/* Missing: MODULE_VMT1 of type Ifx_VMT1 */
extern Ifx_VMT1 MODULE_VMT1;
/* Missing: MODULE_VMT2 of type Ifx_VMT2 */
extern Ifx_VMT2 MODULE_VMT2;
/* Missing: MODULE_VMT3 of type Ifx_VMT3 */
extern Ifx_VMT3 MODULE_VMT3;
/* Missing: MODULE_VMT4 of type Ifx_VMT4 */
extern Ifx_VMT4 MODULE_VMT4;
/* Missing: MODULE_VMT5 of type Ifx_VMT5 */
extern Ifx_VMT5 MODULE_VMT5;
/* Missing: MODULE_VMT6 of type Ifx_VMT6 */
extern Ifx_VMT6 MODULE_VMT6;
/* Missing: MODULE_VTMON of type Ifx_VTMON */
extern Ifx_VTMON MODULE_VTMON;
/* Missing: MODULE_WTU of type Ifx_WTU */
extern Ifx_WTU MODULE_WTU;
/* Missing: MODULE_XSPI0 of type Ifx_XSPI0 */
extern Ifx_XSPI0 MODULE_XSPI0;

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
