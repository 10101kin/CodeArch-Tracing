/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char boolean;
typedef unsigned int Ifx_Priority;

typedef uint32 Ifx_UReg_32Bit;

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
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1, IfxSrc_Tos_cpu2, IfxSrc_Tos_cpu3, IfxSrc_Tos_dma, IfxSrc_Tos_gtm } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

typedef struct { uint32 reserved; } Ifx_GTM; /* legacy alias if needed elsewhere */

typedef struct { uint32 reserved; } Ifx_P00_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P01_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P02_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P03_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P04_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P10_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P13_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P14_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P15_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P16_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P20_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P21_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P22_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P23_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P25_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P30_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P31_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P32_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P33_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P34_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P35_TypeDummy;
typedef struct { uint32 reserved; } Ifx_P40_TypeDummy;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P00;
extern Ifx_P    MODULE_P01;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P04;
extern Ifx_P    MODULE_P10;
extern Ifx_P    MODULE_P13;
extern Ifx_P    MODULE_P14;
extern Ifx_P    MODULE_P15;
extern Ifx_P    MODULE_P16;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;
extern Ifx_P    MODULE_P22;
extern Ifx_P    MODULE_P23;
extern Ifx_P    MODULE_P25;
extern Ifx_P    MODULE_P30;
extern Ifx_P    MODULE_P31;
extern Ifx_P    MODULE_P32;
extern Ifx_P    MODULE_P33;
extern Ifx_P    MODULE_P34;
extern Ifx_P    MODULE_P35;
extern Ifx_P    MODULE_P40;

/* Spy API externs and control */
#define MOCK_MAX_CHANNELS 16

/* Call counters */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxAdc_isTmadcResultAvailable_callCount;
extern int mock_IfxAdc_clearTmadcResultFlag_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

/* Special additional counter per requirements */
extern uint32 mock_togglePin_callCount;

/* Return value controls for non-void stubs */
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* Value-capture spies */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control APIs */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);
int  mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

/* Required pin symbol externs (also provided in IfxPort_Pinmap.h) */
/* Type comes from IfxEgtm_Pwm.h; duplicate externs allowed */
struct IfxEgtm_Pwm_ToutMap_tag; /* forward tag to allow extern before including header; will be defined as typedef in IfxEgtm_Pwm.h */
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT;
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT;

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
