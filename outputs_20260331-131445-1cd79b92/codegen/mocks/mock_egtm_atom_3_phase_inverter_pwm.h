/*
 * Base mock header for EGTM ATOM 3-Phase Inverter PWM module
 * Provides: base types, macros, shared enums, MODULE_* stubs, spy API declarations
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
typedef uint32              Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit;

/* Macros */
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

/* Interrupt macro (3-arg) */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_none = 0,
    IfxSrc_Tos_cpu0 = 1,
    IfxSrc_Tos_cpu1 = 2,
    IfxSrc_Tos_cpu2 = 3,
    IfxSrc_Tos_dma  = 4
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* EGTM */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

/* EGTM sub-blocks used in driver types */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_ATOM_AGC;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS_CDTM_DTM;

/* Port module */
typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P33;

/* Spy API declarations */
#define MOCK_MAX_CHANNELS 16

/* Spy fields for PWM init/config capture */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty/DT capture */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_dt_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_dt_lastDtFalling[MOCK_MAX_CHANNELS];

/* Additional pin toggle spy */
extern uint32  mock_togglePin_callCount;

/* Call counters and return-value controls for all mocked functions */
extern int mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
int mock_IfxEgtm_isEnabled_getCallCount(void);

extern int mock_IfxEgtm_enable_callCount;
int mock_IfxEgtm_enable_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_setFrequency_callCount;
extern boolean mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_setTrigger_callCount;
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_run_callCount;
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void);

extern int mock_IfxEgtm_Atom_Timer_getPeriod_callCount;
extern uint32 mock_IfxEgtm_Atom_Timer_getPeriod_returnValue;
int mock_IfxEgtm_Atom_Timer_getPeriod_getCallCount(void);

extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);

/* Mandatory CMU extra mocks */
extern int mock_IfxEgtm_Cmu_enable_callCount;
int mock_IfxEgtm_Cmu_enable_getCallCount(void);

extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

extern int mock_IfxEgtm_Pwm_updateChannelDutyImmediate_callCount;
int mock_IfxEgtm_Pwm_updateChannelDutyImmediate_getCallCount(void);

extern int mock_IfxEgtm_Pwm_initConfig_callCount;
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

extern int mock_IfxEgtm_Pwm_init_callCount;
int mock_IfxEgtm_Pwm_init_getCallCount(void);

extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

extern int mock_IfxPort_setPinModeOutput_callCount;
int mock_IfxPort_setPinModeOutput_getCallCount(void);

extern int mock_IfxPort_togglePin_callCount;
int mock_IfxPort_togglePin_getCallCount(void);

extern int mock_IfxEgtm_PinMap_setAtomTout_callCount;
int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void);

/* Mock control */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
