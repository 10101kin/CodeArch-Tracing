#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef float        float32;
typedef unsigned char boolean;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short  sint16;
typedef unsigned int  uint32;
typedef signed int    sint32;
typedef unsigned int  Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE  (1)
#endif
#ifndef FALSE
#define FALSE (0)
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

/* Shared enums used across multiple drivers */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma = 6,
    IfxSrc_Tos_gtm = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* SFR-sized register base type used by iLLD */
typedef uint32 Ifx_UReg_32Bit;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

extern Ifx_EGTM MODULE_EGTM;  /* eGTM module instance */
extern Ifx_P    MODULE_P20;   /* Port module instances used by production */
extern Ifx_P    MODULE_P21;

/* ================= Spy API: counters, return-value controls, and captured values ================ */

/* Sizes */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* IfxPort functions */
extern int     mock_IfxPort_togglePin_callCount;
extern uint32  mock_togglePin_callCount; /* dedicated alias counter */
extern int     mock_IfxPort_setPinModeOutput_callCount;

/* IfxEgtm (enable/status) */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu (clock management) */
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue; /* if 0.0f, stub returns default 100e6 */

/* IfxEgtm_Pwm (primary PWM driver) */
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

/* Captured config values from init/initConfig */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Captured duty/deadtime arrays from update calls */
extern float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_lastDtFalling[MOCK_MAX_CHANNELS];

/* CPU/IRQ helpers (often used by production init) */
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

/* ============== Mock control API (reset + getters for every callCount) ============== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int mock_IfxPort_togglePin_getCallCount(void);
int mock_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);

int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);

int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
