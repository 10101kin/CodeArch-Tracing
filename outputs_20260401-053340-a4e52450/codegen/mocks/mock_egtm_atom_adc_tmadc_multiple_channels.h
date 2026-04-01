/* Base types + MODULE stubs + shared enums + spy API (owner of shared/basic types only) */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef unsigned char boolean;
typedef float         float32;
typedef signed char   sint8;
typedef unsigned char uint8;
typedef signed short  sint16;
typedef unsigned short uint16;
typedef signed int    sint32;
typedef unsigned int  uint32;
typedef unsigned char Ifx_Priority; /* priority alias */

typedef uint32 Ifx_UReg_32Bit; /* register unit */

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
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_cpu2 = 2, IfxSrc_Tos_cpu3 = 3, IfxSrc_Tos_cpu4 = 4, IfxSrc_Tos_cpu5 = 5 } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (only those required) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS; /* cluster SFR stub */
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

/* Extern module instances needed by this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P33;

/* Spy API: counters, return controls, captured values */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm */
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern int     mock_IfxEgtm_enable_callCount;
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);

/* IfxAdc */
extern int mock_IfxAdc_clearTmadcResultFlag_callCount;
extern int mock_IfxAdc_enableModule_callCount;
extern int     mock_IfxAdc_isTmadcResultAvailable_callCount;
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;
int  mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);
int  mock_IfxAdc_enableModule_getCallCount(void);
int  mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

/* IfxEgtm_Trigger */
extern int     mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

/* IfxPort */
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern uint32 mock_togglePin_callCount; /* explicit test counter */
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

/* IfxEgtm_Pwm (primary PWM) */
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* Dead-time capture arrays (even if not used by a stubbed function) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

/* IfxAdc_Tmadc */
extern int mock_IfxAdc_Tmadc_runModule_callCount;
int  mock_IfxAdc_Tmadc_runModule_getCallCount(void);

/* Mock control */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
