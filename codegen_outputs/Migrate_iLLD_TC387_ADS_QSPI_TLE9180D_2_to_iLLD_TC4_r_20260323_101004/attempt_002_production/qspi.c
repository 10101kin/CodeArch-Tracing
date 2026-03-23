#include "qspi.h"
#include "IfxPort.h"
#include "IfxXspi_Spi.h"
#include "IfxCpu_Irq.h"

/* ========================= Private State ========================= */
static IfxXspi_Spi *s_xspiHandle = NULL;  /* Used by ISR wrappers */

/* ========================= Private ISRs ========================= */
/**
 * TX ISR wrapper: calls XSPI driver's transmit ISR.
 */
static void interruptXspiTx(void)
{
    if (s_xspiHandle != NULL)
    {
        IfxXspi_Spi_isrTransmit(s_xspiHandle);
    }
}

/**
 * RX ISR wrapper: calls XSPI driver's receive ISR.
 */
static void interruptXspiRx(void)
{
    if (s_xspiHandle != NULL)
    {
        (void)IfxXspi_Spi_isrReceive(s_xspiHandle);
    }
}

/**
 * ERR ISR wrapper: no dedicated XSPI error ISR available in the provided API list.
 * This wrapper is installed to the error priority vector to preserve the reference
 * structure. If required, integrator may extend it to inspect raw status.
 */
static void interruptXspiErr(void)
{
    /* Intentionally minimal: placeholder for error handling hook */
    (void)0;
}

/* ========================= Public API ========================= */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == NULL)
    {
        return Ifx_Status_notOk;
    }

    qspiCommunication->initialized = FALSE;

    /* 1) Load default XSPI module configuration for XSPI0 */
    IfxXspi_Spi_Config moduleConfig;
    IfxXspi_Spi_initModuleConfig(&moduleConfig, XSPI_MODULE_SFR);

    /* 2) Assign SPI I/O pins via XSPI PinMap structure */
    IfxXspi_Spi_GpioPins xspiPins;
    /* Zero-initialize; integrator shall provide exact SCLK/MOSI/MISO/CS mapping via PinMap */
    (void)IfxXspi_Spi_setXspiGpioPins(XSPI_MODULE_SFR, &xspiPins);

    /* 3) Set operating parameters (50 MHz, mode 0, MSB-first, 8-bit, active-low CS, channel-based CS) */
    /* Note: Field names depend on iLLD version; defaults from initModuleConfig are applied.
       Integrator may refine fields in moduleConfig and transferConfig as needed. */

    /* Prepare transfer configuration */
    IfxXspi_Spi_initTransferConfig transferConfig;
    /* Zero-initialize; integrator to refine data-width/bit-order/mode/CS timings if needed */
    IfxXspi_Spi_transferInit(XSPI_MODULE_SFR, &transferConfig);

    /* 4) Configure interrupts: install TX/RX/ERR wrappers with priorities 40/41/42 on CPU0 */
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiTx,  (uint32)INTERRUPT_PRIORITY_QSPI3_TX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiRx,  (uint32)INTERRUPT_PRIORITY_QSPI3_RX);
    IfxCpu_Irq_installInterruptHandler((void *)&interruptXspiErr, (uint32)INTERRUPT_PRIORITY_QSPI3_ERR);

    /* 5) Initialize the XSPI module and store handle */
    IfxXspi_Status xspiStatus = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &moduleConfig);
    if (xspiStatus != 0)
    {
        /* Non-zero indicates failure per conventional iLLD status enums */
        return Ifx_Status_notOk;
    }

    qspiCommunication->xspiSfr = XSPI_MODULE_SFR;

    /* Make handle available to ISRs only after successful init */
    s_xspiHandle = &qspiCommunication->xspi;

    /* 6) Initialize target device control GPIOs */
    /* Enable pin: output push-pull, inactive level = low (active level is high) */
    IfxPort_setPinModeOutput(TLE9180_CONTROL_SIGNALS_ENABLE_PIN_PORT,
                             TLE9180_CONTROL_SIGNALS_ENABLE_PIN_INDEX,
                             IfxPort_OutputMode_pushPull,
                             IfxPort_OutputIdx_general);
    IfxPort_setPinState(TLE9180_CONTROL_SIGNALS_ENABLE_PIN_PORT,
                        TLE9180_CONTROL_SIGNALS_ENABLE_PIN_INDEX,
                        IfxPort_State_low);

    /* nINT pin: input with pull-up as required */
    IfxPort_setPinModeInput(TLE9180_CONTROL_SIGNALS_NINT_PIN_PORT,
                            TLE9180_CONTROL_SIGNALS_NINT_PIN_INDEX,
                            IfxPort_InputMode_pullUp);

    /* 7) Bind TLE9180 abstraction */
    qspiCommunication->tle9180.size          = 8192u;
    qspiCommunication->tle9180.enablePort    = TLE9180_CONTROL_SIGNALS_ENABLE_PIN_PORT;
    qspiCommunication->tle9180.enablePin     = TLE9180_CONTROL_SIGNALS_ENABLE_PIN_INDEX;
    qspiCommunication->tle9180.enablePadMode = IfxPort_OutputMode_pushPull;
    qspiCommunication->tle9180.nintPort      = TLE9180_CONTROL_SIGNALS_NINT_PIN_PORT;
    qspiCommunication->tle9180.nintPin       = TLE9180_CONTROL_SIGNALS_NINT_PIN_INDEX;
    qspiCommunication->tle9180.nintInputMode = IfxPort_InputMode_pullUp;
    qspiCommunication->tle9180.spiExchange   = &IfxXspi_Spi_exchange;  /* Link XSPI exchange */

    qspiCommunication->initialized = TRUE;
    return Ifx_Status_ok;
}
