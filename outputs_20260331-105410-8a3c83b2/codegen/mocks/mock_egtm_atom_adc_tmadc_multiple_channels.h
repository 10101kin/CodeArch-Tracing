/*
 * Base mock header for EGTM_ATOM_ADC_TMADC_Multiple_Channels
 * Owns: base types, macros, shared enums, MODULE_* SFR stubs, and spy API declarations only.
 */
#ifndef MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H
#define MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H

/* Base type aliases */
typedef float               float32;
typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short      uint16;
typedef signed short        sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned char       boolean;
typedef uint32              Ifx_Priority;

typedef uint32              Ifx_UReg_32Bit; /* SFR register 32-bit type */

/* Macros */
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

/* MODULE_* register-block stubs (typedef + extern) */
/* Minimal set required by this module + a GTM example */
typedef struct { uint32 reserved; } Ifx_EGTM;
extern Ifx_EGTM MODULE_EGTM;

typedef struct { uint32 reserved; } Ifx_P;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P33;

/* Optional GTM legacy example stub */
typedef struct { uint32 reserved; } Ifx_GTM;
extern Ifx_GTM MODULE_GTM;

/* Also provide EGTM CLS stub used by PWM driver */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;

/* Spy counters, return-value controls, and capture buffers */
#define MOCK_MAX_CHANNELS 16

/* Primary PWM init/initConfig spies */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty and dead-time capture arrays */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelDeadTime_lastDtFalling[MOCK_MAX_CHANNELS];

/* Generic toggle spy */
extern uint32 mock_togglePin_callCount;

/* Call counters for mocked functions */
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_enable_callCount;
extern int mock_IfxEgtm_Cmu_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxEgtm_Cmu_setClkFrequency_callCount;

extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;

extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;

extern int mock_IfxPort_setPinModeOutput_callCount;

/* Return value controls for non-void stubs */
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern boolean mock_IfxEgtm_Cmu_isEnabled_returnValue;

/* Mock control API */
void mock_egtm_atom_adc_tmadc_multiple_channels_reset(void);

/* Per-function getters for call counts */
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxEgtm_Cmu_enable_getCallCount(void);
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void);

int mock_IfxEgtm_enable_getCallCount(void);
int mock_IfxEgtm_isEnabled_getCallCount(void);

int mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int mock_IfxEgtm_Pwm_init_getCallCount(void);
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

int mock_IfxPort_setPinModeOutput_getCallCount(void);
int mock_togglePin_getCallCount(void);

#endif /* MOCK_EGTM_ATOM_ADC_TMADC_MULTIPLE_CHANNELS_H */
