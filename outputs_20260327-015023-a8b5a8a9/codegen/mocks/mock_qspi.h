/* mock_qspi.h - Base types + MODULE stubs + spy API only */
#ifndef MOCK_QSPI_H
#define MOCK_QSPI_H

/* Base type aliases */
typedef float    float32;
typedef unsigned int   uint32;
typedef signed int     sint32;
typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef signed short   sint16;
typedef unsigned char  boolean;
typedef uint32         Ifx_Priority;

/* Macros */
#ifndef TRUE
#define TRUE      (1u)
#endif
#ifndef FALSE
#define FALSE     (0u)
#endif
#ifndef NULL_PTR
#define NULL_PTR  ((void*)0)
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
    IfxSrc_Tos_dma  = 4,
    IfxSrc_Tos_cpu4 = 5,
    IfxSrc_Tos_cpu5 = 6
} IfxSrc_Tos;

/* MODULE_* register-block stubs (typedef + extern) */
/* Unique SFR type stubs */
typedef struct { uint32 reserved; } Ifx_XSPI;
typedef struct { uint32 reserved; } Ifx_CPU;
typedef struct { uint32 reserved; } Ifx_P;
typedef struct { uint32 reserved; } Ifx_ASCLIN0;
typedef struct { uint32 reserved; } Ifx_ASCLIN1;
typedef struct { uint32 reserved; } Ifx_ASCLIN2;
typedef struct { uint32 reserved; } Ifx_ASCLIN3;
typedef struct { uint32 reserved; } Ifx_ASCLIN4;
typedef struct { uint32 reserved; } Ifx_ASCLIN5;
typedef struct { uint32 reserved; } Ifx_ASCLIN6;
typedef struct { uint32 reserved; } Ifx_ASCLIN7;
typedef struct { uint32 reserved; } Ifx_ASCLIN8;
typedef struct { uint32 reserved; } Ifx_ASCLIN9;
typedef struct { uint32 reserved; } Ifx_ASCLIN10;
typedef struct { uint32 reserved; } Ifx_ASCLIN11;
typedef struct { uint32 reserved; } Ifx_ASCLIN12;
typedef struct { uint32 reserved; } Ifx_ASCLIN13;
typedef struct { uint32 reserved; } Ifx_ASCLIN14;
typedef struct { uint32 reserved; } Ifx_ASCLIN15;
typedef struct { uint32 reserved; } Ifx_ASCLIN16;
typedef struct { uint32 reserved; } Ifx_ASCLIN17;
typedef struct { uint32 reserved; } Ifx_ASCLIN18;
typedef struct { uint32 reserved; } Ifx_ASCLIN19;
typedef struct { uint32 reserved; } Ifx_ASCLIN20;
typedef struct { uint32 reserved; } Ifx_ASCLIN21;
typedef struct { uint32 reserved; } Ifx_ASCLIN22;
typedef struct { uint32 reserved; } Ifx_ASCLIN23;
typedef struct { uint32 reserved; } Ifx_CAN0;
typedef struct { uint32 reserved; } Ifx_CAN1;
typedef struct { uint32 reserved; } Ifx_CAN2;
typedef struct { uint32 reserved; } Ifx_CBS;
typedef struct { uint32 reserved; } Ifx_CCU60;
typedef struct { uint32 reserved; } Ifx_CCU61;
typedef struct { uint32 reserved; } Ifx_CONVCTRL;
typedef struct { uint32 reserved; } Ifx_CPU0;
typedef struct { uint32 reserved; } Ifx_CPU1;
typedef struct { uint32 reserved; } Ifx_CPU2;
typedef struct { uint32 reserved; } Ifx_CPU3;
typedef struct { uint32 reserved; } Ifx_DAM0;
typedef struct { uint32 reserved; } Ifx_DMA;
typedef struct { uint32 reserved; } Ifx_DMU;
typedef struct { uint32 reserved; } Ifx_DOM0;
typedef struct { uint32 reserved; } Ifx_EDSADC;
typedef struct { uint32 reserved; } Ifx_ERAY0;
typedef struct { uint32 reserved; } Ifx_ERAY1;
typedef struct { uint32 reserved; } Ifx_EVADC;
typedef struct { uint32 reserved; } Ifx_FCE;
typedef struct { uint32 reserved; } Ifx_FSI;
typedef struct { uint32 reserved; } Ifx_GETH;
typedef struct { uint32 reserved; } Ifx_GPT120;
typedef struct { uint32 reserved; } Ifx_GTM;
typedef struct { uint32 reserved; } Ifx_HSCT0;
typedef struct { uint32 reserved; } Ifx_HSSL0;
typedef struct { uint32 reserved; } Ifx_I2C0;
typedef struct { uint32 reserved; } Ifx_I2C1;
typedef struct { uint32 reserved; } Ifx_INT;
typedef struct { uint32 reserved; } Ifx_IOM;
typedef struct { uint32 reserved; } Ifx_LMU0;
typedef struct { uint32 reserved; } Ifx_MINIMCDS;
typedef struct { uint32 reserved; } Ifx_MSC0;
typedef struct { uint32 reserved; } Ifx_MSC1;
typedef struct { uint32 reserved; } Ifx_MSC2;
typedef struct { uint32 reserved; } Ifx_MTU;
typedef struct { uint32 reserved; } Ifx_PFI0;
typedef struct { uint32 reserved; } Ifx_PFI1;
typedef struct { uint32 reserved; } Ifx_PFI2;
typedef struct { uint32 reserved; } Ifx_PFI3;
typedef struct { uint32 reserved; } Ifx_PMS;
typedef struct { uint32 reserved; } Ifx_PMU;
typedef struct { uint32 reserved; } Ifx_P01;
typedef struct { uint32 reserved; } Ifx_P02;
typedef struct { uint32 reserved; } Ifx_P10;
typedef struct { uint32 reserved; } Ifx_P11;
typedef struct { uint32 reserved; } Ifx_P12;
typedef struct { uint32 reserved; } Ifx_P13;
typedef struct { uint32 reserved; } Ifx_P14;
typedef struct { uint32 reserved; } Ifx_P15;
typedef struct { uint32 reserved; } Ifx_P20;
typedef struct { uint32 reserved; } Ifx_P21;
typedef struct { uint32 reserved; } Ifx_P22;
typedef struct { uint32 reserved; } Ifx_P23;
typedef struct { uint32 reserved; } Ifx_P24;
typedef struct { uint32 reserved; } Ifx_P25;
typedef struct { uint32 reserved; } Ifx_P26;
typedef struct { uint32 reserved; } Ifx_P30;
typedef struct { uint32 reserved; } Ifx_P31;
typedef struct { uint32 reserved; } Ifx_P32;
typedef struct { uint32 reserved; } Ifx_P33;
typedef struct { uint32 reserved; } Ifx_P34;
typedef struct { uint32 reserved; } Ifx_P40;
typedef struct { uint32 reserved; } Ifx_P41;
typedef struct { uint32 reserved; } Ifx_PSI5S;
typedef struct { uint32 reserved; } Ifx_PSI5;
typedef struct { uint32 reserved; } Ifx_QSPI0;
typedef struct { uint32 reserved; } Ifx_QSPI1;
typedef struct { uint32 reserved; } Ifx_QSPI2;
typedef struct { uint32 reserved; } Ifx_QSPI3;
typedef struct { uint32 reserved; } Ifx_QSPI4;
typedef struct { uint32 reserved; } Ifx_SBCU;
typedef struct { uint32 reserved; } Ifx_SCU;
typedef struct { uint32 reserved; } Ifx_SENT;
typedef struct { uint32 reserved; } Ifx_SMU;
typedef struct { uint32 reserved; } Ifx_SRC;
typedef struct { uint32 reserved; } Ifx_STM0;
typedef struct { uint32 reserved; } Ifx_STM1;
typedef struct { uint32 reserved; } Ifx_STM2;
typedef struct { uint32 reserved; } Ifx_STM3;

/* MODULE externs */
extern Ifx_XSPI   MODULE_XSPI;
extern Ifx_CPU    MODULE_CPU;
extern Ifx_P      MODULE_P00;
extern Ifx_ASCLIN0  MODULE_ASCLIN0;
extern Ifx_ASCLIN1  MODULE_ASCLIN1;
extern Ifx_ASCLIN2  MODULE_ASCLIN2;
extern Ifx_ASCLIN3  MODULE_ASCLIN3;
extern Ifx_ASCLIN4  MODULE_ASCLIN4;
extern Ifx_ASCLIN5  MODULE_ASCLIN5;
extern Ifx_ASCLIN6  MODULE_ASCLIN6;
extern Ifx_ASCLIN7  MODULE_ASCLIN7;
extern Ifx_ASCLIN8  MODULE_ASCLIN8;
extern Ifx_ASCLIN9  MODULE_ASCLIN9;
extern Ifx_ASCLIN10 MODULE_ASCLIN10;
extern Ifx_ASCLIN11 MODULE_ASCLIN11;
extern Ifx_ASCLIN12 MODULE_ASCLIN12;
extern Ifx_ASCLIN13 MODULE_ASCLIN13;
extern Ifx_ASCLIN14 MODULE_ASCLIN14;
extern Ifx_ASCLIN15 MODULE_ASCLIN15;
extern Ifx_ASCLIN16 MODULE_ASCLIN16;
extern Ifx_ASCLIN17 MODULE_ASCLIN17;
extern Ifx_ASCLIN18 MODULE_ASCLIN18;
extern Ifx_ASCLIN19 MODULE_ASCLIN19;
extern Ifx_ASCLIN20 MODULE_ASCLIN20;
extern Ifx_ASCLIN21 MODULE_ASCLIN21;
extern Ifx_ASCLIN22 MODULE_ASCLIN22;
extern Ifx_ASCLIN23 MODULE_ASCLIN23;
extern Ifx_CAN0   MODULE_CAN0;
extern Ifx_CAN1   MODULE_CAN1;
extern Ifx_CAN2   MODULE_CAN2;
extern Ifx_CBS    MODULE_CBS;
extern Ifx_CCU60  MODULE_CCU60;
extern Ifx_CCU61  MODULE_CCU61;
extern Ifx_CONVCTRL MODULE_CONVCTRL;
extern Ifx_CPU0   MODULE_CPU0;
extern Ifx_CPU1   MODULE_CPU1;
extern Ifx_CPU2   MODULE_CPU2;
extern Ifx_CPU3   MODULE_CPU3;
extern Ifx_DAM0   MODULE_DAM0;
extern Ifx_DMA    MODULE_DMA;
extern Ifx_DMU    MODULE_DMU;
extern Ifx_DOM0   MODULE_DOM0;
extern Ifx_EDSADC MODULE_EDSADC;
extern Ifx_ERAY0  MODULE_ERAY0;
extern Ifx_ERAY1  MODULE_ERAY1;
extern Ifx_EVADC  MODULE_EVADC;
extern Ifx_FCE    MODULE_FCE;
extern Ifx_FSI    MODULE_FSI;
extern Ifx_GETH   MODULE_GETH;
extern Ifx_GPT120 MODULE_GPT120;
extern Ifx_GTM    MODULE_GTM;
extern Ifx_HSCT0  MODULE_HSCT0;
extern Ifx_HSSL0  MODULE_HSSL0;
extern Ifx_I2C0   MODULE_I2C0;
extern Ifx_I2C1   MODULE_I2C1;
extern Ifx_INT    MODULE_INT;
extern Ifx_IOM    MODULE_IOM;
extern Ifx_LMU0   MODULE_LMU0;
extern Ifx_MINIMCDS MODULE_MINIMCDS;
extern Ifx_MSC0   MODULE_MSC0;
extern Ifx_MSC1   MODULE_MSC1;
extern Ifx_MSC2   MODULE_MSC2;
extern Ifx_MTU    MODULE_MTU;
extern Ifx_PFI0   MODULE_PFI0;
extern Ifx_PFI1   MODULE_PFI1;
extern Ifx_PFI2   MODULE_PFI2;
extern Ifx_PFI3   MODULE_PFI3;
extern Ifx_PMS    MODULE_PMS;
extern Ifx_PMU    MODULE_PMU;
extern Ifx_P01    MODULE_P01;
extern Ifx_P02    MODULE_P02;
extern Ifx_P10    MODULE_P10;
extern Ifx_P11    MODULE_P11;
extern Ifx_P12    MODULE_P12;
extern Ifx_P13    MODULE_P13;
extern Ifx_P14    MODULE_P14;
extern Ifx_P15    MODULE_P15;
extern Ifx_P20    MODULE_P20;
extern Ifx_P21    MODULE_P21;
extern Ifx_P22    MODULE_P22;
extern Ifx_P23    MODULE_P23;
extern Ifx_P24    MODULE_P24;
extern Ifx_P25    MODULE_P25;
extern Ifx_P26    MODULE_P26;
extern Ifx_P30    MODULE_P30;
extern Ifx_P31    MODULE_P31;
extern Ifx_P32    MODULE_P32;
extern Ifx_P33    MODULE_P33;
extern Ifx_P34    MODULE_P34;
extern Ifx_P40    MODULE_P40;
extern Ifx_P41    MODULE_P41;
extern Ifx_PSI5S  MODULE_PSI5S;
extern Ifx_PSI5   MODULE_PSI5;
extern Ifx_QSPI0  MODULE_QSPI0;
extern Ifx_QSPI1  MODULE_QSPI1;
extern Ifx_QSPI2  MODULE_QSPI2;
extern Ifx_QSPI3  MODULE_QSPI3;
extern Ifx_QSPI4  MODULE_QSPI4;
extern Ifx_SBCU   MODULE_SBCU;
extern Ifx_SCU    MODULE_SCU;
extern Ifx_SENT   MODULE_SENT;
extern Ifx_SMU    MODULE_SMU;
extern Ifx_SRC    MODULE_SRC;
extern Ifx_STM0   MODULE_STM0;
extern Ifx_STM1   MODULE_STM1;
extern Ifx_STM2   MODULE_STM2;
extern Ifx_STM3   MODULE_STM3;

/* Spy counters and return-value controls */
/* IfxXspi_Spi */
extern int mock_IfxXspi_Spi_transferInit_callCount;
extern int mock_IfxXspi_Spi_exchange_callCount;
extern int mock_IfxXspi_Spi_isrDmaReceive_callCount;
extern int mock_IfxXspi_Spi_setXspiGpioPins_callCount;
extern int mock_IfxXspi_Spi_initModule_callCount;
extern int mock_IfxXspi_Spi_initModuleConfig_callCount;
extern int mock_IfxXspi_Spi_isrTransmit_callCount;
extern int mock_IfxXspi_Spi_isrReceive_callCount;

extern boolean      mock_IfxXspi_Spi_exchange_returnValue;
extern uint32       mock_IfxXspi_Spi_isrDmaReceive_returnValue;
extern uint32       mock_IfxXspi_Spi_isrReceive_returnValue;
/* IfxXspi_Status is defined in IfxXspi_Spi.h; declare as int here to avoid circular dep? Single-owner rule forbids. We'll declare after including per-driver? No — instead use a weakly-typed placeholder via integer? Not allowed. So we only declare pointer to a generic return; to keep single-owner, we won't declare the typed variable here. We'll provide a generic storage via uint32 and cast in C file. However the rules state: place spy externs in base. We'll expose a generic uint32 holder and handle cast in C file. */
extern uint32       mock_IfxXspi_Spi_initModule_returnValue_u32;

/* IfxCpu_Irq */
extern int mock_IfxCpu_Irq_installInterruptHandler_callCount;
extern int mock_IfxCpu_Irq_getTos_callCount;
extern IfxSrc_Tos mock_IfxCpu_Irq_getTos_returnValue;

/* IfxPort */
extern int mock_IfxPort_getPinState_callCount;
extern int mock_IfxPort_setPinFunctionMode_callCount;
extern int mock_IfxPort_setPinHigh_callCount;
extern int mock_IfxPort_setPinLow_callCount;
extern int mock_IfxPort_setPinModeInput_callCount;
extern int mock_IfxPort_setPinModeOutput_callCount;
extern int mock_IfxPort_setPinState_callCount;
extern int mock_IfxPort_togglePin_callCount;
extern int mock_IfxPort_disableEmergencyStop_callCount;
extern int mock_IfxPort_enableEmergencyStop_callCount;
extern int mock_IfxPort_setPinMode_callCount;
extern uint32 mock_togglePin_callCount;
extern uint32 mock_IfxPort_getPinState_returnValue_u32; /* stores IfxPort_State */

/* Generic value-capture spy fields */
#ifndef MOCK_MAX_CHANNELS
#define MOCK_MAX_CHANNELS 16
#endif
extern uint32  mock_IfxXspi_Spi_initModule_lastNumChannels;
extern float32 mock_IfxXspi_Spi_initModule_lastFrequency;
extern float32 mock_IfxXspi_Spi_exchange_lastDuties[MOCK_MAX_CHANNELS];
extern float32 mock_IfxXspi_Spi_transferInit_lastDtRising[MOCK_MAX_CHANNELS];
extern float32 mock_IfxXspi_Spi_transferInit_lastDtFalling[MOCK_MAX_CHANNELS];

/* Mock control */
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
int  mock_IfxCpu_Irq_getTos_getCallCount(void);

int  mock_IfxPort_getPinState_getCallCount(void);
int  mock_IfxPort_setPinFunctionMode_getCallCount(void);
int  mock_IfxPort_setPinHigh_getCallCount(void);
int  mock_IfxPort_setPinLow_getCallCount(void);
int  mock_IfxPort_setPinModeInput_getCallCount(void);
int  mock_IfxPort_setPinModeOutput_getCallCount(void);
int  mock_IfxPort_setPinState_getCallCount(void);
int  mock_IfxPort_togglePin_getCallCount(void);
int  mock_IfxPort_disableEmergencyStop_getCallCount(void);
int  mock_IfxPort_enableEmergencyStop_getCallCount(void);
int  mock_IfxPort_setPinMode_getCallCount(void);

#endif /* MOCK_QSPI_H */
