/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + spy API only (no driver-specific structs/functions here)
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base scalar types */
typedef float           float32;
typedef unsigned int    uint32;
typedef int             sint32;
typedef unsigned short  uint16;
typedef short           sint16;
typedef unsigned char   uint8;
typedef unsigned char   boolean;
typedef unsigned char   Ifx_Priority;
typedef uint32          Ifx_UReg_32Bit;

/* Macros */
#ifndef TRUE
# define TRUE   ((boolean)1)
#endif
#ifndef FALSE
# define FALSE  ((boolean)0)
#endif
#ifndef NULL_PTR
# define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
# define IFX_STATIC static
#endif

/* IFX_INTERRUPT must be 3-arg and expand to a void function with no args */
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_dma  = 2,
    IfxSrc_Tos_none = 3
} IfxSrc_Tos;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Extern instances actually referenced by this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;

/* Spy API / counters / return-value controls */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm_Pwm stubs */
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* DT spy fields (reserved for potential DT update function tests) */
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxPort stubs */
extern int      mock_IfxPort_setPinModeOutput_callCount;
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* generic alias for quick assertions */

/* IfxEgtm base stubs */
extern int      mock_IfxEgtm_isEnabled_callCount;
extern int      mock_IfxEgtm_enable_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu stubs */
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32  mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32  mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* IfxCpu IRQ helpers (used by PWM driver setup sometimes) */
extern int      mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int      mock_IfxCpu_enableInterrupts_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);

int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);

int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);

int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);

int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
