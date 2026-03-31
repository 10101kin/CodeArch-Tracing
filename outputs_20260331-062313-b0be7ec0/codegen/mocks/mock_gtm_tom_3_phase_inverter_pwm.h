/* Base types + MODULE stubs + spy API only */
#ifndef MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H
#define MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Base type aliases */
typedef float float32;
typedef unsigned int uint32;
typedef int sint32;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef short sint16;
typedef unsigned char boolean;
typedef uint8 Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE ((boolean)1)
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

/* Shared enums */
typedef enum
{
    Ifx_ActiveState_low = 0,
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

typedef enum { IfxSrc_VmId_0 = 0, IfxSrc_VmId_1 = 1, IfxSrc_VmId_2 = 2 } IfxSrc_VmId;

/* MODULE_* register-block stubs (typedef + extern) */
/* Minimal SFR type stubs */
typedef struct { uint32 reserved; } Ifx_ASCLIN;
typedef struct { uint32 reserved; } Ifx_CAN;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CCU6;
typedef struct { uint32 reserved; } Ifx_CONVCTRL;
typedef struct { uint32 reserved; } Ifx_CPU;
typedef struct { uint32 reserved; } Ifx_DAM;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM;
typedef struct { uint32 reserved; } Ifx_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY;
typedef struct { uint32 reserved; } Ifx_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_GPT120;
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_HSCT;
typedef struct { uint32 reserved; } Ifx_HSSL;
typedef struct { uint32 reserved; } Ifx_I2C;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_IOM;
typedef struct { uint32 reserved; } Ifx_LMU;
typedef struct { uint32 reserved; } Ifx_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC;
typedef struct { uint32 reserved; } Ifx_MTU;
typedef struct { uint32 reserved; } Ifx_PFI;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_STM;

/* Extern MODULE_* instances */
extern Ifx_ASCLIN MODULE_ASCLIN0;
extern Ifx_ASCLIN MODULE_ASCLIN1;
extern Ifx_ASCLIN MODULE_ASCLIN2;
extern Ifx_ASCLIN MODULE_ASCLIN3;
extern Ifx_ASCLIN MODULE_ASCLIN4;
extern Ifx_ASCLIN MODULE_ASCLIN5;
extern Ifx_ASCLIN MODULE_ASCLIN6;
extern Ifx_ASCLIN MODULE_ASCLIN7;
extern Ifx_ASCLIN MODULE_ASCLIN8;
extern Ifx_ASCLIN MODULE_ASCLIN9;
extern Ifx_ASCLIN MODULE_ASCLIN10;
extern Ifx_ASCLIN MODULE_ASCLIN11;
extern Ifx_ASCLIN MODULE_ASCLIN12;
extern Ifx_ASCLIN MODULE_ASCLIN13;
extern Ifx_ASCLIN MODULE_ASCLIN14;
extern Ifx_ASCLIN MODULE_ASCLIN15;
extern Ifx_ASCLIN MODULE_ASCLIN16;
extern Ifx_ASCLIN MODULE_ASCLIN17;
extern Ifx_ASCLIN MODULE_ASCLIN18;
extern Ifx_ASCLIN MODULE_ASCLIN19;
extern Ifx_ASCLIN MODULE_ASCLIN20;
extern Ifx_ASCLIN MODULE_ASCLIN21;
extern Ifx_ASCLIN MODULE_ASCLIN22;
extern Ifx_ASCLIN MODULE_ASCLIN23;
extern Ifx_CAN MODULE_CAN0;
extern Ifx_CAN MODULE_CAN1;
extern Ifx_CAN MODULE_CAN2;
extern Ifx_CBS MODULE_CBS;
extern Ifx_CCU6 MODULE_CCU60;
extern Ifx_CCU6 MODULE_CCU61;
extern Ifx_CONVCTRL MODULE_CONVCTRL;
extern Ifx_CPU MODULE_CPU0;
extern Ifx_CPU MODULE_CPU1;
extern Ifx_CPU MODULE_CPU2;
extern Ifx_CPU MODULE_CPU3;
extern Ifx_DAM MODULE_DAM0;
extern Ifx_DMA MODULE_DMA;
extern Ifx_DMU MODULE_DMU;
extern Ifx_DOM MODULE_DOM0;
extern Ifx_EDSADC MODULE_EDSADC;
extern Ifx_ERAY MODULE_ERAY0;
extern Ifx_ERAY MODULE_ERAY1;
extern Ifx_EVADC MODULE_EVADC;
extern Ifx_FCE MODULE_FCE;
extern Ifx_FSI MODULE_FSI;
extern Ifx_GETH MODULE_GETH;
extern Ifx_GPT120 MODULE_GPT120;
extern Ifx_GTM MODULE_GTM;
extern Ifx_HSCT MODULE_HSCT0;
extern Ifx_HSSL MODULE_HSSL0;
extern Ifx_I2C MODULE_I2C0;
extern Ifx_I2C MODULE_I2C1;
extern Ifx_INT MODULE_INT;
extern Ifx_IOM MODULE_IOM;
extern Ifx_LMU MODULE_LMU0;
extern Ifx_MINIMCDS MODULE_MINIMCDS;
extern Ifx_MSC MODULE_MSC0;
extern Ifx_MSC MODULE_MSC1;
extern Ifx_MSC MODULE_MSC2;
extern Ifx_MTU MODULE_MTU;
extern Ifx_PFI MODULE_PFI0;
extern Ifx_PFI MODULE_PFI1;
extern Ifx_PFI MODULE_PFI2;
extern Ifx_PFI MODULE_PFI3;
extern Ifx_PMS MODULE_PMS;
extern Ifx_PMU MODULE_PMU;
extern Ifx_P MODULE_P00;
extern Ifx_P MODULE_P01;
extern Ifx_P MODULE_P02;
extern Ifx_P MODULE_P10;
extern Ifx_P MODULE_P11;
extern Ifx_P MODULE_P12;
extern Ifx_P MODULE_P13;
extern Ifx_P MODULE_P14;
extern Ifx_P MODULE_P15;
extern Ifx_P MODULE_P20;
extern Ifx_P MODULE_P21;
extern Ifx_P MODULE_P22;
extern Ifx_P MODULE_P23;
extern Ifx_P MODULE_P24;
extern Ifx_P MODULE_P25;
extern Ifx_P MODULE_P26;
extern Ifx_P MODULE_P30;
extern Ifx_P MODULE_P31;
extern Ifx_P MODULE_P32;
extern Ifx_P MODULE_P33;
extern Ifx_P MODULE_P34;
extern Ifx_P MODULE_P40;
extern Ifx_P MODULE_P41;
extern Ifx_PSI5S MODULE_PSI5S;
extern Ifx_PSI5 MODULE_PSI5;
extern Ifx_QSPI MODULE_QSPI0;
extern Ifx_QSPI MODULE_QSPI1;
extern Ifx_QSPI MODULE_QSPI2;
extern Ifx_QSPI MODULE_QSPI3;
extern Ifx_QSPI MODULE_QSPI4;
extern Ifx_SBCU MODULE_SBCU;
extern Ifx_SCU MODULE_SCU;
extern Ifx_SENT MODULE_SENT;
extern Ifx_SMU MODULE_SMU;
extern Ifx_SRC MODULE_SRC;
extern Ifx_STM MODULE_STM0;
extern Ifx_STM MODULE_STM1;
extern Ifx_STM MODULE_STM2;
extern Ifx_STM MODULE_STM3;

/* Spy counters and return-value controls for mocked functions */
/* IfxGtm */
extern int mock_IfxGtm_isEnabled_callCount;
extern boolean mock_IfxGtm_isEnabled_returnValue;
extern int mock_IfxGtm_enable_callCount;
extern int mock_IfxGtm_isModuleSuspended_callCount;
extern boolean mock_IfxGtm_isModuleSuspended_returnValue;
extern int mock_IfxGtm_setSuspendMode_callCount;
extern int mock_IfxGtm_disable_callCount;
extern int mock_IfxGtm_getSysClkFrequency_callCount;
extern float32 mock_IfxGtm_getSysClkFrequency_returnValue;
extern int mock_IfxGtm_getClusterFrequency_callCount;
extern float32 mock_IfxGtm_getClusterFrequency_returnValue;

/* IfxGtm_Cmu */
extern int mock_IfxGtm_Cmu_enable_callCount;
extern int mock_IfxGtm_Cmu_isEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isEnabled_returnValue;
extern int mock_IfxGtm_Cmu_getModuleFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
extern int mock_IfxGtm_Cmu_setGclkFrequency_callCount;
extern int mock_IfxGtm_Cmu_setClkFrequency_callCount;
extern int mock_IfxGtm_Cmu_enableClocks_callCount;
extern int mock_IfxGtm_Cmu_getClkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getClkFrequency_returnValue;
extern int mock_IfxGtm_Cmu_getEclkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getEclkFrequency_returnValue;
extern int mock_IfxGtm_Cmu_getFxClkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getFxClkFrequency_returnValue;
extern int mock_IfxGtm_Cmu_getGclkFrequency_callCount;
extern float32 mock_IfxGtm_Cmu_getGclkFrequency_returnValue;
extern int mock_IfxGtm_Cmu_isClkClockEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isClkClockEnabled_returnValue;
extern int mock_IfxGtm_Cmu_isEclkClockEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isEclkClockEnabled_returnValue;
extern int mock_IfxGtm_Cmu_isFxClockEnabled_callCount;
extern boolean mock_IfxGtm_Cmu_isFxClockEnabled_returnValue;
extern int mock_IfxGtm_Cmu_selectClkInput_callCount;
extern int mock_IfxGtm_Cmu_setEclkFrequency_callCount;

/* IfxGtm_Pwm */
extern int mock_IfxGtm_Pwm_init_callCount;
extern int mock_IfxGtm_Pwm_initConfig_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateFrequency_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsDuty_callCount;
extern int mock_IfxGtm_Pwm_setChannelPolarity_callCount;
extern int mock_IfxGtm_Pwm_updateChannelPhase_callCount;
extern int mock_IfxGtm_Pwm_updateChannelPhaseImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelDuty_callCount;
extern int mock_IfxGtm_Pwm_updateChannelDutyImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_callCount;
extern int mock_IfxGtm_Pwm_initChannelConfig_callCount;
extern int mock_IfxGtm_Pwm_startSyncedChannels_callCount;
extern int mock_IfxGtm_Pwm_stopSyncedChannels_callCount;
extern int mock_IfxGtm_Pwm_startSyncedGroups_callCount;
extern int mock_IfxGtm_Pwm_stopSyncedGroups_callCount;
extern int mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_callCount;
extern int mock_IfxGtm_Pwm_updateFrequencyImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelPulse_callCount;
extern int mock_IfxGtm_Pwm_updateChannelPulseImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsPhase_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsPulse_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_callCount;
extern int mock_IfxGtm_Pwm_updateChannelsPulseImmediate_callCount;
extern int mock_IfxGtm_Pwm_interruptHandler_callCount;
extern int mock_IfxGtm_Pwm_getChannelState_callCount;
extern int mock_IfxGtm_Pwm_stopChannelOutputs_callCount;
extern int mock_IfxGtm_Pwm_startChannelOutputs_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;

extern int mock_togglePin_callCount;

/* Return-value controls for non-void PWM APIs */
extern uint32 mock_IfxGtm_Pwm_getChannelState_returnValue; /* stores IfxGtm_Pwm_ChannelState as uint32 */

/* Spy capture fields */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif
extern uint32  mock_IfxGtm_Pwm_init_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_init_lastFrequency;
extern uint32  mock_IfxGtm_Pwm_initConfig_lastNumChannels;
extern float32 mock_IfxGtm_Pwm_initConfig_lastFrequency;
extern float32 mock_IfxGtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control API */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void);

/* Getters for call counts */
int mock_IfxGtm_isEnabled_getCallCount(void);
int mock_IfxGtm_enable_getCallCount(void);
int mock_IfxGtm_isModuleSuspended_getCallCount(void);
int mock_IfxGtm_setSuspendMode_getCallCount(void);
int mock_IfxGtm_disable_getCallCount(void);
int mock_IfxGtm_getSysClkFrequency_getCallCount(void);
int mock_IfxGtm_getClusterFrequency_getCallCount(void);

int mock_IfxGtm_Cmu_enable_getCallCount(void);
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void);
int mock_IfxGtm_Cmu_getClkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_getEclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_getFxClkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_getGclkFrequency_getCallCount(void);
int mock_IfxGtm_Cmu_isClkClockEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_isEclkClockEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_isFxClockEnabled_getCallCount(void);
int mock_IfxGtm_Cmu_selectClkInput_getCallCount(void);
int mock_IfxGtm_Cmu_setEclkFrequency_getCallCount(void);

int mock_IfxGtm_Pwm_init_getCallCount(void);
int mock_IfxGtm_Pwm_initConfig_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDutyImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateFrequency_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(void);
int mock_IfxGtm_Pwm_setChannelPolarity_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelPhase_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelPhaseImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelDuty_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelDutyImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_initChannelConfig_getCallCount(void);
int mock_IfxGtm_Pwm_startSyncedChannels_getCallCount(void);
int mock_IfxGtm_Pwm_stopSyncedChannels_getCallCount(void);
int mock_IfxGtm_Pwm_startSyncedGroups_getCallCount(void);
int mock_IfxGtm_Pwm_stopSyncedGroups_getCallCount(void);
int mock_IfxGtm_Pwm_updateSyncedGroupsFrequency_getCallCount(void);
int mock_IfxGtm_Pwm_updateFrequencyImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelPulse_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelPulseImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsPhase_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsPulse_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsPhaseImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_updateChannelsPulseImmediate_getCallCount(void);
int mock_IfxGtm_Pwm_interruptHandler_getCallCount(void);
int mock_IfxGtm_Pwm_getChannelState_getCallCount(void);
int mock_IfxGtm_Pwm_stopChannelOutputs_getCallCount(void);
int mock_IfxGtm_Pwm_startChannelOutputs_getCallCount(void);
int mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);

#endif /* MOCK_GTM_TOM_3_PHASE_INVERTER_PWM_H */
