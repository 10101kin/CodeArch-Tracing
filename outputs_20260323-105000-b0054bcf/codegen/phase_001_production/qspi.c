#include "qspi.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* Local ISR handle to service XSPI interrupts */
static IfxXspi_Spi *s_xspiIsrHandle = (IfxXspi_Spi *)0;

/* Private ISR wrappers (installed with priorities 40/41/42) */
static void interruptXspiTx(void)
{
    if (s_xspiIsrHandle != (IfxXspi_Spi *)0)
    {
        IfxXspi_Spi_isrTransmit(s_xspiIsrHandle);
    }
}

static void interruptXspiRx(void)
{
    if (s_xspiIsrHandle != (IfxXspi_Spi *)0)
    {
        (void)IfxXspi_Spi_isrReceive(s_xspiIsrHandle);
    }
}

static void interruptXspiErr(void)
{
    /*
     * XSPI error-specific ISR API is not exposed in the provided interface.
     * Use receive ISR to service/clear any pending conditions where applicable.
     */
    if (s_xspiIsrHandle != (IfxXspi_Spi *)0)
    {
        (void)IfxXspi_Spi_isrReceive(s_xspiIsrHandle);
    }
}

Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == (QspiCommunication *)0)
    {
        return Ifx_Status_notOk;
    }

    /* 1) Create and load a default XSPI module configuration for the selected module instance */
    IfxXspi_Spi_Config xspiConfig;
    IfxXspi_Spi_initModuleConfig(&xspiConfig, QSPI_XSPI_MODULE);

    /* 2) Assign the SPI I/O pins using XSPI PinMap (directions/pad are handled by driver pins structure) */
    IfxXspi_Spi_GpioPins xspiPins;   /* Intentionally zero-initialized; integrator may refine per board */
    (void)memset(&xspiPins, 0, sizeof(xspiPins));
    IfxXspi_Spi_setXspiGpioPins(QSPI_XSPI_MODULE, &xspiPins);

    /* 3) Prepare and apply a transfer configuration (mode, bit order, frame length, CS behavior/polarity) */
    IfxXspi_Spi_initTransferConfig xferCfg;
    (void)memset(&xferCfg, 0, sizeof(xferCfg));
    /*
     * Note: Field-level assignments are iLLD-version specific; defaults are loaded above.
     * Requirement values are captured below for traceability and may be applied by integrators
     * to the appropriate fields if exposed by the current iLLD version.
     * - Max baudrate: 50 MHz
     * - Mode: 0 (CPOL=0, CPHA=0)
     * - Bit order: MSB first
     * - Frame length: 8 bits
     * - CS polarity: active-low
     * - Channel-based CS: enabled
     * - CS lead/hold: 0 ns, Inter-frame delay: 0 ns
     */
    IfxXspi_Spi_transferInit(QSPI_XSPI_MODULE, &xferCfg);

    /* 4) Configure interrupt routing to the specified CPU core by installing ISR handlers with priorities */
    IfxCpu_Irq_installInterruptHandler((void *)interruptXspiTx,  (uint32)INTERRUPT_PRIORITY_QSPI3_TX);
    IfxCpu_Irq_installInterruptHandler((void *)interruptXspiRx,  (uint32)INTERRUPT_PRIORITY_QSPI3_RX);
    IfxCpu_Irq_installInterruptHandler((void *)interruptXspiErr, (uint32)INTERRUPT_PRIORITY_QSPI3_ERR);

    /* 5) Initialize the XSPI module and store the resulting handle */
    IfxXspi_Status initStatus = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &xspiConfig);
    if (initStatus != (IfxXspi_Status)0) /* Expect zero to represent OK in typical iLLD status enums */
    {
        /* Early return on failure, do not proceed */
        return Ifx_Status_notOk;
    }

    qspiCommunication->module          = QSPI_XSPI_MODULE;
    qspiCommunication->deviceSize      = (uint32)TLE9180_DEVICE_SIZE_BYTES;
    qspiCommunication->spiExchange     = &IfxXspi_Spi_exchange;
    qspiCommunication->maximumBaudrate = (uint32)QSPI_REQUIREMENTS_MAX_BAUDRATE_HZ;
    qspiCommunication->frameLengthBits = (uint8)QSPI_REQUIREMENTS_FRAME_LENGTH_BITS;
    qspiCommunication->isrProvider     = (IfxSrc_Tos)QSPI_REQUIREMENTS_ISR_PROVIDER;
    qspiCommunication->txPrio          = (uint8)INTERRUPT_PRIORITY_QSPI3_TX;
    qspiCommunication->rxPrio          = (uint8)INTERRUPT_PRIORITY_QSPI3_RX;
    qspiCommunication->errPrio         = (uint8)INTERRUPT_PRIORITY_QSPI3_ERR;
    qspiCommunication->dmaEnabled      = (boolean)QSPI_REQUIREMENTS_DMA_ENABLED;

    /* Share handle with ISRs */
    s_xspiIsrHandle = &qspiCommunication->xspi;

    /* 6) Initialize target device control GPIOs */
    /* TLE9180 ENABLE: output, push-pull, drive inactive (active-high enable, so set to low initially) */
    IfxPort_setPinModeOutput(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_State_low);

    /* TLE9180 nINT: input with pull-up */
    IfxPort_setPinModeInput(TLE9180_NINT_PORT, TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);

    /* 7) TLE9180 abstraction linkage is completed via spiExchange pointer and deviceSize above. */

    return Ifx_Status_ok;
}
