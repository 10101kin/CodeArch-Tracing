/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef float               float32;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned char       uint8;
typedef unsigned char       boolean;
typedef uint32              Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit;

#ifndef IFX_CONST
#define IFX_CONST const
#endif

/* Macros */
#ifndef TRUE
#define TRUE   ((boolean)1)
#endif
#ifndef FALSE
#define FALSE  ((boolean)0)
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
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 6,
    IfxSrc_Tos_eru  = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P33;

/* Spy API declarations (counters, returns, capture fields) */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm */
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
int  mock_IfxEgtm_isEnabled_getCallCount(void);

extern int mock_IfxEgtm_enable_callCount;
int  mock_IfxEgtm_enable_getCallCount(void);

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);

/* Mandatory CMU additional stubs */
extern int     mock_IfxEgtm_Cmu_enable_callCount;
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);

extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

extern int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);

extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

/* IfxEgtm_Trigger */
extern int     mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

/* IfxPort */
extern int mock_IfxPort_togglePin_callCount;
extern uint32 mock_togglePin_callCount; /* additional simple counter */
int  mock_IfxPort_togglePin_getCallCount(void);

extern int mock_IfxPort_setPinModeOutput_callCount;
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

/* IfxEgtm_Pwm */
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

extern int mock_IfxEgtm_Pwm_init_callCount;
int  mock_IfxEgtm_Pwm_init_getCallCount(void);

extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxAdc */
extern int mock_IfxAdc_clearTmadcResultFlag_callCount;
int  mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);

extern int mock_IfxAdc_enableModule_callCount;
int  mock_IfxAdc_enableModule_getCallCount(void);

extern int     mock_IfxAdc_isTmadcResultAvailable_callCount;
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;
int  mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);

/* IfxAdc_Tmadc */
extern int mock_IfxAdc_Tmadc_runModule_callCount;
int  mock_IfxAdc_Tmadc_runModule_getCallCount(void);

/* Mock control */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
