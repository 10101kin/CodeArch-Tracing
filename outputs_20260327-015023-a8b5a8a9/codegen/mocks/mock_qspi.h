#ifndef MOCK_QSPI_H
#define MOCK_QSPI_H

/* Base type aliases */
typedef float               float32;
typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef signed short        sint16;
typedef unsigned char       boolean;
typedef uint32              Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE     ((boolean)1)
#endif
#ifndef FALSE
#define FALSE    ((boolean)0)
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
typedef enum {
    Ifx_ActiveState_low = 0,
    Ifx_ActiveState_high = 1
} Ifx_ActiveState;

typedef enum {
    IfxSrc_Tos_cpu0 = 0,
    IfxSrc_Tos_cpu1 = 1,
    IfxSrc_Tos_cpu2 = 2,
    IfxSrc_Tos_cpu3 = 3,
    IfxSrc_Tos_dma  = 8,
    IfxSrc_Tos_none = 255
} IfxSrc_Tos;

/* MODULE_* register-block stubs (typedef + extern) */
typedef struct { uint32 reserved; } Ifx_XSPI;   extern Ifx_XSPI MODULE_XSPI;
typedef struct { uint32 reserved; } Ifx_CPU;    extern Ifx_CPU MODULE_CPU;
typedef struct { uint32 reserved; } Ifx_P;      extern Ifx_P MODULE_P00; extern Ifx_P MODULE_P01; extern Ifx_P MODULE_P02; extern Ifx_P MODULE_P10; extern Ifx_P MODULE_P11; extern Ifx_P MODULE_P12; extern Ifx_P MODULE_P13; extern Ifx_P MODULE_P14; extern Ifx_P MODULE_P15; extern Ifx_P MODULE_P20; extern Ifx_P MODULE_P21; extern Ifx_P MODULE_P22; extern Ifx_P MODULE_P23; extern Ifx_P MODULE_P24; extern Ifx_P MODULE_P25; extern Ifx_P MODULE_P26; extern Ifx_P MODULE_P30; extern Ifx_P MODULE_P31; extern Ifx_P MODULE_P32; extern Ifx_P MODULE_P33; extern Ifx_P MODULE_P34; extern Ifx_P MODULE_P40; extern Ifx_P MODULE_P41;
typedef struct { uint32 reserved; } Ifx_ASCLIN0; extern Ifx_ASCLIN0 MODULE_ASCLIN0; extern Ifx_ASCLIN0 MODULE_ASCLIN1; extern Ifx_ASCLIN0 MODULE_ASCLIN2; extern Ifx_ASCLIN0 MODULE_ASCLIN3; extern Ifx_ASCLIN0 MODULE_ASCLIN4; extern Ifx_ASCLIN0 MODULE_ASCLIN5; extern Ifx_ASCLIN0 MODULE_ASCLIN6; extern Ifx_ASCLIN0 MODULE_ASCLIN7; extern Ifx_ASCLIN0 MODULE_ASCLIN8; extern Ifx_ASCLIN0 MODULE_ASCLIN9; extern Ifx_ASCLIN0 MODULE_ASCLIN10; extern Ifx_ASCLIN0 MODULE_ASCLIN11; extern Ifx_ASCLIN0 MODULE_ASCLIN12; extern Ifx_ASCLIN0 MODULE_ASCLIN13; extern Ifx_ASCLIN0 MODULE_ASCLIN14; extern Ifx_ASCLIN0 MODULE_ASCLIN15; extern Ifx_ASCLIN0 MODULE_ASCLIN16; extern Ifx_ASCLIN0 MODULE_ASCLIN17; extern Ifx_ASCLIN0 MODULE_ASCLIN18; extern Ifx_ASCLIN0 MODULE_ASCLIN19; extern Ifx_ASCLIN0 MODULE_ASCLIN20; extern Ifx_ASCLIN0 MODULE_ASCLIN21; extern Ifx_ASCLIN0 MODULE_ASCLIN22; extern Ifx_ASCLIN0 MODULE_ASCLIN23;
typedef struct { uint32 reserved; } Ifx_CAN0;   extern Ifx_CAN0 MODULE_CAN0; extern Ifx_CAN0 MODULE_CAN1; extern Ifx_CAN0 MODULE_CAN2;
typedef struct { uint32 reserved; } Ifx_CBS;    extern Ifx_CBS MODULE_CBS;
typedef struct { uint32 reserved; } Ifx_CCU60;  extern Ifx_CCU60 MODULE_CCU60; extern Ifx_CCU60 MODULE_CCU61;
typedef struct { uint32 reserved; } Ifx_CONVCTRL; extern Ifx_CONVCTRL MODULE_CONVCTRL;
typedef struct { uint32 reserved; } Ifx_CPU0;   extern Ifx_CPU0 MODULE_CPU0; extern Ifx_CPU0 MODULE_CPU1; extern Ifx_CPU0 MODULE_CPU2; extern Ifx_CPU0 MODULE_CPU3;
typedef struct { uint32 reserved; } Ifx_DAM0;   extern Ifx_DAM0 MODULE_DAM0;
typedef struct { uint32 reserved; } Ifx_DMA;    extern Ifx_DMA MODULE_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;    extern Ifx_DMU MODULE_DMU;
typedef struct { uint32 reserved; } Ifx_DOM0;   extern Ifx_DOM0 MODULE_DOM0;
typedef struct { uint32 reserved; } Ifx_EDSADC; extern Ifx_EDSADC MODULE_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY0;  extern Ifx_ERAY0 MODULE_ERAY0; extern Ifx_ERAY0 MODULE_ERAY1;
typedef struct { uint32 reserved; } Ifx_EVADC;  extern Ifx_EVADC MODULE_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;    extern Ifx_FCE MODULE_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;    extern Ifx_FSI MODULE_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;   extern Ifx_GETH MODULE_GETH;
typedef struct { uint32 reserved; } Ifx_GPT120; extern Ifx_GPT120 MODULE_GPT120;
typedef struct { uint32 reserved; } Ifx_GTM;    extern Ifx_GTM MODULE_GTM;
typedef struct { uint32 reserved; } Ifx_HSCT0;  extern Ifx_HSCT0 MODULE_HSCT0;
typedef struct { uint32 reserved; } Ifx_HSSL0;  extern Ifx_HSSL0 MODULE_HSSL0;
typedef struct { uint32 reserved; } Ifx_I2C0;   extern Ifx_I2C0 MODULE_I2C0; extern Ifx_I2C0 MODULE_I2C1;
typedef struct { uint32 reserved; } Ifx_INT;    extern Ifx_INT MODULE_INT;
typedef struct { uint32 reserved; } Ifx_IOM;    extern Ifx_IOM MODULE_IOM;
typedef struct { uint32 reserved; } Ifx_LMU0;   extern Ifx_LMU0 MODULE_LMU0;
typedef struct { uint32 reserved; } Ifx_MINIMCDS; extern Ifx_MINIMCDS MODULE_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC0;   extern Ifx_MSC0 MODULE_MSC0; extern Ifx_MSC0 MODULE_MSC1; extern Ifx_MSC0 MODULE_MSC2;
typedef struct { uint32 reserved; } Ifx_MTU;    extern Ifx_MTU MODULE_MTU;
typedef struct { uint32 reserved; } Ifx_PFI0;   extern Ifx_PFI0 MODULE_PFI0; extern Ifx_PFI0 MODULE_PFI1; extern Ifx_PFI0 MODULE_PFI2; extern Ifx_PFI0 MODULE_PFI3;
typedef struct { uint32 reserved; } Ifx_PMS;    extern Ifx_PMS MODULE_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;    extern Ifx_PMU MODULE_PMU;
typedef struct { uint32 reserved; } Ifx_PSI5S;  extern Ifx_PSI5S MODULE_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;   extern Ifx_PSI5 MODULE_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI0;  extern Ifx_QSPI0 MODULE_QSPI0; extern Ifx_QSPI0 MODULE_QSPI1; extern Ifx_QSPI0 MODULE_QSPI2; extern Ifx_QSPI0 MODULE_QSPI3; extern Ifx_QSPI0 MODULE_QSPI4;
typedef struct { uint32 reserved; } Ifx_SBCU;   extern Ifx_SBCU MODULE_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;    extern Ifx_SCU MODULE_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;   extern Ifx_SENT MODULE_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;    extern Ifx_SMU MODULE_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;    extern Ifx_SRC MODULE_SRC;
typedef struct { uint32 reserved; } Ifx_STM0;   extern Ifx_STM0 MODULE_STM0; extern Ifx_STM0 MODULE_STM1; extern Ifx_STM0 MODULE_STM2; extern Ifx_STM0 MODULE_STM3;

/* Spy counters and return-value controls for mocked functions */
extern int mock_IfxXspi_Spi_transferInit_callCount;
extern int mock_IfxXspi_Spi_exchange_callCount;
extern boolean mock_IfxXspi_Spi_exchange_returnValue;
extern int mock_IfxXspi_Spi_isrDmaReceive_callCount;
extern int mock_IfxXspi_Spi_setXspiGpioPins_callCount;
extern int mock_IfxXspi_Spi_initModule_callCount;
extern int mock_IfxXspi_Spi_initModuleConfig_callCount;
extern int mock_IfxXspi_Spi_isrTransmit_callCount;
extern int mock_IfxXspi_Spi_isrReceive_callCount;
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxPort_setPinState_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxPort_setPinModeInput_callCount;

/* Value-capture spy fields (generic PWM-oriented spies as required) */
#define MOCK_MAX_CHANNELS 16
extern uint32  mock_initEgtmAtom_lastNumChannels;
extern float32 mock_initEgtmAtom_lastFrequency;
extern float32 mock_updateEgtmAtom_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_dtEgtmAtom_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_dtEgtmAtom_lastDtFalling[MOCK_MAX_CHANNELS];
extern uint32  mock_togglePin_callCount;

/* Mock control API */
void mock_qspi_reset(void);
int  mock_IfxXspi_Spi_transferInit_getCallCount(void);
int  mock_IfxXspi_Spi_exchange_getCallCount(void);
int  mock_IfxXspi_Spi_isrDmaReceive_getCallCount(void);
int  mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(void);
int  mock_IfxXspi_Spi_initModule_getCallCount(void);
int  mock_IfxXspi_Spi_initModuleConfig_getCallCount(void);
int  mock_IfxXspi_Spi_isrTransmit_getCallCount(void);
int  mock_IfxXspi_Spi_isrReceive_getCallCount(void);
int  mock_IfxCpu_Irq_installInterruptHandler_getCallCount(void);
int  mock_IfxPort_setPinState_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxPort_setPinModeInput_getCallCount(void);

#endif /* MOCK_QSPI_H */
