/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base primitive types */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
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

/* Shared enums used across multiple drivers */
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
    IfxSrc_Tos_dma  = 6
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_SRC;

typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_BCU;
typedef struct { uint32 reserved; } Ifx_CANXL;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CLOCK;
typedef struct { uint32 reserved; } Ifx_CPU_CFI;
typedef struct { uint32 reserved; } Ifx_CPU;
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
typedef struct { uint32 reserved; } Ifx_PCIE_USP;
typedef struct { uint32 reserved; } Ifx_PFRWB;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PPU;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SDMMC;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMM;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SMU_STDBY;
typedef struct { uint32 reserved; } Ifx_TRIF;
typedef struct { uint32 reserved; } Ifx_TRI;
typedef struct { uint32 reserved; } Ifx_VMT;
typedef struct { uint32 reserved; } Ifx_VTMON;
typedef struct { uint32 reserved; } Ifx_WTU;
typedef struct { uint32 reserved; } Ifx_XSPI;

/* Extern instances for modules we commonly touch in this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;
extern Ifx_SRC  MODULE_SRC;
extern Ifx_WTU  MODULE_WTU;

/* Spy counters and return-value controls */
/* IfxEgtm */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;

/* IfxEgtm_Pwm */
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDeadTime_callCount;

/* IfxPort */
extern uint32  mock_togglePin_callCount;

/* Spy capture fields for primary PWM driver */
#define MOCK_MAX_CHANNELS 16
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDeadTime_getCallCount(void);

int mock_IfxPort_togglePin_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
