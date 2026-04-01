#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef float          float32;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef signed int     sint32;
typedef signed short   sint16;
typedef unsigned char  boolean;
typedef unsigned int   Ifx_Priority;

/* Macros */
#ifndef TRUE
# define TRUE  (1)
#endif
#ifndef FALSE
# define FALSE (0)
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
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_P;
/* Additional SFR base structs used by other MODULE_* symbols (declared for completeness) */
typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CCU6;
typedef struct { uint32 reserved; } Ifx_CONVERTER;
typedef struct { uint32 reserved; } Ifx_CPU;
typedef struct { uint32 reserved; } Ifx_DAM;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM;
typedef struct { uint32 reserved; } Ifx_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY;
typedef struct { uint32 reserved; } Ifx_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_GPT12;
typedef struct { uint32 reserved; } Ifx_HSCT;
typedef struct { uint32 reserved; } Ifx_HSSL;
typedef struct { uint32 reserved; } Ifx_I2C;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_IOM;
typedef struct { uint32 reserved; } Ifx_LMU;
typedef struct { uint32 reserved; } Ifx_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC;
typedef struct { uint32 reserved; } Ifx_MTU;
typedef struct { uint32 reserved; } Ifx_PFI;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_STM;

/* Extern MODULE instances (commonly used) */
extern Ifx_GTM MODULE_GTM;
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

/* Spy counters and return-value controls */
/* IfxGtm_Cmu */
extern int     mock_IfxGtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxGtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxGtm_Cmu_enableClocks_callCount;
extern int     mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxGtm_Cmu_enable_callCount;
extern int     mock_IfxGtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isEnabled_returnValue;

/* IfxGtm_Pwm */
extern int     mock_IfxGtm_Pwm_initConfig_callCount;
extern int     mock_IfxGtm_Pwm_init_callCount;
extern int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;

/* IfxGtm_Tom_Timer */
extern int     mock_IfxGtm_Tom_Timer_disableUpdate_callCount;
extern int     mock_IfxGtm_Tom_Timer_init_callCount;
extern boolean mock_IfxGtm_Tom_Timer_init_returnValue;
extern int     mock_IfxGtm_Tom_Timer_applyUpdate_callCount;
extern int     mock_IfxGtm_Tom_Timer_initConfig_callCount;

/* IfxGtm base */
extern int     mock_IfxGtm_isEnabled_callCount;
extern boolean mock_IfxGtm_isEnabled_returnValue;
extern int     mock_IfxGtm_enable_callCount;

/* IfxPort */
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern int     mock_IfxPort_togglePin_callCount;
extern uint32  mock_togglePin_callCount; /* additional alias counter */

/* PWM capture spy fields */
#ifndef MOCK_MAX_CHANNELS
# define MOCK_MAX_CHANNELS 16
#endif
extern uint32  mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);

/* Call count getters (one per mocked function) */
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enable_getCallCount(void);
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void);

int mock_IfxGtm_Pwm_initConfig_getCallCount(void);
int mock_IfxGtm_Pwm_init_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void);
int mock_IfxGtm_Tom_Timer_init_getCallCount(void);
int mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void);
int mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void);

int mock_IfxGtm_isEnabled_getCallCount(void);
int mock_IfxGtm_enable_getCallCount(void);

int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_togglePin_getCallCount(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
