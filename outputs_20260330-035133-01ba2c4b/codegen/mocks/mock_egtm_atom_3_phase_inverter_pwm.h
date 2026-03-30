/*
 * mock_egtm_atom_3_phase_inverter_pwm.h
 * Base types + MODULE stubs + spy API only
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* ================= Base type aliases ================= */
typedef float  float32;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char boolean;
typedef unsigned int Ifx_Priority;
typedef uint32 Ifx_UReg_32Bit; /* SFR base unit */

typedef uint32 IfxSrc_VmId; /* simple scalar in mock */

/* Shared AP/PROT config base types (single-owner) */
typedef struct { uint32 reserved; } IfxApApu_ApuConfig;
typedef struct { uint32 reserved; } IfxApProt_ProtConfig;

/* ================= Macros ================= */
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

/* IFX_INTERRUPT must be 3-arg and return void */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* ================= Shared enums ================= */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_dma  = 3,
    IfxSrc_Tos_gtm  = 4,
    IfxSrc_Tos_scr  = 5
} IfxSrc_Tos;

/* ================= MODULE_* register-block stubs ================= */
/* EGTM module (TC4xx) */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

/* Port register block shared across all Px modules */
typedef struct { uint32 reserved; } Ifx_P;

/* Only the needed port modules as extern instances for this module */
extern Ifx_P MODULE_P03;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;

/* ================= Spy API declarations ================= */
#define MOCK_MAX_CHANNELS 16u

/* Primary PWM init spies (capture numChannels and frequency) */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_init_callCount;
int  mock_IfxEgtm_Pwm_init_getCallCount(void);

extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_callCount;
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);

/* Duty update spies (bounded copy) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern uint32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

/* Optional dead-time capture arrays (kept for test compatibility) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxPort spies */
extern uint32 mock_IfxPort_setPinModeOutput_callCount;
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

extern uint32 mock_IfxPort_togglePin_callCount;
extern uint32 mock_togglePin_callCount; /* compatibility alias */
int  mock_IfxPort_togglePin_getCallCount(void);

/* IfxEgtm enable/status spies */
extern uint32  mock_IfxEgtm_enable_callCount;
int   mock_IfxEgtm_enable_getCallCount(void);
extern uint32  mock_IfxEgtm_isEnabled_callCount;
int   mock_IfxEgtm_isEnabled_getCallCount(void);
extern boolean mock_IfxEgtm_isEnabled_returnValue;

/* CMU spies + return-value controls */
extern uint32  mock_IfxEgtm_Cmu_enableClocks_callCount;
int   mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

extern uint32  mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
int   mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);

extern uint32  mock_IfxEgtm_Cmu_setClkFrequency_callCount;
int   mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

extern uint32  mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
int   mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;

/* Additional mandatory CMU enable/status */
extern uint32  mock_IfxEgtm_Cmu_enable_callCount;
int   mock_IfxEgtm_Cmu_enable_getCallCount(void);
extern uint32  mock_IfxEgtm_Cmu_isEnabled_callCount;
int   mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* ================= Mock control ================= */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
