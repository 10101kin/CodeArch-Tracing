/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef unsigned char boolean;
typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;
typedef float float32;
typedef uint8 Ifx_Priority;
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
typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 6,
    IfxSrc_Tos_gtm  = 7
} IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

typedef struct { uint32 reserved; } Ifx_ADC;
extern Ifx_ADC MODULE_ADC;

typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P01;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P03; /* required */
extern Ifx_P MODULE_P04;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P16;
extern Ifx_P MODULE_P20; /* required */
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P30;
extern Ifx_P MODULE_P31;
extern Ifx_P MODULE_P32;
extern Ifx_P MODULE_P33; /* required */
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P35;
extern Ifx_P MODULE_P40;

/* Spy API - counters, return controls, captures */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* IfxEgtm_Pwm */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* Dead-time capture placeholders for completeness */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxAdc_Tmadc */
extern int mock_IfxAdc_Tmadc_initModuleConfig_callCount;
extern int mock_IfxAdc_Tmadc_initModule_callCount;
extern int mock_IfxAdc_Tmadc_initChannelConfig_callCount;
extern int mock_IfxAdc_Tmadc_initChannel_callCount;
extern int mock_IfxAdc_Tmadc_runModule_callCount;
extern int mock_IfxAdc_Tmadc_enableChannelEvent_callCount;

/* IfxEgtm_Cmu */
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* IfxEgtm_Trigger */
extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* IfxEgtm */
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxPort */
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;

/* CPU */
extern int mock_IfxCpu_enableInterrupts_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;

/* Value-capture spy fields */
extern uint32  mock_initEgtmAtom_lastNumChannels; /* legacy alias if tests look for it */
extern float32 mock_initEgtmAtom_lastFrequency;   /* legacy alias */
extern uint32  mock_togglePin_callCount;          /* mirror of IfxPort_togglePin */

/* Spy getters */
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);
int mock_IfxAdc_Tmadc_initModule_getCallCount(void);
int mock_IfxAdc_Tmadc_initChannelConfig_getCallCount(void);
int mock_IfxAdc_Tmadc_initChannel_getCallCount(void);
int mock_IfxAdc_Tmadc_runModule_getCallCount(void);
int mock_IfxAdc_Tmadc_enableChannelEvent_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

/* Mock control */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
