/* Base types + shared enums + MODULE stubs + spy API only */
#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float                float32;
typedef unsigned char        uint8;
typedef signed char          sint8;
typedef unsigned short       uint16;
typedef signed short         sint16;
typedef unsigned int         uint32;
typedef signed int           sint32;
typedef unsigned char        boolean;
typedef unsigned char        Ifx_Priority;

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
#ifndef IFX_INTERRUPT
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)
#endif

/* Shared enums used across drivers */
typedef enum
{
    Ifx_ActiveState_low  = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum
{
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_dma  = 4
} IfxSrc_Tos;

typedef enum
{
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_P;

/* Required module externs */
extern Ifx_GTM MODULE_GTM;
extern Ifx_P   MODULE_P00;
extern Ifx_P   MODULE_P01;
extern Ifx_P   MODULE_P02;
extern Ifx_P   MODULE_P10;
extern Ifx_P   MODULE_P11;
extern Ifx_P   MODULE_P12;
extern Ifx_P   MODULE_P13;
extern Ifx_P   MODULE_P14;
extern Ifx_P   MODULE_P15;
extern Ifx_P   MODULE_P20;
extern Ifx_P   MODULE_P21;
extern Ifx_P   MODULE_P22;
extern Ifx_P   MODULE_P23;
extern Ifx_P   MODULE_P24;
extern Ifx_P   MODULE_P25;
extern Ifx_P   MODULE_P26;
extern Ifx_P   MODULE_P30;
extern Ifx_P   MODULE_P31;
extern Ifx_P   MODULE_P32;
extern Ifx_P   MODULE_P33;
extern Ifx_P   MODULE_P34;
extern Ifx_P   MODULE_P40;
extern Ifx_P   MODULE_P41;

/* Spy API: counters, return values, captured values */
#define MOCK_MAX_CHANNELS 16

/* IfxGtm_Cmu spies */
extern int      mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_enableClocks_callCount;
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);

extern int      mock_IfxGtm_Cmu_setGclkFrequency_callCount;
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_setClkFrequency_callCount;
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_enable_callCount;
int mock_IfxGtm_Cmu_enable_getCallCount(void);

extern int      mock_IfxGtm_Cmu_isEnabled_callCount;
extern boolean  mock_IfxGtm_Cmu_isEnabled_returnValue;
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void);

/* Additional CMU getters (control values + counters) */
extern int      mock_IfxGtm_Cmu_getClkFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getClkFrequency_returnValue;
int mock_IfxGtm_Cmu_getClkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_getEclkFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getEclkFrequency_returnValue;
int mock_IfxGtm_Cmu_getEclkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_getFxClkFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getFxClkFrequency_returnValue;
int mock_IfxGtm_Cmu_getFxClkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_getGclkFrequency_callCount;
extern float32  mock_IfxGtm_Cmu_getGclkFrequency_returnValue;
int mock_IfxGtm_Cmu_getGclkFrequency_getCallCount(void);

extern int      mock_IfxGtm_Cmu_isClkClockEnabled_callCount;
extern boolean  mock_IfxGtm_Cmu_isClkClockEnabled_returnValue;
int mock_IfxGtm_Cmu_isClkClockEnabled_getCallCount(void);

extern int      mock_IfxGtm_Cmu_isEclkClockEnabled_callCount;
extern boolean  mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue;
int mock_IfxGtm_Cmu_isEclkClockEnabled_getCallCount(void);

extern int      mock_IfxGtm_Cmu_isFxClockEnabled_callCount;
extern boolean  mock_IfxGtm_Cmu_isFxClockEnabled_returnValue;
int mock_IfxGtm_Cmu_isFxClockEnabled_getCallCount(void);

extern int      mock_IfxGtm_Cmu_selectClkInput_callCount;
int mock_IfxGtm_Cmu_selectClkInput_getCallCount(void);

extern int      mock_IfxGtm_Cmu_setEclkFrequency_callCount;
int mock_IfxGtm_Cmu_setEclkFrequency_getCallCount(void);

/* IfxGtm base spies */
extern int      mock_IfxGtm_isEnabled_callCount;
extern boolean  mock_IfxGtm_isEnabled_returnValue;
int mock_IfxGtm_isEnabled_getCallCount(void);

extern int      mock_IfxGtm_enable_callCount;
int mock_IfxGtm_enable_getCallCount(void);

/* IfxPort spies */
extern int      mock_IfxPort_togglePin_callCount;
extern uint32   mock_togglePin_callCount; /* dedicated alias counter required */
int mock_IfxPort_togglePin_getCallCount(void);

extern int      mock_IfxPort_setPinModeOutput_callCount;
int mock_IfxPort_setPinModeOutput_getCallCount(void);

/* IfxGtm_Pwm spies and captured values */
extern int      mock_IfxGtm_Pwm_initConfig_callCount;
extern uint32   mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32  mock_IfxGtm_Pwm_initConfig_lastFrequency;
int mock_IfxGtm_Pwm_initConfig_getCallCount(void);

extern int      mock_IfxGtm_Pwm_init_callCount;
extern uint32   mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32  mock_IfxGtm_Pwm_init_lastFrequency;
int mock_IfxGtm_Pwm_init_getCallCount(void);

extern int      mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern float32  mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);

/* Optional DT capture arrays for completeness */
extern float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32  mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
