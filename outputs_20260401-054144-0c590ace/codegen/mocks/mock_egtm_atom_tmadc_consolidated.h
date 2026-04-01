/*
 * mock_egtm_atom_tmadc_consolidated.h
 * Base types + MODULE stubs + shared enums + spy API only.
 */
#ifndef MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H
#define MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H

/* Base type aliases */
typedef float            float32;
typedef unsigned char    uint8;
typedef signed short     sint16;
typedef unsigned short   uint16;
typedef signed int       sint32;
typedef unsigned int     uint32;
typedef unsigned char    boolean;
typedef unsigned int     Ifx_Priority;
typedef uint32           Ifx_UReg_32Bit;

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

/* IFX_INTERRUPT must be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across drivers */
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_none = 0,
    IfxSrc_Tos_cpu0 = 1,
    IfxSrc_Tos_cpu1 = 2,
    IfxSrc_Tos_cpu2 = 3,
    IfxSrc_Tos_cpu3 = 4,
    IfxSrc_Tos_cpu4 = 5,
    IfxSrc_Tos_cpu5 = 6,
    IfxSrc_Tos_dma  = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

/* Extern module instances actually used by this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* =====================
 * Spy counters & controls
 * ===================== */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM driver spies */
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Atom Timer spies */
extern int      mock_IfxEgtm_Atom_Timer_setFrequency_callCount;
extern boolean  mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
extern int      mock_IfxEgtm_Atom_Timer_setTrigger_callCount;
extern int      mock_IfxEgtm_Atom_Timer_run_callCount;

/* CMU spies */
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* EGTM enable/status spies */
extern int      mock_IfxEgtm_enable_callCount;
extern int      mock_IfxEgtm_isEnabled_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;

/* Trigger-to-ADC spies */
extern int      mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean  mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* TMADC spies */
extern int      mock_IfxAdc_Tmadc_initModule_callCount;
extern int      mock_IfxAdc_Tmadc_initModuleConfig_callCount;

/* ADC module spies */
extern int      mock_IfxAdc_enableModule_callCount;

/* PinMap spies */
extern int      mock_IfxEgtm_PinMap_setAtomTout_callCount;

/* CPU/IRQ spies */
extern int      mock_IfxCpu_enableInterrupts_callCount;
extern int      mock_IfxCpu_Irq_installInterruptHandler_callCount;

/* Misc generic spy */
extern uint32   mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_tmadc_consolidated_reset(void);

int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void);
int  mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void);
int  mock_IfxEgtm_Atom_Timer_run_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int  mock_IfxAdc_Tmadc_initModule_getCallCount(void);
int  mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);
int  mock_IfxAdc_enableModule_getCallCount(void);
int  mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H */
