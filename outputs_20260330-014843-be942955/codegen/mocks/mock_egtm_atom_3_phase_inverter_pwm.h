/*
 * Base mock for EGTM_ATOM_3_Phase_Inverter_PWM
 * Owns: base types, macros, shared enums, MODULE_* stubs, spy API declarations only
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base types */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char boolean;
typedef unsigned char Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE ((boolean)1)
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

/* Shared enums */
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 6,
    IfxSrc_Tos_scr  = 7
} IfxSrc_Tos;

/* Common register unit type used in iLLD */
typedef uint32 Ifx_UReg_32Bit;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS; /* Cluster SFR stub type used by IfxEgtm_Pwm */
/* Required EGTM module extern */
extern Ifx_EGTM MODULE_EGTM;

/* Port module externs (as Ifx_P instances) */
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P40;
extern Ifx_P MODULE_P41;

/* Spy API declarations */
#define MOCK_MAX_CHANNELS 16

/* Call counters (extern) */
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern uint32 mock_togglePin_callCount; /* compatibility alias */

extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;

extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setEclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_setGclkDivider_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;

/* Return-value controls */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

/* Value-capture spy fields */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Dead-time spy arrays (kept for test compatibility) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

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
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getClkFrequency_getCallCount(void);
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
