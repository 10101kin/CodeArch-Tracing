/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned long long uint64;
typedef signed long long sint64;
typedef unsigned char boolean;
typedef sint32 Ifx_Priority;

typedef uint32 Ifx_UReg_32Bit; /* 32-bit register type used in SFR pointer fields */

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
/* ISR macro must take 3 args */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across multiple drivers */
typedef enum { Ifx_ActiveState_low = 0, Ifx_ActiveState_high = 1 } Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3
} IfxSrc_Tos;

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Specific MODULE externs required by this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;

/* Additional port module externs from required stub map */
extern Ifx_P MODULE_P00; extern Ifx_P MODULE_P01; extern Ifx_P MODULE_P02; extern Ifx_P MODULE_P03; extern Ifx_P MODULE_P04; extern Ifx_P MODULE_P10; extern Ifx_P MODULE_P13; extern Ifx_P MODULE_P14; extern Ifx_P MODULE_P15; extern Ifx_P MODULE_P16; extern Ifx_P MODULE_P20; extern Ifx_P MODULE_P21; extern Ifx_P MODULE_P22; extern Ifx_P MODULE_P23; extern Ifx_P MODULE_P25; extern Ifx_P MODULE_P30; extern Ifx_P MODULE_P31; extern Ifx_P MODULE_P32; extern Ifx_P MODULE_P33; extern Ifx_P MODULE_P34; extern Ifx_P MODULE_P35; extern Ifx_P MODULE_P40;

/* Spy API declarations */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif

/* Per-function call counters (extern) */
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;

extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;

extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;

extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;

/* Return-value controls for non-void functions */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* Value-capture spy fields */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Per-function getters for call counters */
int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);

int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
