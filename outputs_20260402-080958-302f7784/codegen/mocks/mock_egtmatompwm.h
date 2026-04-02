#ifndef MOCK_EGTMATOMPWM_H
#define MOCK_EGTMATOMPWM_H

/* Base types */
typedef unsigned char boolean;
typedef unsigned char uint8;
typedef signed char sint8;
typedef unsigned short uint16;
typedef signed short sint16;
typedef unsigned int uint32;
typedef signed int sint32;
typedef unsigned long long uint64;
typedef signed long long sint64;
typedef float float32;
typedef double float64;
typedef uint32 Ifx_Priority;
typedef uint32 Ifx_UReg_32Bit;

/* Macros */
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
/* IFX_INTERRUPT must be 3-arg */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Shared enums used across drivers */
typedef enum {
    Ifx_ActiveState_activeLow = 0,
    Ifx_ActiveState_activeHigh = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_cpu4 = 4,
    IfxSrc_Tos_cpu5 = 5,
    IfxSrc_Tos_dma  = 6,
    IfxSrc_Tos_erp  = 7
} IfxSrc_Tos;

typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P33;

/* Additional SFR type stubs often referenced */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

typedef struct { uint32 reserved; } Ifx_ADC;            extern Ifx_ADC MODULE_ADC;
typedef struct { uint32 reserved; } Ifx_PMS;            extern Ifx_PMS MODULE_PMS;
typedef struct { uint32 reserved; } Ifx_SCU;            extern Ifx_SCU MODULE_SCU;
typedef struct { uint32 reserved; } Ifx_SRC;            extern Ifx_SRC MODULE_SRC;
typedef struct { uint32 reserved; } Ifx_CLOCK;          extern Ifx_CLOCK MODULE_CLOCK;
/* (Add more module externs here as needed by tests) */

/* Spy API storage (extern) */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM spies */
extern int      mock_IfxEgtm_Pwm_init_callCount;
extern uint32   mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_init_lastFrequency;
extern int      mock_IfxEgtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxEgtm_Pwm_initConfig_lastFrequency;
extern int      mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
/* dead-time capture arrays (available for tests if needed) */
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* CMU spies */
extern int      mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int      mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern int      mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_setClkFrequency_callCount;
extern int      mock_IfxEgtm_Cmu_enable_callCount;
extern int      mock_IfxEgtm_Cmu_isEnabled_callCount;
extern boolean  mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* EGTM enable/status spies */
extern int      mock_IfxEgtm_enable_callCount;
extern int      mock_IfxEgtm_isEnabled_callCount;
extern boolean  mock_IfxEgtm_isEnabled_returnValue;

/* Trigger spies */
extern int      mock_IfxEgtm_Trigger_trigToAdc_callCount;
extern boolean  mock_IfxEgtm_Trigger_trigToAdc_returnValue;

/* Port spies */
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* legacy name required by tests */
extern int      mock_IfxPort_setPinModeOutput_callCount;

/* CPU/IRQ spies */
extern int      mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int      mock_IfxCpu_enableInterrupts_callCount;

/* Mock control API */
void mock_egtmatompwm_reset(void);

int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);
int  mock_IfxEgtm_Cmu_enable_getCallCount(void);
int  mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);

int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);

int  mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void);

int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);

int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

#endif /* MOCK_EGTMATOMPWM_H */
