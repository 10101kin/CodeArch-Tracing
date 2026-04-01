/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H
#define MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H

/* Base type aliases */
typedef float float32;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char boolean;
typedef unsigned int Ifx_Priority;
typedef unsigned int Ifx_UReg_32Bit; /* SFR 32-bit register type */

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
/* IFX_INTERRUPT must take 3 args */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;

typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_cpu2 = 2, IfxSrc_Tos_dma = 3 } IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* Minimal MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

typedef struct { uint32 reserved; } Ifx_ADC;
extern Ifx_ADC MODULE_ADC;

typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P33;

/* Spy API storage externs and controls */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16U
#endif

/* Primary PWM init/initConfig spies */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_update_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_update_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Call counters for all mocked functions */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern int mock_IfxAdc_Tmadc_initModule_callCount;
extern int mock_IfxAdc_Tmadc_initModuleConfig_callCount;
extern int mock_IfxAdc_enableModule_callCount;
extern int mock_IfxEgtm_Atom_Timer_setFrequency_callCount;
extern int mock_IfxEgtm_Atom_Timer_setTrigger_callCount;
extern int mock_IfxEgtm_Atom_Timer_run_callCount;
extern int mock_IfxEgtm_PinMap_setAtomTout_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;

/* Return-value controls for non-void stubs */
extern boolean mock_IfxEgtm_Pwm_init_returnValue; /* typically not used, keep for test-flex */
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
extern boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;

/* Getters per mocked function (COMPLETE SPY API RULE) */
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int mock_IfxAdc_Tmadc_initModule_getCallCount(void);
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);
int mock_IfxAdc_enableModule_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void);
int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);

/* Mock control */
void mock_egtm_atom_tmadc_consolidated_reset(void);

#endif /* MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H */
