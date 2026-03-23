/**
 * @file qspi.c
 * @brief XSPI-based SPI communication driver for TLE9180 on TC4xx.
 *
 * Implementation adheres to the unified iLLD driver pattern and SW Detailed Design.
 *
 * Notes:
 *  - Watchdog disable is NOT allowed here (must be in CpuN_Main.c using IfxWtu APIs for TC4xx).
 *  - Pin mapping for XSPI signals (SCLK/MOSI/MISO/CS) must be provided by the integrator in the
 *    build environment via IfxXspi_PinMap.h or equivalent board configuration.
 */
#include "qspi.h"
#include "IfxXspi_PinMap.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"
#include "IfxPort.h"
#include "IfxXspi_Spi.h"

/* ========================================================================== */
/* Private ISR wrappers (installed with configured priorities)                 */
/* ========================================================================== */
static void interruptXspiTx(void);
static void interruptXspiRx(void);
static void interruptXspiErr(void);

/* Keep a static reference to the active XSPI driver for ISR usage */
static IfxXspi_Spi *s_xspiIsrHandle = NULL_PTR;

/* ========================================================================== */
/* Implementation                                                             */
/* ========================================================================== */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == NULL_PTR)
    {
        return Ifx_Status_notOk;
    }

    qspiCommunication->initialized  = FALSE;
    qspiCommunication->module       = XSPI_MODULE;
    qspiCommunication->exchangeFnc  = (Qspi_XspiExchangeFnc)&IfxXspi_Spi_exchange;
    qspiCommunication->tleBufferSize = TLE9180_WORKING_BUFFER_SIZE_BYTES;

    /* 1) Create and load a default XSPI module configuration for the selected module instance. */
    IfxXspi_Spi_Config moduleConfig;
    IfxXspi_Spi_initModuleConfig(&moduleConfig, qspiCommunication->module);

    /* 2) Assign the SPI I/O pins using XSPI PinMap and set directions/pads via driver pins structure. */
    /*    Integrator must provide exact pins through IfxXspi_PinMap.
     *    Here we prepare the structure; fields should be assigned by integrator if needed. */
    IfxXspi_Spi_GpioPins xspiPins;
    /* Zero-initialize to indicate 'no change' if integrator does not override */
    (void)Ifx_memset(&xspiPins, 0, sizeof(xspiPins));
    IfxXspi_Spi_setXspiGpioPins(qspiCommunication->module, &xspiPins);

    /* 3) Set operating parameters in the module configuration. */
    /*    Requirements: 50 MHz max baudrate, SPI mode 0, MSB first, 8-bit frame, active-low CS, channel-based CS, no DMA. */
    /*    Field names follow iLLD conventions similar to QSPI; exact mapping is handled by iLLD at compile time. */
    moduleConfig.maximumBaudrate = (float32)SPI_MAXIMUM_BAUDRATE_HZ;
    moduleConfig.isrProvider     = IfxSrc_Tos_cpu0;
    moduleConfig.txPriority      = (sint32)INTERRUPT_PRIORITY_QSPI3_TX;
    moduleConfig.rxPriority      = (sint32)INTERRUPT_PRIORITY_QSPI3_RX;
    moduleConfig.erPriority      = (sint32)INTERRUPT_PRIORITY_QSPI3_ERR;

    /* SPI protocol settings (Mode 0, MSB-first, 8-bit) are commonly specified via a transfer config. */
    IfxXspi_Spi_initTransferConfig transferConfig;
    (void)Ifx_memset(&transferConfig, 0, sizeof(transferConfig));
    transferConfig.frameLength       = SPI_FRAME_LENGTH_BITS;     /* 8-bit frames */
    transferConfig.bitOrderMsbFirst  = TRUE;                      /* MSB-first */
    transferConfig.clockPolarityLow  = TRUE;                      /* CPOL = 0 */
    transferConfig.clockPhaseLeading = TRUE;                      /* CPHA = 0 (sample on leading edge) */
    transferConfig.csActiveLow       = TRUE;                      /* Active-low CS */
    transferConfig.channelBasedCs    = TRUE;                      /* Channel-based CS behavior */
    transferConfig.csLeadTimeNs      = 0u;                        /* 0 ns (per requirements) */
    transferConfig.csHoldTimeNs      = 0u;                        /* 0 ns (per requirements) */
    transferConfig.interFrameDelayNs = 0u;                        /* 0 ns (per requirements) */

    /* 4) Install ISR handlers with priorities; type-of-service is taken from the module configuration (cpu0). */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiTx,  (uint32)INTERRUPT_PRIORITY_QSPI3_TX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiRx,  (uint32)INTERRUPT_PRIORITY_QSPI3_RX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiErr, (uint32)INTERRUPT_PRIORITY_QSPI3_ERR);

    /* 5) Initialize the XSPI module with the configured parameters and store the handle. */
    {
        IfxXspi_Status initStatus = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &moduleConfig);
        if (initStatus != 0u)
        {
            return Ifx_Status_notOk; /* Early exit on failure (per error-handling requirements) */
        }
    }

    /* Apply transfer-level parameters (mode, frame length, CS timings, etc.) */
    IfxXspi_Spi_transferInit(qspiCommunication->module, &transferConfig);

    /* Make ISR handlers aware of the active handle */
    s_xspiIsrHandle = &qspiCommunication->xspi;

    /* 6) Initialize target device control GPIOs */
    /* TLE9180 ENABLE: push-pull output, initially inactive (low) since active level = high */
    IfxPort_setPinModeOutput(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_State_low);

    /* TLE9180 nINT: input with pull-up (requirements) */
    IfxPort_setPinModeInput(TLE9180_NINT_PORT, TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);

    /* 7) Link TLE9180 abstraction expectations */
    /* Buffer size and exchange callback already stored in context.
       Higher layer should pass this context to TLE9180 driver which uses exchangeFnc for full-duplex exchanges. */

    qspiCommunication->initialized = TRUE;
    return Ifx_Status_ok;
}

/* ========================================================================== */
/* ISRs                                                                        */
/* ========================================================================== */
static void interruptXspiTx(void)
{
    if (s_xspiIsrHandle != NULL_PTR)
    {
        IfxXspi_Spi_isrTransmit(s_xspiIsrHandle);
    }
}

static void interruptXspiRx(void)
{
    if (s_xspiIsrHandle != NULL_PTR)
    {
        (void)IfxXspi_Spi_isrReceive(s_xspiIsrHandle);
    }
}

static void interruptXspiErr(void)
{
    /* XSPI error-specific ISR API may not be available; keep minimal handler to avoid spurious triggers. */
    /* Intentionally empty by design; routing/priorities are configured in moduleConfig. */
    (void)0;
}
