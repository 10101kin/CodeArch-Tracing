/*
 * Base mock header for EGTM_ATOM_TMADC_Consolidated
 * Owns: base types, macros, shared enums, MODULE_* stubs, spy API declarations
 */
#ifndef MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H
#define MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H

/* =========================
 * Base Types
 * ========================= */
typedef float               float32;
typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       boolean;      /* iLLD boolean */
typedef unsigned int        Ifx_Priority; /* priority type used in interrupts */

/* =========================
 * Macros
 * ========================= */
#ifndef TRUE
#define TRUE   ((boolean)1)
#endif
#ifndef FALSE
#define FALSE  ((boolean)0)
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void*)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif
/* IFX_INTERRUPT must be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* =========================
 * Shared Enums
 * ========================= */
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

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* =========================
 * MODULE_* Register block stubs (typedef + extern)
 * ========================= */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_ADC;
typedef struct { uint32 reserved; } Ifx_P;

extern Ifx_EGTM MODULE_EGTM;
extern Ifx_ADC  MODULE_ADC;
extern Ifx_P    MODULE_P00;
extern Ifx_P    MODULE_P01;
extern Ifx_P    MODULE_P02;
extern Ifx_P    MODULE_P03;
extern Ifx_P    MODULE_P04;
extern Ifx_P    MODULE_P10;
extern Ifx_P    MODULE_P13;
extern Ifx_P    MODULE_P14;
extern Ifx_P    MODULE_P15;
extern Ifx_P    MODULE_P16;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P21;
extern Ifx_P    MODULE_P22;
extern Ifx_P    MODULE_P23;
extern Ifx_P    MODULE_P25;
extern Ifx_P    MODULE_P30;
extern Ifx_P    MODULE_P31;
extern Ifx_P    MODULE_P32;
extern Ifx_P    MODULE_P33;
extern Ifx_P    MODULE_P34;
extern Ifx_P    MODULE_P35;
extern Ifx_P    MODULE_P40;

/* =========================
 * Spy API Declarations
 * ========================= */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM init/initConfig spies */
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty update spies */
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Deadtime update spies (single and channels) */
extern int      mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_callCount;
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* The tests referenced these exact plural-named arrays previously — provide them */
extern int      mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* CMU and EGTM control spies */
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

extern int      mock_IfxEgtm_enable_callCount;
extern int      mock_IfxEgtm_isEnabled_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;

/* Atom Timer spies */
extern int      mock_IfxEgtm_Atom_Timer_run_callCount;
extern int      mock_IfxEgtm_Atom_Timer_setFrequency_callCount;
extern boolean  mock_IfxEgtm_Atom_Timer_setFrequency_returnValue;
extern int      mock_IfxEgtm_Atom_Timer_setTrigger_callCount;

/* EGTM Trigger to ADC spy */
extern int      mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean  mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* ADC and TMADC spies */
extern int      mock_IfxAdc_enableModule_callCount;
extern int      mock_IfxAdc_Tmadc_initModule_callCount;
extern int      mock_IfxAdc_Tmadc_initModuleConfig_callCount;

/* PinMap setAtomTout spy */
extern int      mock_IfxEgtm_PinMap_setAtomTout_callCount;
extern uint32   mock_togglePin_callCount; /* generic toggle/pin mapping activity */

/* Control helpers for captured counts */
extern uint32   mock_lastCapturedNumChannels; /* internal tracker exposed for tests if needed */

/* Mock control API */
void mock_egtm_atom_tmadc_consolidated_reset(void);

/* Call count getters (one per mocked function with a counter) */
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_run_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_setFrequency_getCallCount(void);
int mock_IfxEgtm_Atom_Timer_setTrigger_getCallCount(void);
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);
int mock_IfxAdc_enableModule_getCallCount(void);
int mock_IfxAdc_Tmadc_initModule_getCallCount(void);
int mock_IfxAdc_Tmadc_initModuleConfig_getCallCount(void);
int mock_IfxEgtm_PinMap_setAtomTout_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_TMADC_CONSOLIDATED_H */
