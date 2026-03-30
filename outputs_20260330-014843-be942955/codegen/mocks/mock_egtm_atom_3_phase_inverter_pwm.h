/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + spy API (single-owner base layer)
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases (single-owner) */
typedef float              float32;
typedef unsigned char      uint8;
typedef signed int         sint32;
typedef unsigned int       uint32;
typedef unsigned short     uint16;
typedef signed short       sint16;
typedef unsigned char      boolean;
typedef unsigned char      Ifx_Priority; /* priority type */
typedef uint32             Ifx_UReg_32Bit; /* 32-bit register image */

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
/* ISR macro must be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums (single-owner) */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5
} IfxSrc_Tos;

/* MODULE_* register block stubs (typedef + extern) */
/* Generic peripheral SFR block stubs used by this module */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Required module externs */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P00;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P10;
extern Ifx_P    MODULE_P13;
extern Ifx_P    MODULE_P14;
extern Ifx_P    MODULE_P15;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;
extern Ifx_P    MODULE_P22;
extern Ifx_P    MODULE_P23;
extern Ifx_P    MODULE_P33;
extern Ifx_P    MODULE_P34;
extern Ifx_P    MODULE_P40;
extern Ifx_P    MODULE_P41;

/* Spy buffers and controls */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* IfxEgtm_Pwm init/initConfig capture */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty update capture */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Deadtime capture (available for tests even if not used by this module) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Additional spy counter for generic pin toggling */
extern uint32  mock_togglePin_callCount;

/* Call counters (one per stubbed function) */
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setEclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_setGclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;

/* Return-value controls for non-void stubs */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Getters for call counts */
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);


/* ── Auto-injected missing declarations ── */
/* Missing: MODULE_P01 of type Ifx_P */
extern Ifx_P MODULE_P01;
/* Missing: MODULE_P03 of type Ifx_P */
extern Ifx_P MODULE_P03;
/* Missing: MODULE_P04 of type Ifx_P */
extern Ifx_P MODULE_P04;
/* Missing: MODULE_P16 of type Ifx_P */
extern Ifx_P MODULE_P16;
/* Missing: MODULE_P25 of type Ifx_P */
extern Ifx_P MODULE_P25;
/* Missing: MODULE_P30 of type Ifx_P */
extern Ifx_P MODULE_P30;
/* Missing: MODULE_P31 of type Ifx_P */
extern Ifx_P MODULE_P31;
/* Missing: MODULE_P32 of type Ifx_P */
extern Ifx_P MODULE_P32;
/* Missing: MODULE_P35 of type Ifx_P */
extern Ifx_P MODULE_P35;

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
