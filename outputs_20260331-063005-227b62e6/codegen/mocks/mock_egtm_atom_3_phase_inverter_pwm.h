/*
 * Base mock for EGTM_ATOM_3_Phase_Inverter_PWM
 * - Provides base types, shared enums, MODULE_* stubs, and spy API declarations
 * - Does NOT contain driver-specific structs or function prototypes
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float               float32;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned char       boolean;
typedef uint8               Ifx_Priority;

typedef uint32              Ifx_UReg_32Bit; /* generic 32-bit register type */

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
/* IFX_INTERRUPT must be 3-arg macro */
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums used across multiple drivers */
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

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_P;

/* Required MODULE stubs for this module */
extern Ifx_EGTM MODULE_EGTM;
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
extern Ifx_P MODULE_P20;
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

/*
 * Spy counters, return-value controls, and captured values
 * For EVERY stubbed function, we provide a callCount and (if non-void) a returnValue control.
 */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm enable/status */
extern int     mock_IfxEgtm_enable_callCount;
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* IfxEgtm_Cmu clock functions */
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* IfxEgtm_Pwm init/config/update */
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

extern int     mock_IfxEgtm_Pwm_init_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;

extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Optional dead-time capture arrays for completeness */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxPort stubs */
extern int     mock_IfxPort_togglePin_callCount;
extern uint32  mock_togglePin_callCount; /* generic counter alias requested */
extern int     mock_IfxPort_setPinModeOutput_callCount;

/* Mock control API (getters + reset) */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
