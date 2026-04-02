#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char uint8;
typedef unsigned char boolean;
typedef uint8 Ifx_Priority;

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

/* iLLD ISR macro (3 args) */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums */
typedef enum { Ifx_ActiveState_inactive = 0, Ifx_ActiveState_active = 1 } Ifx_ActiveState;
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_dma = 2 } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* Common iLLD base register types used in structs */
typedef uint32 Ifx_UReg_32Bit;

/* MODULE_* SFR register-block stubs (only those required for this module) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy counters and return controls (Complete Spy API Rule) */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm_Pwm */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* Optional DT spies (kept for test compatibility) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* IfxEgtm */
extern int     mock_IfxEgtm_isEnabled_callCount;
extern int     mock_IfxEgtm_enable_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Trigger */
extern int     mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* IfxAdc */
extern int     mock_IfxAdc_isTmadcResultAvailable_callCount;
extern int     mock_IfxAdc_clearTmadcResultFlag_callCount;
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;

/* IfxPort */
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
/* Alias kept for compatibility with tests expecting this name */
extern int mock_togglePin_callCount;

/* Mock control API (getters + reset) */
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int  mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);
int  mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
