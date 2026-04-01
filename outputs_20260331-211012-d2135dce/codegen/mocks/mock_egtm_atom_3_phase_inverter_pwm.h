/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float               float32;
typedef unsigned char       uint8;
typedef signed short        sint16;
typedef unsigned short      uint16;
typedef signed int          sint32;
typedef unsigned int        uint32;
typedef unsigned char       boolean;    /* iLLD boolean */
typedef uint32              Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit; /* register type used in SFR structs */

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
/* IFX_INTERRUPT must be a 3-arg macro */
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums used across multiple drivers */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

/* EGTM cluster SFR type stub (used by PWM driver) */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Port module SFR stub */
typedef struct { uint32 reserved; } Ifx_P;
/* Port modules needed */
extern Ifx_P MODULE_P20;

/* Spy API declarations --------------------------------------------------- */
#define MOCK_MAX_CHANNELS 16

/* Capture fields for primary PWM driver's init() and initConfig() */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Capture fields for duty update */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Capture fields for dead-time updates (even if not used, tests may expect them) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* Generic toggle-pin counter (in addition to function-specific counter) */
extern uint32  mock_togglePin_callCount;

/* Call counters for all stubbed functions */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

/* Return value controls for non-void functions */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* Mock control */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Call-count getters (one per mocked function) */
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
