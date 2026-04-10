/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_2_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_2_H

/* Base type aliases */
typedef float               float32;
typedef unsigned int        uint32;
typedef int                 sint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef short               sint16;
typedef unsigned char       boolean;
typedef unsigned int        Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE  (1u)
#endif
#ifndef FALSE
#define FALSE (0u)
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif
/* ISR define: 3-arg macro */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

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
    IfxSrc_Tos_dma  = 4,
    IfxSrc_Tos_eray = 5
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_GTM;
/* Additional SFR block stubs used by PWM internals */
typedef struct { uint32 reserved; } Ifx_GTM_ATOM;
typedef struct { uint32 reserved; } Ifx_GTM_TOM;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM;

typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CCU6;
typedef struct { uint32 reserved; } Ifx_CONVCTRL;
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

/* Extern MODULE_* used by this module */
extern Ifx_GTM MODULE_GTM;
extern Ifx_P   MODULE_P02;
extern Ifx_P   MODULE_P13;
/* Optional wider extern visibility for other PORTs if included by production */
extern Ifx_P   MODULE_P00; extern Ifx_P MODULE_P01; extern Ifx_P MODULE_P10; extern Ifx_P MODULE_P11; extern Ifx_P MODULE_P12; extern Ifx_P MODULE_P14; extern Ifx_P MODULE_P15; extern Ifx_P MODULE_P20; extern Ifx_P MODULE_P21; extern Ifx_P MODULE_P22; extern Ifx_P MODULE_P23; extern Ifx_P MODULE_P24; extern Ifx_P MODULE_P25; extern Ifx_P MODULE_P26; extern Ifx_P MODULE_P30; extern Ifx_P MODULE_P31; extern Ifx_P MODULE_P32; extern Ifx_P MODULE_P33; extern Ifx_P MODULE_P34; extern Ifx_P MODULE_P40; extern Ifx_P MODULE_P41; extern Ifx_P MODULE_P02; extern Ifx_P MODULE_P13;

/* Spy counters and return-value controls */
#define MOCK_MAX_CHANNELS 16u

/* IfxPort */
extern int     mock_IfxPort_togglePin_callCount;
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern uint32  mock_togglePin_callCount;

/* IfxGtm */
extern int     mock_IfxGtm_isEnabled_callCount;
extern boolean mock_IfxGtm_isEnabled_returnValue;
extern int     mock_IfxGtm_enable_callCount;

/* IfxGtm_Pwm */
extern int     mock_IfxGtm_Pwm_init_callCount;
extern uint32  mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_init_lastFrequency;

extern int     mock_IfxGtm_Pwm_initConfig_callCount;
extern uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_initConfig_lastFrequency;

extern int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* Dead-time spies (kept for tests compatibility, even if not used by current stubs) */
extern float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxGtm_Cmu */
extern int     mock_IfxGtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxGtm_Cmu_enableClocks_callCount;
extern int     mock_IfxGtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
/* Mandatory CMU extras */
extern int     mock_IfxGtm_Cmu_enable_callCount;
extern int     mock_IfxGtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxGtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getGclkFrequency_returnValue;
extern int     mock_IfxGtm_Cmu_getClkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getClkFrequency_returnValue;

/* Optional IRQ install (used by examples/tests) */
extern int     mock_IfxCpu_Irq_installInterruptHandler_callCount;

/* Mock control API */
void mock_gtm_tom_3_phase_inverter_pwm_2_reset(void);

/* Per-function getters (COMPLETE SPY API RULE) */
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxGtm_isEnabled_getCallCount(void);
int mock_IfxGtm_enable_getCallCount(void);
int mock_IfxGtm_Pwm_init_getCallCount(void);
int mock_IfxGtm_Pwm_initConfig_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enable_getCallCount(void);
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_getClkFrequency_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_2_H */
