/*
 * Base mock header for EGTM ATOM 3-Phase Inverter PWM
 * Owns: base types, macros, shared enums, MODULE_* stubs, spy externs, mock control API decls
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* =====================
 * Base type aliases
 * ===================== */
typedef float               float32;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned char       boolean;   /* iLLD boolean */
typedef uint32              Ifx_Priority;  /* generic priority type */
typedef uint32              Ifx_UReg_32Bit; /* used as 32-bit register type */

/* =====================
 * Macros
 * ===================== */
#ifndef TRUE
#define TRUE   (1u)
#endif
#ifndef FALSE
#define FALSE  (0u)
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif
/* 3-arg ISR macro per requirement */
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* =====================
 * Shared enums
 * ===================== */
typedef enum
{
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 8
} IfxSrc_Tos;

typedef enum
{
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* =====================
 * MODULE_* register-block stubs (typedef + extern)
 * ===================== */
/* EGTM module */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

/* EGTM cluster SFR type referenced by PWM types */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Port module stub */
typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P01;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P03;
extern Ifx_P MODULE_P04;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P16;
extern Ifx_P MODULE_P20; /* mandatory */
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P30;
extern Ifx_P MODULE_P31;
extern Ifx_P MODULE_P32;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P35;
extern Ifx_P MODULE_P40;

/* =====================
 * Spy counters, return controls, capture fields
 * ===================== */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* IfxEgtm APIs */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu APIs */
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* IfxEgtm_Pwm APIs */
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxPort APIs */
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern int     mock_IfxPort_togglePin_callCount;
extern uint32  mock_togglePin_callCount; /* generic toggle spy per requirement */

/* IfxCpu APIs */
extern int     mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int     mock_IfxCpu_enableInterrupts_callCount;

/* =====================
 * Mock control API declarations (getters + reset)
 * ===================== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);

int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
