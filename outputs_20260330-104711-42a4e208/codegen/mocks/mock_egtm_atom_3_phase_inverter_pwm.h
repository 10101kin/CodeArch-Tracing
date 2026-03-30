/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + spy API only
 */
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
typedef unsigned char Ifx_Priority;
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

/* ISR macro: MUST be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3
} IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* Shared config structs used by multiple drivers (single-owner in base header) */
typedef struct { uint32 reserved; } IfxApApu_ApuConfig;
typedef struct { uint32 reserved; } IfxApProt_ProtConfig;

/* MODULE_* register-block stubs (SFR) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Required extern MODULE instances */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;

/* Spy API: counters, return-value controls, captured values */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM driver spies */
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* DT helpers (available for tests even if not used by stubs) */
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* EGTM base driver spies */
extern int      mock_IfxEgtm_isEnabled_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;
extern int      mock_IfxEgtm_enable_callCount;

/* CMU spies (mandatory list + used-by-template) */
extern int      mock_IfxEgtm_Cmu_enable_callCount;
extern int      mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean  mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;

/* IfxPort spies */
extern int      mock_IfxPort_setPinModeOutput_callCount;
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* legacy alias for tests */

/* IfxCpu helpers used by PWM driver */
extern int      mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int      mock_IfxCpu_enableInterrupts_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_togglePin_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
