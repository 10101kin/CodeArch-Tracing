/*
 * Base mock layer for EGTM_ATOM_3_Phase_Inverter_PWM
 * Provides base types, shared enums, MODULE_* stubs, and spy API declarations
 */
#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float              float32;
typedef unsigned char      uint8;
typedef signed char        sint8;
typedef unsigned short     uint16;
typedef signed short       sint16;
typedef unsigned int       uint32;
typedef signed int         sint32;
typedef unsigned long long uint64;
typedef signed long long   sint64;
typedef unsigned char      boolean;
typedef unsigned char      Ifx_Priority;

typedef uint32 Ifx_UReg_32Bit; /* 32-bit register type */

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

/* ISR macro (3 args) */
#define IFX_INTERRUPT(isr_name, vectab_num, priority) void isr_name(void)

/* Commonly referenced configuration size macros used by structs in driver mocks */
#ifndef IFXEGTM_NUM_CCM_OBJECTS
# define IFXEGTM_NUM_CCM_OBJECTS 1
#endif
#ifndef IFXPORT_NUM_APU
# define IFXPORT_NUM_APU 1
#endif
#ifndef IFXPORT_NUM_PINS
# define IFXPORT_NUM_PINS 1
#endif

/* Shared enums used across drivers */
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
    IfxSrc_Tos_cpu6 = 6,
    IfxSrc_Tos_cpu7 = 7,
    IfxSrc_Tos_dma  = 8
} IfxSrc_Tos;

/* Virtual machine IDs shared enum */
typedef enum {
    IfxSrc_VmId_0 = 0,
    IfxSrc_VmId_1 = 1,
    IfxSrc_VmId_2 = 2
} IfxSrc_VmId;

/* Minimal SFR register block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_GTM;     /* legacy GTM */
typedef struct { uint32 reserved; } Ifx_EGTM;    /* TC4xx eGTM */
typedef struct { uint32 reserved; } Ifx_EGTM_CLS;/* eGTM Cluster */
typedef struct { uint32 reserved; } Ifx_P;       /* Port */

/* Extern MODULE_* instances (declared; defined in .c as needed) */
extern Ifx_GTM  MODULE_GTM;
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

/* Mock spy API declarations */
#define MOCK_MAX_CHANNELS 16

/* PWM init/initConfig capture */
extern uint32  mock_IfxEgtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency;

/* Duty update spy capture */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];

/* Deadtime capture (kept for test compatibility) */
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Call counters (only for key mocked functions) */
extern int mock_IfxEgtm_Pwm_init_callCount;
extern int mock_IfxEgtm_Pwm_initConfig_callCount;
extern int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxEgtm_enable_callCount;
extern int mock_IfxEgtm_isEnabled_callCount;
extern int mock_IfxEgtm_Cmu_enableClocks_callCount;
extern int mock_IfxEgtm_Cmu_getModuleFrequency_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_enableInterrupts_callCount;

/* Return value controls */
extern boolean mock_IfxEgtm_isEnabled_returnValue;
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue;
extern float32 mock_IfxEgtm_Cmu_getClkFrequency_returnValue;

extern boolean      mock_IfxPort_getPinState_returnValue;
extern unsigned int mock_IfxPort_getGroupState_returnValue;
extern boolean      mock_IfxPort_disableEmergencyStop_returnValue;
extern boolean      mock_IfxPort_enableEmergencyStop_returnValue;
extern unsigned int mock_IfxPort_getRawPinWakeUpStatus_returnValue;
extern boolean      mock_IfxPort_getPinWakeUpStatus_returnValue;
extern unsigned int mock_IfxPort_getIndex_returnValue; /* cast-compatible */
extern unsigned int mock_IfxPort_getPinLVDS_returnValue; /* IfxPort_LvdsMode enum as uint */

/* Toggle pin counter (legacy alias retained) */
extern uint32 mock_togglePin_callCount;

/* Mock control API */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void);
int  mock_IfxEgtm_Pwm_init_getCallCount(void);
int  mock_IfxEgtm_Pwm_initConfig_getCallCount(void);
int  mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int  mock_IfxEgtm_enable_getCallCount(void);
int  mock_IfxEgtm_isEnabled_getCallCount(void);
int  mock_IfxEgtm_Cmu_enableClocks_getCallCount(void);
int  mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxCpu_enableInterrupts_getCallCount(void);

/* Pin symbol stubs (ToutMap externs) used by production code */
/* Fixes previous 'undeclared' error: IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT */
struct IfxEgtm_Pwm_ToutMap_tag; /* forward for extern when including order varies */
extern struct IfxEgtm_Pwm_ToutMap_tag IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT;

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
