#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef unsigned char       boolean;
typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef float               float32;
typedef unsigned int        Ifx_Priority;
typedef uint32              Ifx_UReg_32Bit;

/* Macros */
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
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums used across multiple drivers */
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
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 6,
    IfxSrc_Tos_gtm  = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* Common auxiliary config types used by multiple headers */
typedef struct { uint32 reserved; } IfxApApu_ApuConfig;
typedef struct { uint32 reserved; } IfxApProt_ProtConfig;

/* Useful constants used in multiple headers */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
#define IFXEGTM_NUM_CCM_OBJECTS (3)
#endif
#ifndef IFXPORT_NUM_APU
#define IFXPORT_NUM_APU (1)
#endif
#ifndef IFXPORT_NUM_PINS
#define IFXPORT_NUM_PINS (128)
#endif

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Extern MODULE instances actually used by this module */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy counters, return controls, and capture buffers */
#define MOCK_MAX_CHANNELS 16

/* IfxEgtm */
extern int     mock_IfxEgtm_isEnabled_callCount;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern int     mock_IfxEgtm_enable_callCount;

/* IfxEgtm_Cmu */
extern int     mock_IfxEgtm_Cmu_enable_callCount;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern int     mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int     mock_IfxEgtm_Cmu_enableClocks_callCount;

/* IfxPort */
extern int     mock_IfxPort_setPinModeOutput_callCount;
extern uint32  mock_togglePin_callCount;

/* IfxEgtm_Pwm init/config spies */
extern int     mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern int     mock_IfxEgtm_Pwm_init_callCount;
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;

/* Duty update capture (bounded by last-captured numChannels) */
extern int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* DeadTime update capture (bounded) — include both plural and singular spy names to satisfy tests */
extern int     mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount;
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];
/* Backward-compatible aliases (singular) */
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* IfxEgtm_Trigger */
extern int     mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Getters for call counters */
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_togglePin_getCallCount(void);
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void);
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
