/* mock_egtm_atom_3_phase_inverter_pwm.h - Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float               float32;
typedef unsigned int        uint32;
typedef int                 sint32;
typedef unsigned short      uint16;
typedef short               sint16;
typedef unsigned char       uint8;
typedef unsigned char       boolean;
typedef uint8               Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit; /* SFR-sized register type used in iLLD */

/* Common macros */
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
#ifndef IFX_INTERRUPT
# define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums (cross-driver) */
typedef enum {
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1
} IfxSrc_Tos;

/* Shared SRC VM Id (keep simple) */
typedef uint8 IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* TC4xx EGTM */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

/* EGTM Cluster SFR (used by PWM driver state) */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* PORT module SFR type and required instances */
typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;

/* Spy counters and return-value controls for all mocked functions */
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern int     mock_IfxEgtm_enable_callCount;

extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

extern int     mock_IfxPort_togglePin_callCount;
extern int     mock_togglePin_callCount; /* compatibility alias */
extern int     mock_IfxPort_setPinModeOutput_callCount;

/* Spy getters (one per mocked function) */
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxPort_togglePin_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);

/* Value-capture spy fields */
#define MOCK_MAX_CHANNELS 16
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
