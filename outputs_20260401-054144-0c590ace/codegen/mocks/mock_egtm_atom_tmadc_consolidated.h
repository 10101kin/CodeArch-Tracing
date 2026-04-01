/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H
#define MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H

/* Base type aliases */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char boolean;
typedef unsigned char Ifx_Priority;
typedef uint32 Ifx_UReg_32Bit;

/* Macros */
#ifndef TRUE
#define TRUE ((boolean)1)
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

/* IFX_INTERRUPT macro (3-arg) */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_cpu2 = 2, IfxSrc_Tos_dma = 3 } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy API: counters, return-value controls, and captured values */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM init/initConfig capture */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty/DT capture arrays (tests may use them even if update stubs are not present) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Toggle spy */
extern uint32  mock_togglePin_callCount;

/* Call counters and getters */
extern int mock_IfxEgtm_Pwm_init_callCount;
int  mock_IfxEgtm_Pwm_init_getCallCount(void);

extern int mock_IfxEgtm_Pwm_initConfig_callCount;
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_setFrequency_callCount;
int  mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_setTrigger_callCount;
int  mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_run_callCount;
int  mock_IfxEgtm_Atom_Timer_run_getCallCount(void);

extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_enable_callCount;
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);

extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

extern int mock_IfxEgtm_enable_callCount;
int  mock_IfxEgtm_enable_getCallCount(void);

extern int mock_IfxEgtm_isEnabled_callCount;
int  mock_IfxEgtm_isEnabled_getCallCount(void);

extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

extern int mock_IfxAdc_Tmadc_initModule_callCount;
int  mock_IfxAdc_Tmadc_initModule_getCallCount(void);

extern int mock_IfxAdc_Tmadc_initModuleConfig_callCount;
int  mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);

extern int mock_IfxAdc_enableModule_callCount;
int  mock_IfxAdc_enableModule_getCallCount(void);

extern int mock_IfxEgtm_PinMap_setAtomTout_callCount;
int  mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void);

extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

extern int mock_IfxCpu_enableInterrupts_callCount;
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

/* Return value controls */
extern boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* Reset API */
void mock_egtm_atom_tmadc_consolidated_reset(void);

#endif /* MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H */
