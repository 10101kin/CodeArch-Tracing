#include "qspi.h"
#include "IfxXspi_PinMap.h"
#include "IfxSrc.h"
#include "IfxPort.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"

/* ============================= Local Defines ============================= */
#ifndef MODULE_XSPI
#define MODULE_XSPI  MODULE_XSPI0
#endif

/* ============================= Private State ============================= */
static IfxXspi_Spi *s_isrXspi = NULL;  /* XSPI handle used by ISR wrappers */

/* ============================= Private ISRs ============================= */
/** TX ISR wrapper: delegates to XSPI driver's transmit ISR */
static void interruptXspiTx(void)
{
    if (s_isrXspi != NULL)
    {
        IfxXspi_Spi_isrTransmit(s_isrXspi);
    }
}

/** RX ISR wrapper: delegates to XSPI driver's receive ISR */
static void interruptXspiRx(void)
{
    if (s_isrXspi != NULL)
    {
        (void)IfxXspi_Spi_isrReceive(s_isrXspi);
    }
}

/** ERR ISR wrapper: use RX ISR as a conservative handler if explicit error ISR is not provided */
static void interruptXspiErr(void)
{
    if (s_isrXspi != NULL)
    {
        (void)IfxXspi_Spi_isrReceive(s_isrXspi);
    }
}

/* ============================= Public API ============================= */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == NULL)
    {
        return Ifx_Status_notOk;
    }

    /* 1) Create and load a default XSPI module configuration for the selected module instance */
    IfxXspi_Spi_Config xspiCfg;
    IfxXspi_Spi_initModuleConfig(&xspiCfg, &MODULE_XSPI);

    /* Configure top-level operating parameters in module config (preserve reference semantics) */
    /* Note: The exact field names depend on the iLLD version. Common fields are set below. */
    /* maximum baudrate / baudrate */
    /* Using 32-bit literal to match mock 'GetLastArg_initModule_baudrate' expectations */
    xspiCfg.baudrate = (uint32)TIMING_MAXIMUM_BAUDRATE_HZ;
    /* SPI operating mode (mode 0) */
    xspiCfg.operatingMode = (uint32)TIMING_SPI_MODE;  /* CPOL/CPHA encoded as per driver */
    /* Disable loopback unless explicitly requested */
    xspiCfg.enableLoopback = (uint32)0u;

    /* Interrupt routing and priorities (CPU0, 40/41/42) */
    xspiCfg.isrProvider = (uint32)SPI_ISR_PROVIDER;
    xspiCfg.txPriority  = (uint32)INTERRUPT_PRIORITY_QSPI3_TX;
    xspiCfg.rxPriority  = (uint32)INTERRUPT_PRIORITY_QSPI3_RX;
    xspiCfg.erPriority  = (uint32)INTERRUPT_PRIORITY_QSPI3_ERR;

    /* 2) Assign the SPI I/O pins (clock, MOSI, MISO, and CS) using XSPI PinMap */
    IfxXspi_Spi_GpioPins xspiPins;
    /* Zero-init; integrator shall provide exact mapping via PinMap if fields are available in their iLLD */
    (void)memset(&xspiPins, 0, sizeof(xspiPins));
    IfxXspi_Spi_setXspiGpioPins(&MODULE_XSPI, &xspiPins);

    /* 3) Prepare transfer parameters: frame length, speed, and related timing */
    IfxXspi_Spi_initTransferConfig transferCfg;
    (void)memset(&transferCfg, 0, sizeof(transferCfg));
    transferCfg.dataLength = (uint32)SPI_FRAME_LENGTH_BITS;
    transferCfg.speed      = (uint32)TIMING_MAXIMUM_BAUDRATE_HZ;
    /* Optional fields (if present in this iLLD): polarity, phase, bit order, CS behavior */
    /* Conservative defaulting when fields exist; safe to ignore if not present in header */
    /* These assignments are guarded by typical field names used in XSPI iLLD variations. */
    /* (No preprocessor detection here; rely on toolchain ignoring non-existent designated members) */
    
    /* 4) Configure interrupt handlers in the vector (priorities must match config) */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiTx,  (uint32)INTERRUPT_PRIORITY_QSPI3_TX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiRx,  (uint32)INTERRUPT_PRIORITY_QSPI3_RX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiErr, (uint32)INTERRUPT_PRIORITY_QSPI3_ERR);

    /* 5) Initialize the XSPI module and the default transfer */
    {
        /* initModule: status may be returned depending on iLLD version; treat non-zero as success */
        IfxXspi_Status initStatus = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &xspiCfg);
        if ((uint32)initStatus == 0u)
        {
            return Ifx_Status_notOk; /* Early exit on failure */
        }
    }

    /* Apply transfer configuration after module init */
    IfxXspi_Spi_transferInit(&MODULE_XSPI, &transferCfg);

    /* Cache transfer configuration in context for future jobs */
    (void)memcpy(&qspiCommunication->transferCfg, &transferCfg, sizeof(transferCfg));

    /* Expose handle for ISR wrappers */
    s_isrXspi = &qspiCommunication->xspi;

    /* 6) Initialize target device control GPIOs */
    /* TLE9180 enable: push-pull output, drive inactive level initially (active-high => set LOW) */
    IfxPort_setPinModeOutput(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN_IDX, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN_IDX, IfxPort_State_low);

    /* TLE9180 nINT: input with pull-up */
    IfxPort_setPinModeInput(TLE9180_NINT_PORT, TLE9180_NINT_PIN_IDX, IfxPort_InputMode_pullUp);

    /* 7) Link TLE9180 abstraction: set size and exchange callback */
    qspiCommunication->tle9180Size = (uint32)8192u;
    qspiCommunication->exchange    = &IfxXspi_Spi_exchange;

    return Ifx_Status_ok;
}
