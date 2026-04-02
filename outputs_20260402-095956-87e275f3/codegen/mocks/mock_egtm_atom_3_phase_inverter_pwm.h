/*
 * Base mock header for EGTM_ATOM_3_Phase_Inverter_PWM
 * - Owns base types, macros, shared enums, MODULE_* SFR stubs, and spy API only
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef float               float32;
typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       boolean;
typedef uint8               Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit;

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

/* Shared enums used across drivers */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;
typedef enum { IfxSrc_Tos_cpu0 = 0, IfxSrc_Tos_cpu1 = 1, IfxSrc_Tos_cpu2 = 2, IfxSrc_Tos_dma = 3 } IfxSrc_Tos;
typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* SFR register block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS; /* cluster SFR stub */
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_ADC_TMADC;
typedef struct { uint32 reserved; } Ifx_P;

/* Additional SFR module typedefs (stubs) — provide as needed by build */
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_WTU;

/* MODULE_* extern instances (at least those used by production) */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* ===================== Spy API (counters, return controls, captures) ===================== */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM init/initConfig spies */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty/DT capture spies */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Call counters */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxAdc_isTmadcResultAvailable_callCount;
extern int mock_IfxAdc_clearTmadcResultFlag_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;

/* Return-value controls */
extern boolean mock_IfxAdc_isTmadcResultAvailable_returnValue;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue; /* if 0.0f, stub returns default 100 MHz */

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxAdc_isTmadcResultAvailable_getCallCount(void);
int  mock_IfxAdc_clearTmadcResultFlag_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
