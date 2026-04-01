/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned long uint32;
typedef signed long sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned char boolean;
typedef unsigned int Ifx_Priority;

typedef uint32 Ifx_UReg_32Bit;

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
    IfxSrc_Tos_dma  = 3
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs */
typedef struct { uint32 reserved; } Ifx_EGTM;
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;
typedef struct { uint32 reserved; } Ifx_P;

/* Required extern MODULE instances (definitions provided in .c) */
extern Ifx_EGTM MODULE_EGTM;
extern Ifx_P    MODULE_P20;
extern Ifx_P    MODULE_P33;

/* Spy/Mock control API declarations */
#define MOCK_MAX_CHANNELS 16

/* Call counters (extern) */
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_enable_callCount;

extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_getGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;

extern int mock_IfxPort_setPinModeOutput_callCount;

extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount;

extern int mock_IfxEgtm_Trigger_trigToAdc_callCount;

extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

extern uint32 mock_togglePin_callCount;

/* Return-value control (extern) */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue; /* 0.0f means use default 100 MHz */
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;   /* 0.0f means use default 100 MHz */
extern boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* Value-capture spies for PWM init/initConfig and updates */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);

/* Getters for call counters */
int mock_IfxEgtm_isEnabled_getCallCount(void);
int mock_IfxEgtm_enable_getCallCount(void);

int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);

int mock_IfxPort_setPinModeOutput_getCallCount(void);

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void);

int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
