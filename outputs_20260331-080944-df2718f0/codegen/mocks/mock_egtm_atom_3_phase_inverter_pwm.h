/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float               float32;
typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned long long  uint64;
typedef signed long long    sint64;
typedef unsigned char       boolean;
typedef uint8               Ifx_Priority;

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
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums */
typedef enum {
    Ifx_ActiveState_low  = 0,
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
    IfxSrc_Tos_cpu6 = 7,
    IfxSrc_Tos_cpu7 = 8,
    IfxSrc_Tos_dma  = 9
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;

/* Spy counters and return-value controls */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* IfxEgtm_Pwm */
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int     mock_IfxEgtm_Pwm_updateDeadtime_callCount;
extern boolean mock_IfxEgtm_Pwm_init_returnValue;

extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateDeadtime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateDeadtime_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxPort */
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Getters for call counts */
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateDeadtime_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
