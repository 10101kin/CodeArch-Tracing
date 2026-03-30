#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef unsigned char boolean;
typedef float         float32;
typedef signed char   sint8;
typedef signed short  sint16;
typedef signed int    sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;
typedef uint32        Ifx_Priority;
typedef uint32        Ifx_TimerValue;

/* Macros */
#ifndef TRUE
# define TRUE  (1u)
#endif
#ifndef FALSE
# define FALSE (0u)
#endif
#ifndef NULL_PTR
# define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
# define IFX_STATIC static
#endif

/* IFX_INTERRUPT must be 3-arg */
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
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3,
    IfxSrc_Tos_safety = 4
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_GTM;
extern Ifx_GTM MODULE_GTM;

typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P01;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P11;
extern Ifx_P MODULE_P12;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P24;
extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P26;
extern Ifx_P MODULE_P30;
extern Ifx_P MODULE_P31;
extern Ifx_P MODULE_P32;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P40;
extern Ifx_P MODULE_P41;

/* Additional SFR register modules (stubs) */
typedef struct { uint32 reserved; } Ifx_ASCLIN;  extern Ifx_ASCLIN MODULE_ASCLIN0, MODULE_ASCLIN1, MODULE_ASCLIN2, MODULE_ASCLIN3, MODULE_ASCLIN4, MODULE_ASCLIN5, MODULE_ASCLIN6, MODULE_ASCLIN7, MODULE_ASCLIN8, MODULE_ASCLIN9, MODULE_ASCLIN10, MODULE_ASCLIN11, MODULE_ASCLIN12, MODULE_ASCLIN13, MODULE_ASCLIN14, MODULE_ASCLIN15, MODULE_ASCLIN16, MODULE_ASCLIN17, MODULE_ASCLIN18, MODULE_ASCLIN19, MODULE_ASCLIN20, MODULE_ASCLIN21, MODULE_ASCLIN22, MODULE_ASCLIN23;
typedef struct { uint32 reserved; } Ifx_CAN;     extern Ifx_CAN MODULE_CAN0, MODULE_CAN1, MODULE_CAN2;
typedef struct { uint32 reserved; } Ifx_CBS;     extern Ifx_CBS MODULE_CBS;
typedef struct { uint32 reserved; } Ifx_CCU6;    extern Ifx_CCU6 MODULE_CCU60, MODULE_CCU61;
typedef struct { uint32 reserved; } Ifx_CONVERTER; extern Ifx_CONVERTER MODULE_CONVCTRL;
typedef struct { uint32 reserved; } Ifx_CPU;     extern Ifx_CPU MODULE_CPU0, MODULE_CPU1, MODULE_CPU2, MODULE_CPU3;
typedef struct { uint32 reserved; } Ifx_DAM;     extern Ifx_DAM MODULE_DAM0;
typedef struct { uint32 reserved; } Ifx_DMA;     extern Ifx_DMA MODULE_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;     extern Ifx_DMU MODULE_DMU;
typedef struct { uint32 reserved; } Ifx_DOM;     extern Ifx_DOM MODULE_DOM0;
typedef struct { uint32 reserved; } Ifx_EDSADC;  extern Ifx_EDSADC MODULE_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY;    extern Ifx_ERAY MODULE_ERAY0, MODULE_ERAY1;
typedef struct { uint32 reserved; } Ifx_EVADC;   extern Ifx_EVADC MODULE_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;     extern Ifx_FCE MODULE_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;     extern Ifx_FSI MODULE_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;    extern Ifx_GETH MODULE_GETH;
typedef struct { uint32 reserved; } Ifx_GPT12;   extern Ifx_GPT12 MODULE_GPT120;
typedef struct { uint32 reserved; } Ifx_HSCT;    extern Ifx_HSCT MODULE_HSCT0;
typedef struct { uint32 reserved; } Ifx_HSSL;    extern Ifx_HSSL MODULE_HSSL0;
typedef struct { uint32 reserved; } Ifx_I2C;     extern Ifx_I2C MODULE_I2C0, MODULE_I2C1;
typedef struct { uint32 reserved; } Ifx_INT;     extern Ifx_INT MODULE_INT;
typedef struct { uint32 reserved; } Ifx_IOM;     extern Ifx_IOM MODULE_IOM;
typedef struct { uint32 reserved; } Ifx_LMU;     extern Ifx_LMU MODULE_LMU0;
typedef struct { uint32 reserved; } Ifx_MINIMCDS; extern Ifx_MINIMCDS MODULE_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC;     extern Ifx_MSC MODULE_MSC0, MODULE_MSC1, MODULE_MSC2;
typedef struct { uint32 reserved; } Ifx_MTU;     extern Ifx_MTU MODULE_MTU;
typedef struct { uint32 reserved; } Ifx_PFI;     extern Ifx_PFI MODULE_PFI0, MODULE_PFI1, MODULE_PFI2, MODULE_PFI3;
typedef struct { uint32 reserved; } Ifx_PMS;     extern Ifx_PMS MODULE_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;     extern Ifx_PMU MODULE_PMU;
typedef struct { uint32 reserved; } Ifx_PSI5S;   extern Ifx_PSI5S MODULE_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;    extern Ifx_PSI5 MODULE_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI;    extern Ifx_QSPI MODULE_QSPI0, MODULE_QSPI1, MODULE_QSPI2, MODULE_QSPI3, MODULE_QSPI4;
typedef struct { uint32 reserved; } Ifx_SBCU;    extern Ifx_SBCU MODULE_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;     extern Ifx_SCU MODULE_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;    extern Ifx_SENT MODULE_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;     extern Ifx_SMU MODULE_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;     extern Ifx_SRC MODULE_SRC;
typedef struct { uint32 reserved; } Ifx_STM;     extern Ifx_STM MODULE_STM0, MODULE_STM1, MODULE_STM2, MODULE_STM3;

/* Spy counters and return-value controls */
#define MOCK_MAX_CHANNELS 16

/* IfxGtm_Tom_Timer */
extern int      mock_IfxGtm_Tom_Timer_applyUpdate_callCount;
extern int      mock_IfxGtm_Tom_Timer_getPeriod_callCount;
extern int      mock_IfxGtm_Tom_Timer_init_callCount;
extern int      mock_IfxGtm_Tom_Timer_initConfig_callCount;
extern int      mock_IfxGtm_Tom_Timer_disableUpdate_callCount;
extern Ifx_TimerValue mock_IfxGtm_Tom_Timer_getPeriod_returnValue;
extern boolean  mock_IfxGtm_Tom_Timer_init_returnValue;

/* IfxGtm */
extern int      mock_IfxGtm_isEnabled_callCount;
extern int      mock_IfxGtm_enable_callCount;
extern boolean  mock_IfxGtm_isEnabled_returnValue;

/* IfxGtm_Cmu */
extern int      mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern int      mock_IfxGtm_Cmu_enableClocks_callCount;
extern int      mock_IfxGtm_Cmu_enable_callCount;
extern int      mock_IfxGtm_Cmu_isEnabled_callCount;
extern int      mock_IfxGtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxGtm_Cmu_setClkFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
extern boolean  mock_IfxGtm_Cmu_isEnabled_returnValue;

/* IfxGtm_Tom_PwmHl */
extern int      mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount;
extern int      mock_IfxGtm_Tom_PwmHl_init_callCount;
extern int      mock_IfxGtm_Tom_PwmHl_setOnTime_callCount;
extern int      mock_IfxGtm_Tom_PwmHl_setMode_callCount;
extern int      mock_IfxGtm_Tom_PwmHl_initConfig_callCount;
extern boolean  mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue;
extern boolean  mock_IfxGtm_Tom_PwmHl_init_returnValue;
extern boolean  mock_IfxGtm_Tom_PwmHl_setMode_returnValue;

/* IfxGtm_PinMap */
extern int      mock_IfxGtm_PinMap_setTomTout_callCount;

/* IfxPort */
extern int      mock_IfxPort_togglePin_callCount;
extern int      mock_IfxPort_setPinModeOutput_callCount;

/* Value-capture spy fields */
extern uint32  mock_IfxGtm_Tom_PwmHl_init_lastNumChannels;
extern float32 mock_IfxGtm_Tom_PwmHl_init_lastFrequency;
extern uint32  mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels;
extern float32 mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency;
extern float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount; /* alias to IfxPort toggle for legacy tests */

/* Mock control API */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);
int  mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void);
int  mock_IfxGtm_Tom_Timer_getPeriod_getCallCount(void);
int  mock_IfxGtm_Tom_Timer_init_getCallCount(void);
int  mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void);
int  mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void);
int  mock_IfxGtm_isEnabled_getCallCount(void);
int  mock_IfxGtm_enable_getCallCount(void);
int  mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxGtm_Cmu_enable_getCallCount(void);
int  mock_IfxGtm_Cmu_isEnabled_getCallCount(void);
int  mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxGtm_Tom_PwmHl_setDeadtime_getCallCount(void);
int  mock_IfxGtm_Tom_PwmHl_init_getCallCount(void);
int  mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(void);
int  mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(void);
int  mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(void);
int  mock_IfxGtm_PinMap_setTomTout_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
