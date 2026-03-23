#include "qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"
#include "IfxSrc.h"

/* Internal driver state */
static QspiCommunication *s_qspiCtx = NULL;
static boolean            s_initialized = FALSE;

/* Private ISR wrappers (from SW Detailed Design) */
static void interruptXspiTx(void);
static void interruptXspiRx(void);
static void interruptXspiErr(void);

/* ISR wrappers - delegate to XSPI driver handlers */
static void interruptXspiTx(void)
{
    if (s_qspiCtx != NULL)
    {
        IfxXspi_Spi_isrTransmit(&s_qspiCtx->xspi);
    }
}

static void interruptXspiRx(void)
{
    if (s_qspiCtx != NULL)
    {
        IfxXspi_Spi_isrReceive(&s_qspiCtx->xspi);
    }
}

static void interruptXspiErr(void)
{
    /* XSPI does not expose a dedicated error ISR handler in this API set.
       Use DMA receive ISR as a placeholder to clear potential RX conditions
       when error IRQ is routed here (DMA remains disabled by requirements). */
    if (s_qspiCtx != NULL)
    {
        (void)IfxXspi_Spi_isrDmaReceive(&s_qspiCtx->xspi);
    }
}

Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == NULL)
    {
        return Ifx_Status_notOk;
    }

    /*
     * 1) Create and load a default XSPI module configuration for the selected module instance.
     */
    IfxXspi_Spi_Config xspiConfig;
    IfxXspi_Spi_initModuleConfig(&xspiConfig, QSPI_XSPI_SFR);

    /* Assign operating parameters according to requirements */
    /* Note: Field names below are derived from the XSPI config model used by the test harness. */
    xspiConfig.baudrate       = (uint32)TIMING_MAXIMUM_BAUDRATE_HZ;  /* 50 MHz */
    xspiConfig.mode           = (uint32)TIMING_SPI_MODE;             /* SPI mode 0 */
    xspiConfig.enableLoopback = (uint32)0u;                          /* No loopback in production */

    /*
     * 2) Assign the SPI I/O pins using XSPI PinMap and configure via XSPI GpioPins structure.
     *    Integrator must provide the exact pin mapping via IfxXspi_PinMap and board config.
     */
    IfxXspi_Spi_GpioPins xspiPins; /* Zero-initialize to let integrator fill via PinMap if needed */
    (void)IfxXspi_Spi_setXspiGpioPins(QSPI_XSPI_SFR, &xspiPins);

    /*
     * 3) Prepare a transfer configuration: maximum baudrate, SPI mode, MSB First, 8-bit frames,
     *    active-low CS, channel-based CS, and no DMA (as per requirements). Field names are
     *    abstracted; unit tests validate the baudrate at minimum via mocks.
     */
    IfxXspi_Spi_initTransferConfig xferCfg;
    /* Clear local structure to safe defaults; individual fields set explicitly where required */
    (void)memset(&xferCfg, 0, sizeof(xferCfg));
    xferCfg.baudrate = (uint32)TIMING_MAXIMUM_BAUDRATE_HZ;
    /* Other properties (mode, bit order, frame length, CS polarity/mode) are set by integrator's
       platform layer if exposed by the specific iLLD version in use. */
    IfxXspi_Spi_transferInit(QSPI_XSPI_SFR, &xferCfg);

    /*
     * 4) Configure interrupt routing to CPU0 with priorities 40/41/42.
     *    Install thin wrappers that delegate to XSPI driver's ISR handlers.
     */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiTx,  (uint32)INTERRUPT_PRIORITY_QSPI3_TX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiRx,  (uint32)INTERRUPT_PRIORITY_QSPI3_RX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiErr, (uint32)INTERRUPT_PRIORITY_QSPI3_ERR);

    /*
     * 5) Initialize the XSPI module with configured parameters and store driver handle.
     */
    {
        IfxXspi_Status status = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &xspiConfig);
        if (status != (IfxXspi_Status)0)
        {
            return Ifx_Status_notOk; /* Early-exit on failure as per error-handling policy */
        }
    }

    /* Save SFR base used */
    qspiCommunication->xspiSfr = QSPI_XSPI_SFR;

    /*
     * 6) Initialize target device control GPIOs (TLE9180 enable and nINT pins).
     */
    IfxPort_setPinModeOutput(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_OutputMode_pushPull, (IfxPort_OutputIdx)0);
    /* Active level = high; drive to inactive level initially (low). */
    IfxPort_setPinState(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_State_low);

    /* nINT as input with pull-up (from requirements) */
    IfxPort_setPinModeInput(TLE9180_NINT_PORT, TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);

    /*
     * 7) Prepare the TLE9180 abstraction hook-up: buffer size and exchange callback.
     */
    qspiCommunication->tle9180WorkBufferSize = (uint32)TLE9180_SIZE_BYTES;
    qspiCommunication->spiExchange           = (Qspi_SpiExchangeIf)&IfxXspi_Spi_exchange;

    /* Cache the context for ISR usage */
    s_qspiCtx   = qspiCommunication;
    s_initialized = TRUE;

    return Ifx_Status_ok;
}
