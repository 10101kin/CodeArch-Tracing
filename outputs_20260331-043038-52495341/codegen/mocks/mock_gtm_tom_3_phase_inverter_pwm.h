/* Base types + MODULE stubs + shared enums + spy API (single-owner) */
#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned char boolean;
typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned long long uint64;
typedef signed long long sint64;
typedef uint32 Ifx_Priority;

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
/* ISR macro (3 args) */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
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
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_cpu6 = 6,
    IfxSrc_Tos_cpu7 = 7,
    IfxSrc_Tos_dma  = 8
} IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_P;

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

/* Spy API: counters, return-value controls, and capture buffers */
#define MOCK_MAX_CHANNELS 16

/* IfxGtm */
extern int     mock_IfxGtm_enable_callCount;
extern int     mock_IfxGtm_isEnabled_callCount;
extern boolean mock_IfxGtm_isEnabled_returnValue;
int mock_IfxGtm_enable_getCallCount(void);
int mock_IfxGtm_isEnabled_getCallCount(void);

/* IfxGtm_Cmu */
extern int     mock_IfxGtm_Cmu_enable_callCount;
extern int     mock_IfxGtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxGtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxGtm_Cmu_enableClocks_callCount;
extern int     mock_IfxGtm_Cmu_setClkFrequency_callCount;
int mock_IfxGtm_Cmu_enable_getCallCount(void);
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);

/* IfxGtm_Pwm */
extern int     mock_IfxGtm_Pwm_initConfig_callCount;
extern int     mock_IfxGtm_Pwm_init_callCount;
extern int     mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32  mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxGtm_Pwm_updateDuties_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Pwm_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Pwm_lastDtFalling[MOCK_MAX_CHANNELS];
int mock_IfxGtm_Pwm_initConfig_getCallCount(void);
int mock_IfxGtm_Pwm_init_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

/* IfxPort */
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern int     mock_IfxPort_togglePin_callCount;
extern uint32  mock_togglePin_callCount; /* dedicated counter as requested */
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);

/* IfxCpu IRQ install */
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

/* Mock control */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
