/* Base types + macros + shared enums + MODULE_* stubs + spy API (no driver-specific types or function decls) */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef float float32;
typedef unsigned char boolean;
typedef unsigned char uint8;
typedef signed short sint16;
typedef unsigned short uint16;
typedef signed int sint32;
typedef unsigned int uint32;
typedef unsigned char Ifx_Priority;

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
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif
#ifndef IFX_CONST
# define IFX_CONST const
#endif

/* Shared base register types used by SFR pointers in mocks */
typedef uint32 Ifx_UReg_32Bit;

/* Shared enums used across multiple drivers */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_dma = 2, IfxSrc_Tos_ppu = 3 } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* Only minimal fields required for compilation */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

/* Additional SFR helper blocks referenced by drivers */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;  /* EGTM Cluster SFR block */

/* Required extern MODULE instances */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy counters and control variables (externs) */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm_Pwm stubs */
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* IfxAdc_Tmadc stubs */
extern int mock_IfxAdc_Tmadc_initModuleConfig_callCount;
extern int mock_IfxAdc_Tmadc_runModule_callCount;
extern int mock_IfxAdc_Tmadc_enableChannelEvent_callCount;
extern int mock_IfxAdc_Tmadc_initChannelConfig_callCount;
extern int mock_IfxAdc_Tmadc_initModule_callCount;
extern int mock_IfxAdc_Tmadc_initChannel_callCount;

/* IfxEgtm_Cmu stubs */
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* IfxEgtm base stubs */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Trigger stubs */
extern int     mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* IfxPort stubs */
extern int     mock_IfxPort_togglePin_callCount;
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern uint32  mock_togglePin_callCount; /* alias counter for test convenience */

/* Optional CPU/IRQ helper stubs (commonly referenced) */
extern int mock_IfxCpu_enableInterrupts_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;

/* Spy getters (one per mocked function) */
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);
int mock_IfxAdc_Tmadc_runModule_getCallCount(void);
int mock_IfxAdc_Tmadc_enableChannelEvent_getCallCount(void);
int mock_IfxAdc_Tmadc_initChannelConfig_getCallCount(void);
int mock_IfxAdc_Tmadc_initModule_getCallCount(void);
int mock_IfxAdc_Tmadc_initChannel_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

/* Mock control */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
