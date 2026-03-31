/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char boolean;
typedef unsigned char Ifx_Priority;

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
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3,
    IfxSrc_Tos_cpu3 = 4,
    IfxSrc_Tos_cpu4 = 5,
    IfxSrc_Tos_cpu5 = 6
} IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;

typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_TBCU;
typedef struct { uint32 reserved; } Ifx_CSBCU;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_COMBCU;
typedef struct { uint32 reserved; } Ifx_CANXL;
typedef struct { uint32 reserved; } Ifx_CANXL_RAM;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CAN_RAM;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CLOCK;
typedef struct { uint32 reserved; } Ifx_CPU_CFI;
typedef struct { uint32 reserved; } Ifx_CPU_CFICS;
typedef struct { uint32 reserved; } Ifx_CPU;
typedef struct { uint32 reserved; } Ifx_CPUCS;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM;
typedef struct { uint32 reserved; } Ifx_DRE;
typedef struct { uint32 reserved; } Ifx_ERAY;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_HSCT;
typedef struct { uint32 reserved; } Ifx_HSPHY;
typedef struct { uint32 reserved; } Ifx_HSSL;
typedef struct { uint32 reserved; } Ifx_I2C;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_LETH;
typedef struct { uint32 reserved; } Ifx_LLI;
typedef struct { uint32 reserved; } Ifx_LMU;
typedef struct { uint32 reserved; } Ifx_MCDS;
typedef struct { uint32 reserved; } Ifx_MSC;
typedef struct { uint32 reserved; } Ifx_PCIE_DSP;
typedef struct { uint32 reserved; } Ifx_PCIE_DSP_SRI;
typedef struct { uint32 reserved; } Ifx_PCIE_USP;
typedef struct { uint32 reserved; } Ifx_PCIE_USP_SRI;
typedef struct { uint32 reserved; } Ifx_PFRWB;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PPU;
typedef struct { uint32 reserved; } Ifx_PPU_SUB;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SDMMC;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMM;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SMUSTDBY;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_TRIF;
typedef struct { uint32 reserved; } Ifx_TRI;
typedef struct { uint32 reserved; } Ifx_VMT;
typedef struct { uint32 reserved; } Ifx_VTMON;
typedef struct { uint32 reserved; } Ifx_WTU;
typedef struct { uint32 reserved; } Ifx_XSPI;

/* Extern MODULE_* instances (subset per required mapping + needed ports) */
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

/* Spy counters and return-value controls */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;

extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount;

#define MOCK_MAX_CHANNELS 16
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_update_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_update_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void);
int mock_togglePin_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
