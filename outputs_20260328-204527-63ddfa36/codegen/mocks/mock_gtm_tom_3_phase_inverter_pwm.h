/*
 * Base mock header for GTM_TOM_3_Phase_Inverter_PWM
 * - Owns base types, macros, shared enums, MODULE_* stubs, and spy API declarations only
 */
#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float               float32;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned char       boolean;
typedef unsigned char       Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE       (1u)
#endif
#ifndef FALSE
#define FALSE      (0u)
#endif
#ifndef NULL_PTR
#define NULL_PTR   ((void*)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum {
    Ifx_ActiveState_inactive = 0,
    Ifx_ActiveState_active   = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_dma  = 4
} IfxSrc_Tos;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_P;
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
typedef struct { uint32 reserved; } Ifx_CAN0;
typedef struct { uint32 reserved; } Ifx_CAN1;
typedef struct { uint32 reserved; } Ifx_CAN2;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CCU60;
typedef struct { uint32 reserved; } Ifx_CCU61;
typedef struct { uint32 reserved; } Ifx_CONVCTRL;
typedef struct { uint32 reserved; } Ifx_CPU0;
typedef struct { uint32 reserved; } Ifx_CPU1;
typedef struct { uint32 reserved; } Ifx_CPU2;
typedef struct { uint32 reserved; } Ifx_CPU3;
typedef struct { uint32 reserved; } Ifx_DAM0;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM0;
typedef struct { uint32 reserved; } Ifx_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY0;
typedef struct { uint32 reserved; } Ifx_ERAY1;
typedef struct { uint32 reserved; } Ifx_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_GPT120;
typedef struct { uint32 reserved; } Ifx_HSCT0;
typedef struct { uint32 reserved; } Ifx_HSSL0;
typedef struct { uint32 reserved; } Ifx_I2C0;
typedef struct { uint32 reserved; } Ifx_I2C1;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_IOM;
typedef struct { uint32 reserved; } Ifx_LMU0;
typedef struct { uint32 reserved; } Ifx_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC0;
typedef struct { uint32 reserved; } Ifx_MSC1;
typedef struct { uint32 reserved; } Ifx_MSC2;
typedef struct { uint32 reserved; } Ifx_MTU;
typedef struct { uint32 reserved; } Ifx_PFI0;
typedef struct { uint32 reserved; } Ifx_PFI1;
typedef struct { uint32 reserved; } Ifx_PFI2;
typedef struct { uint32 reserved; } Ifx_PFI3;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI0;
typedef struct { uint32 reserved; } Ifx_QSPI1;
typedef struct { uint32 reserved; } Ifx_QSPI2;
typedef struct { uint32 reserved; } Ifx_QSPI3;
typedef struct { uint32 reserved; } Ifx_QSPI4;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_STM0;
typedef struct { uint32 reserved; } Ifx_STM1;
typedef struct { uint32 reserved; } Ifx_STM2;
typedef struct { uint32 reserved; } Ifx_STM3;

/* Extern MODULE_* instances */
extern Ifx_GTM      MODULE_GTM;
extern Ifx_P        MODULE_P00;
extern Ifx_ASCLIN0  MODULE_ASCLIN0;
extern Ifx_ASCLIN1  MODULE_ASCLIN1;
extern Ifx_ASCLIN2  MODULE_ASCLIN2;
extern Ifx_ASCLIN3  MODULE_ASCLIN3;
extern Ifx_ASCLIN4  MODULE_ASCLIN4;
extern Ifx_ASCLIN5  MODULE_ASCLIN5;
extern Ifx_ASCLIN6  MODULE_ASCLIN6;
extern Ifx_ASCLIN7  MODULE_ASCLIN7;
extern Ifx_ASCLIN8  MODULE_ASCLIN8;
extern Ifx_ASCLIN9  MODULE_ASCLIN9;
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
extern Ifx_CAN0     MODULE_CAN0;
extern Ifx_CAN1     MODULE_CAN1;
extern Ifx_CAN2     MODULE_CAN2;
extern Ifx_CBS      MODULE_CBS;
extern Ifx_CCU60    MODULE_CCU60;
extern Ifx_CCU61    MODULE_CCU61;
extern Ifx_CONVCTRL MODULE_CONVCTRL;
extern Ifx_CPU0     MODULE_CPU0;
extern Ifx_CPU1     MODULE_CPU1;
extern Ifx_CPU2     MODULE_CPU2;
extern Ifx_CPU3     MODULE_CPU3;
extern Ifx_DAM0     MODULE_DAM0;
extern Ifx_DMA      MODULE_DMA;
extern Ifx_DMU      MODULE_DMU;
extern Ifx_DOM0     MODULE_DOM0;
extern Ifx_EDSADC   MODULE_EDSADC;
extern Ifx_ERAY0    MODULE_ERAY0;
extern Ifx_ERAY1    MODULE_ERAY1;
extern Ifx_EVADC    MODULE_EVADC;
extern Ifx_FCE      MODULE_FCE;
extern Ifx_FSI      MODULE_FSI;
extern Ifx_GETH     MODULE_GETH;
extern Ifx_GPT120   MODULE_GPT120;
extern Ifx_HSCT0    MODULE_HSCT0;
extern Ifx_HSSL0    MODULE_HSSL0;
extern Ifx_I2C0     MODULE_I2C0;
extern Ifx_I2C1     MODULE_I2C1;
extern Ifx_INT      MODULE_INT;
extern Ifx_IOM      MODULE_IOM;
extern Ifx_LMU0     MODULE_LMU0;
extern Ifx_MINIMCDS MODULE_MINIMCDS;
extern Ifx_MSC0     MODULE_MSC0;
extern Ifx_MSC1     MODULE_MSC1;
extern Ifx_MSC2     MODULE_MSC2;
extern Ifx_MTU      MODULE_MTU;
extern Ifx_PFI0     MODULE_PFI0;
extern Ifx_PFI1     MODULE_PFI1;
extern Ifx_PFI2     MODULE_PFI2;
extern Ifx_PFI3     MODULE_PFI3;
extern Ifx_PMS      MODULE_PMS;
extern Ifx_PMU      MODULE_PMU;
extern Ifx_P        MODULE_P01;
extern Ifx_P        MODULE_P02;
extern Ifx_P        MODULE_P10;
extern Ifx_P        MODULE_P11;
extern Ifx_P        MODULE_P12;
extern Ifx_P        MODULE_P13;
extern Ifx_P        MODULE_P14;
extern Ifx_P        MODULE_P15;
extern Ifx_P        MODULE_P20;
extern Ifx_P        MODULE_P21;
extern Ifx_P        MODULE_P22;
extern Ifx_P        MODULE_P23;
extern Ifx_P        MODULE_P24;
extern Ifx_P        MODULE_P25;
extern Ifx_P        MODULE_P26;
extern Ifx_P        MODULE_P30;
extern Ifx_P        MODULE_P31;
extern Ifx_P        MODULE_P32;
extern Ifx_P        MODULE_P33;
extern Ifx_P        MODULE_P34;
extern Ifx_P        MODULE_P40;
extern Ifx_P        MODULE_P41;
extern Ifx_PSI5S    MODULE_PSI5S;
extern Ifx_PSI5     MODULE_PSI5;
extern Ifx_QSPI0    MODULE_QSPI0;
extern Ifx_QSPI1    MODULE_QSPI1;
extern Ifx_QSPI2    MODULE_QSPI2;
extern Ifx_QSPI3    MODULE_QSPI3;
extern Ifx_QSPI4    MODULE_QSPI4;
extern Ifx_SBCU     MODULE_SBCU;
extern Ifx_SCU      MODULE_SCU;
extern Ifx_SENT     MODULE_SENT;
extern Ifx_SMU      MODULE_SMU;
extern Ifx_SRC      MODULE_SRC;
extern Ifx_STM0     MODULE_STM0;
extern Ifx_STM1     MODULE_STM1;
extern Ifx_STM2     MODULE_STM2;
extern Ifx_STM3     MODULE_STM3;

/* Spy API: counters, return-value controls, and capture buffers */
#define MOCK_MAX_CHANNELS 16

/* IfxGtm_Cmu */
extern int      mock_IfxGtm_Cmu_enableClocks_callCount;
extern int      mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getModuleFrequency_returnValue;

/* IfxPort */
extern int      mock_IfxPort_setPinModeOutput_callCount;
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* dedicated alias for toggle tracking */

/* IfxGtm_Pwm */
extern int      mock_IfxGtm_Pwm_init_callCount;
extern int      mock_IfxGtm_Pwm_initConfig_callCount;
extern int      mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32   mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxGtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxGtm_Pwm_initConfig_lastFrequency;
extern float32  mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* Optional DT spies (kept for compatibility) */
extern float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxGtm */
extern int      mock_IfxGtm_enable_callCount;
extern int      mock_IfxGtm_isEnabled_callCount;
extern boolean  mock_IfxGtm_isEnabled_returnValue;

/* Spy getters (one per function with a callCount) */
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxGtm_Pwm_init_getCallCount(void);
int mock_IfxGtm_Pwm_initConfig_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxGtm_enable_getCallCount(void);
int mock_IfxGtm_isEnabled_getCallCount(void);

/* Mock control */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
