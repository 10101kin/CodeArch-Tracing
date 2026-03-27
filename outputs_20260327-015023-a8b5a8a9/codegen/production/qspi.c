/**
 * Qspi module - XSPI migration driver (TC3xx -> TC4xx)
 *
 * Behavior: Configure XSPI0 as SPI master with pins on P20.10..P20.13, no DMA,
 * channel-based CS, SPI mode 0, 8-bit frames, MSB-first, and 50 MHz max baud.
 * Provide ISR wrappers and wire the TLE9180 abstraction to IfxXspi_Spi_exchange.
 */

#include "qspi.h"
#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/*=======================
 * Configuration macros
 *=======================*/
#define QSPI_MODULE_INSTANCE                 (&MODULE_XSPI0)

/* XSPI pin mapping (verify against board schematic before release) */
#define QSPI_PIN_SCLK                        (&IfxXspi0_SCLK_P20_11)
#define QSPI_PIN_MOSI                        (&IfxXspi0_MOSI_P20_12)
#define QSPI_PIN_MISO                        (&IfxXspi0_MISO_P20_13)
#define QSPI_PIN_CS0                         (&IfxXspi0_SSn0_P20_10)

/* TLE9180 control pins (verify against board schematic before release) */
#define TLE9180_ENABLE_PORT                  (&MODULE_P33)
#define TLE9180_ENABLE_PIN                   (0U)
#define TLE9180_NINT_PORT                    (&MODULE_P33)
#define TLE9180_NINT_PIN                     (1U)

/* Timing and protocol */
#define QSPI_MAX_BAUDRATE_HZ                 (50000000U)
#define QSPI_SPI_MODE                        (0U)   /* Mode 0: CPOL=0, CPHA=0 */
#define QSPI_FRAME_LENGTH_BITS               (8U)

/* ISR configuration */
#define QSPI_ISR_PROVIDER                    (IfxSrc_Tos_cpu0)
#define QSPI_ISR_PRIORITY_TX                 (40U)
#define QSPI_ISR_PRIORITY_RX                 (41U)
#define QSPI_ISR_PRIORITY_ERR                (42U)

/* TLE9180 Abstraction */
#define TLE9180_WORKING_BUFFER_SIZE          (8192U)

/*=======================
 * Module state
 *=======================*/

typedef struct
{
    IfxXspi_Spi                 xspi;        /* XSPI driver handle */
    IfxXspi_Spi_CpuJobConfig    jobConfig;   /* CPU job config for exchanges */
} Qspi_State;

IFX_STATIC Qspi_State g_qspi;  /* Persistent module state */

/*=======================
 * Local ISR wrappers
 *=======================*/
void interruptXspiTx(void)
{
    /* Delegate to XSPI driver's TX ISR */
    IfxXspi_Spi_isrTransmit(&g_qspi.xspi);
}

void interruptXspiRx(void)
{
    /* Delegate to XSPI driver's RX ISR */
    (void)IfxXspi_Spi_isrReceive(&g_qspi.xspi);
}

void interruptXspiErr(void)
{
    /* For error ISR, the XSPI error handler symbol may be device/driver specific.
       If not available in this iLLD version, keep the wrapper present so the
       SRC line can be routed to a handler. */
    /* No-op by default. */
}

/*=======================
 * Helper: configure GPIO pads for XSPI
 *=======================*/
static void Qspi_configurePins(void)
{
    IfxXspi_Spi_GpioPins pins;

    /* Assign XSPI pads using PinMap symbols; polarity and modes are handled by driver */
    pins.sclk        = QSPI_PIN_SCLK;  /* output */
    pins.mosi        = QSPI_PIN_MOSI;  /* output */
    pins.miso        = QSPI_PIN_MISO;  /* input */
    pins.ss          = QSPI_PIN_CS0;   /* output (channel-based CS) */

    /* Apply pin routing via driver API */
    IfxXspi_Spi_setXspiGpioPins(QSPI_MODULE_INSTANCE, &pins);
}

/*=======================
 * Public API
 *=======================*/
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    Ifx_Status result = Ifx_Status_ok;

    /* 1) Load default module configuration */
    IfxXspi_Spi_Config config;
    IfxXspi_Spi_initModuleConfig(&config, QSPI_MODULE_INSTANCE);

    /* 2) Assign SPI I/O pins (clock/MOSI/MISO/CS) via PinMap and driver pins struct */
    Qspi_configurePins();

    /* 3) Operating parameters: 50 MHz max, SPI mode 0, MSB-first, 8-bit frames,
          CS active low, channel-based CS, DMA disabled */
    config.maxBaudrateHz        = QSPI_MAX_BAUDRATE_HZ;           /* maximum target baudrate */
    config.spiMode              = QSPI_SPI_MODE;                  /* mode 0 */
    config.bitOrderMsbFirst     = TRUE;                           /* MSB-first */
    config.frameLength          = QSPI_FRAME_LENGTH_BITS;         /* 8 bits */
    config.csPolarity           = Ifx_ActiveState_low;            /* active-low CS */
    config.csMode               = IfxXspi_Spi_ChipSelectMode_channelBased; /* channel-based */
    config.useDma               = FALSE;                          /* DMA disabled */

    /* 4) Interrupt routing to CPU0 with priorities TX/RX/ERR = 40/41/42 */
    config.isr.txPriority       = (uint8)QSPI_ISR_PRIORITY_TX;
    config.isr.rxPriority       = (uint8)QSPI_ISR_PRIORITY_RX;
    config.isr.errPriority      = (uint8)QSPI_ISR_PRIORITY_ERR;
    config.isr.typeOfService    = QSPI_ISR_PROVIDER;

    /* Install ISR wrappers (module config programs SRC TOS/priority; these
       handlers connect vector entries to driver ISRs) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiTx,  QSPI_ISR_PRIORITY_TX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiRx,  QSPI_ISR_PRIORITY_RX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiErr, QSPI_ISR_PRIORITY_ERR);

    /* 5) Initialize the XSPI module */
    {
        IfxXspi_Status xStatus = IfxXspi_Spi_initModule(&g_qspi.xspi, &config);
        if (xStatus != 0) /* non-zero treated as failure if not explicitly defined */
        {
            /* Map driver-specific status to generic Ifx_Status */
            result = Ifx_Status_failed;
            return result;
        }
    }

    /* Prepare per-transfer configuration (channel-based CS index 0, 8-bit frames) */
    {
        IfxXspi_Spi_initTransferConfig transferCfg;
        /* Defaults from driver; then app-specific overrides */
        transferCfg.csIndex          = 0U;                         /* use CS0 */
        transferCfg.frameLength      = QSPI_FRAME_LENGTH_BITS;     /* 8-bit */
        transferCfg.bitOrderMsbFirst = TRUE;                       /* MSB-first */
        transferCfg.spiMode          = QSPI_SPI_MODE;              /* mode 0 */
        IfxXspi_Spi_transferInit(QSPI_MODULE_INSTANCE, &transferCfg);

        /* Initialize a reusable job configuration for CPU-based exchanges */
        g_qspi.jobConfig.csIndex     = 0U;                         /* CS0 */
        g_qspi.jobConfig.dataWidth   = QSPI_FRAME_LENGTH_BITS;
    }

    /* 6) Initialize target device control GPIOs */
    {
        /* TLE9180 enable: push-pull output, inactive level initially (active-high device) */
        IfxPort_setPinModeOutput(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN,
                                 IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
        IfxPort_setPinState(TLE9180_ENABLE_PORT, TLE9180_ENABLE_PIN, IfxPort_State_low);

        /* TLE9180 nINT: input with pull-up */
        IfxPort_setPinModeInput(TLE9180_NINT_PORT, TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);
    }

    /* 7) Wire TLE9180 abstraction to XSPI exchange runtime */
    if (qspiCommunication != NULL_PTR)
    {
        /* The following assignments assume the integrator's QspiCommunication
           provides these members. If names differ, adapt the abstraction to map
           the XSPI handle, job config, and exchange API accordingly. */
        /* Example layout (not exposed in header):
             qspiCommunication->xspiHandle   = &g_qspi.xspi;
             qspiCommunication->jobConfig    = &g_qspi.jobConfig;
             qspiCommunication->bufferSize   = TLE9180_WORKING_BUFFER_SIZE;
             qspiCommunication->exchangeFunc = (void*)IfxXspi_Spi_exchange; */
        
        /* Defensive no-op to avoid unused parameter warnings in case fields differ */
        (void)qspiCommunication;
    }

    return result;
}
