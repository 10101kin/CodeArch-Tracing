/*
 * qspi.c
 * XSPI driver migration of QSPI master for TLE9180 on AURIX TC4xx.
 *
 * Implements:
 *   - Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
 *   - ISR wrappers: interruptXspiTx, interruptXspiRx, interruptXspiErr
 *
 * Behavior per SW Detailed Design:
 *   1) Initialize XSPI module config for selected instance
 *   2) Assign SPI I/O pins via PinMap and set pin directions/characteristics
 *   3) Configure 50 MHz max baud, mode-0, MSB-first, 8-bit, active-low CS, channel-based CS, no DMA
 *   4) Configure interrupts (TX/RX/ERR: 40/41/42) on CPU0, provide ISR wrappers
 *   5) Initialize XSPI module and store handles in communication context
 *   6) Initialize TLE9180 control GPIOs (EN: output inactive, nINT: input pull-up)
 *   7) Prepare TLE9180 abstraction: buffer size 8192; exchange callback -> IfxXspi_Spi_exchange
 */

#include "qspi.h"
#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* Numeric configuration values (from requirements) */
#define XSPI_MAX_BAUDRATE_HZ           (50000000u)
#define XSPI_FRAME_LENGTH_BITS         (8u)
#define XSPI_ISR_PRIORITY_TX           (40)
#define XSPI_ISR_PRIORITY_RX           (41)
#define XSPI_ISR_PRIORITY_ERR          (42)
#define TLE9180_WORKING_BUFFER_SIZE    (8192u)

/* XSPI instance and pin mapping (provided by integrator/requirements) */
#define XSPI_MODULE                    (&MODULE_XSPI0)
#define XSPI_PIN_SCLK                  (&IfxXspi0_SCLK_P20_11)
#define XSPI_PIN_MOSI                  (&IfxXspi0_MOSI_P20_12)
#define XSPI_PIN_MISO                  (&IfxXspi0_MISO_P20_13)
#define XSPI_PIN_CS0                   (&IfxXspi0_SSn0_P20_10)

/* TLE9180 control pins (enable: active high, nINT: input pull-up) */
#define TLE9180_EN_PORT                (&MODULE_P33)
#define TLE9180_EN_PIN                 (0u)
#define TLE9180_NINT_PORT              (&MODULE_P33)
#define TLE9180_NINT_PIN               (1u)

/* ISR provider */
#define XSPI_ISR_PROVIDER              (IfxSrc_Tos_cpu0)

/* Internal communication context type (module-owned definition).
 * The header only exposes an opaque pointer, as required. */
typedef boolean (*QspiExchangeFnc)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);

struct QspiCommunication
{
    IfxXspi_Spi                         xspi;            /* XSPI driver handle */
    Ifx_XSPI                           *module;          /* XSPI base */
    IfxXspi_Spi_CpuJobConfig            job;             /* Default job config (channel-based CS) */
    QspiExchangeFnc                     exchange;        /* Exchange callback for TLE9180 */
    uint32                              tle9180BufSize;  /* Working buffer size for TLE9180 */
};

/* Module-local state: keep a pointer to the active context for ISR use */
IFX_STATIC struct QspiCommunication *g_qspiContext = NULL_PTR;

/* Private ISR wrappers (exact signatures required) */
void interruptXspiTx(void)
{
    if (g_qspiContext != NULL_PTR)
    {
        IfxXspi_Spi_isrTransmit(&g_qspiContext->xspi);
    }
}

void interruptXspiRx(void)
{
    if (g_qspiContext != NULL_PTR)
    {
        (void)IfxXspi_Spi_isrReceive(&g_qspiContext->xspi);
    }
}

void interruptXspiErr(void)
{
    if (g_qspiContext != NULL_PTR)
    {
        /* No dedicated error ISR API is available in the provided interface; use receive handler to service/clear */
        (void)IfxXspi_Spi_isrReceive(&g_qspiContext->xspi);
    }
}

/*
 * Initialize XSPI for TLE9180 communication (migration of QSPI master to XSPI on TC4xx).
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    Ifx_Status result = Ifx_Status_notOk;

    if (qspiCommunication == NULL_PTR)
    {
        return Ifx_Status_notOk;
    }

    /* 1) Create and load default XSPI module configuration for selected instance */
    IfxXspi_Spi_Config spiConfig;
    IfxXspi_Spi_initModuleConfig(&spiConfig, XSPI_MODULE);

    /* 2) Assign SPI I/O pins: SCLK/MOSI/MISO/CS via XSPI PinMap and GPIO pins structure */
    IfxXspi_Spi_GpioPins spiPins;
    spiPins.sclk = XSPI_PIN_SCLK;
    spiPins.mosi = XSPI_PIN_MOSI;
    spiPins.miso = XSPI_PIN_MISO;
    spiPins.cs   = XSPI_PIN_CS0;  /* channel-based CS: use channel selection */
    IfxXspi_Spi_setXspiGpioPins(XSPI_MODULE, &spiPins);

    /* 3) Operating parameters */
    /* - Set maximum baudrate to 50 MHz
       - SPI Mode 0 timing (CPOL=0, CPHA=0)
       - MSB-first, 8-bit frame length
       - CS polarity active-low; channel-based CS behavior; DMA disabled */
    spiConfig.maximumBaudrate = XSPI_MAX_BAUDRATE_HZ;
    spiConfig.spiMode.cpol = 0;  /* Mode 0: CPOL=0 */
    spiConfig.spiMode.cpha = 0;  /* Mode 0: CPHA=0 */
    spiConfig.bitOrder = 0;      /* 0: MSB-first (implementation-defined enum) */
    spiConfig.frameLength = XSPI_FRAME_LENGTH_BITS;
    spiConfig.cs.polarityActive = IfxPort_State_low; /* Active low */
    spiConfig.cs.mode = 0;       /* 0: channel-based CS (implementation-defined enum) */
    spiConfig.dmaEnabled = FALSE; /* No DMA */

    /* 4) Interrupt routing: install ISR handlers with priorities 40/41/42 on CPU0 */
    spiConfig.isr.tx.priority  = XSPI_ISR_PRIORITY_TX;
    spiConfig.isr.rx.priority  = XSPI_ISR_PRIORITY_RX;
    spiConfig.isr.err.priority = XSPI_ISR_PRIORITY_ERR;
    spiConfig.isr.provider     = XSPI_ISR_PROVIDER;

    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiTx,  XSPI_ISR_PRIORITY_TX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiRx,  XSPI_ISR_PRIORITY_RX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiErr, XSPI_ISR_PRIORITY_ERR);

    /* 5) Initialize the XSPI module and prepare transfer defaults */
    IfxXspi_Status initSts = IfxXspi_Spi_initModule(&qspiCommunication->xspi, &spiConfig);
    if (initSts != 0) /* compare to IfxXspi_Status_ok (implementation-defined: 0) */
    {
        return Ifx_Status_notOk;
    }

    IfxXspi_Spi_initTransferConfig xferCfg;
    /* Use channel-based CS and default timings; lead/hold/inter-frame delays are 0 ns per requirements */
    xferCfg.cs             = 0; /* Channel selection: 0 (SSn0) */
    xferCfg.csActiveLevel  = IfxPort_State_low;
    xferCfg.frameLength    = XSPI_FRAME_LENGTH_BITS;
    xferCfg.baudrate       = XSPI_MAX_BAUDRATE_HZ;
    xferCfg.bitOrder       = 0; /* MSB-first */
    xferCfg.spiModeCpol    = 0; /* Mode 0 */
    xferCfg.spiModeCpha    = 0; /* Mode 0 */
    xferCfg.csLeadTimeNs   = 0u;
    xferCfg.csHoldTimeNs   = 0u;
    xferCfg.interFrameNs   = 0u;
    IfxXspi_Spi_transferInit(XSPI_MODULE, &xferCfg);

    /* Store runtime context for ISR and exchanges */
    qspiCommunication->module        = XSPI_MODULE;
    qspiCommunication->exchange      = &IfxXspi_Spi_exchange;
    qspiCommunication->tle9180BufSize= TLE9180_WORKING_BUFFER_SIZE;
    /* Initialize default CPU job config (channel-based CS on SSn0) */
    qspiCommunication->job.cs       = 0;   /* CS0 */
    qspiCommunication->job.txBuffer = NULL_PTR;
    qspiCommunication->job.rxBuffer = NULL_PTR;
    qspiCommunication->job.length   = 0u;

    /* 6) Initialize TLE9180 control GPIOs */
    IfxPort_setPinModeOutput(TLE9180_EN_PORT,   (uint8)TLE9180_EN_PIN,   IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState    (TLE9180_EN_PORT,   (uint8)TLE9180_EN_PIN,   IfxPort_State_low); /* inactive (active-high enable) */
    IfxPort_setPinModeInput(TLE9180_NINT_PORT, (uint8)TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);

    /* 7) Link global context for ISR wrappers */
    g_qspiContext = qspiCommunication;

    result = Ifx_Status_ok;
    return result;
}
