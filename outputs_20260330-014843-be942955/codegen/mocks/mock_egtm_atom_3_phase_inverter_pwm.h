#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base types + MODULE stubs + spy API only */
#include <stdint.h>

/* Base type aliases */
typedef float    float32;
typedef uint32_t uint32;
typedef int32_t  sint32;
typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint8_t  boolean;
typedef int32_t  Ifx_Priority;

typedef uint32   Ifx_UReg_32Bit; /* shared register-sized type */

/* Macros */
#ifndef TRUE
#define TRUE  ((boolean)1u)
#endif
#ifndef FALSE
#define FALSE ((boolean)0u)
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
typedef enum
{
    Ifx_ActiveState_activeLow  = 0,
    Ifx_ActiveState_activeHigh = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_cpu6 = 6,
    IfxSrc_Tos_cpu7 = 7
} IfxSrc_Tos;

/* Some shared config holder types referenced by multiple headers (single-owner here) */
typedef struct { uint32 dummy; } IfxApApu_ApuConfig;
typedef struct { uint32 dummy; } IfxApProt_ProtConfig;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* MODULE externs */
extern Ifx_EGTM MODULE_EGTM;
/* Port modules needed as extern Ifx_P instances */
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

/* Spy API externs and controls */
#define MOCK_MAX_CHANNELS 16u

/* IfxPort */
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern uint32 mock_togglePin_callCount; /* required generic toggle counter */
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);

/* IfxEgtm */
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern int     mock_IfxEgtm_enable_callCount;
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);

/* IfxEgtm_Cmu */
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int      mock_IfxEgtm_Cmu_setEclkDivider_callCount;
extern int      mock_IfxEgtm_Cmu_setGclkDivider_callCount;
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setEclkDivider_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkDivider_getCallCount(void);

/* IfxEgtm_Pwm (primary PWM driver) */
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

/* Captured configuration values from init/initConfig (both capture for tests) */
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;

/* Captured duty requests (bounded by numChannels captured in init) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Dead-time spies (fixing previous build errors). Provide both plural and singular names. */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising;
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling;

int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

/* Mock control */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);


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
