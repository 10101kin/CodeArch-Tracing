/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + shared enums + spy API only
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef unsigned char      boolean;
typedef unsigned char      uint8;
typedef signed char        sint8;
typedef unsigned short     uint16;
typedef signed short       sint16;
typedef unsigned int       uint32;
typedef signed int         sint32;
typedef float              float32;
typedef uint32             Ifx_Priority;
typedef uint32             Ifx_UReg_32Bit;

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

/* Shared enums used across drivers */
typedef enum
{
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_dma  = 1,
    IfxSrc_Tos_cpu1 = 2
} IfxSrc_Tos;

typedef enum
{
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy API declarations */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* IfxEgtm_Pwm stubs */
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

/* IfxAdc stubs */
extern int      mock_IfxAdc_isTmadcResultAvailable_callCount;
extern boolean  mock_IfxAdc_isTmadcResultAvailable_returnValue;
extern int      mock_IfxAdc_clearTmadcResultFlag_callCount;
int mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);
int mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);

/* IfxEgtm stubs */
extern int      mock_IfxEgtm_isEnabled_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;
extern int      mock_IfxEgtm_enable_callCount;
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);

/* IfxEgtm_Cmu stubs */
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_enable_callCount;
extern int      mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean  mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

/* IfxEgtm_Trigger stubs */
extern int      mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean  mock_IfxEgtm_Trigger_trigToAdc_returnValue;
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

/* IfxPort stubs */
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* duplicate alias for tests */
extern int      mock_IfxPort_setPinModeOutput_callCount;
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);

/* Mock control */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
