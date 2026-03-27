/**
 * qspi.c
 *
 * Production driver: TC3xx QSPI -> TC4xx XSPI migration for TLE9180 communication
 *
 * Implements: Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
 * Behavior per SW Detailed Design and migration constraints.
 */

#include "qspi.h"
#include "Ifx_Types.h"
#include "IfxXspi_Spi.h"
#include "IfxXspi_PinMap.h"
#include "IfxPort.h"
#include "IfxCpu_Irq.h"

/* ======================================================================================
 * Numeric configuration constants (from CONFIGURATION VALUES FROM REQUIREMENTS)
 * ====================================================================================== */
#define QSPI_MAX_BAUDRATE_HZ                 (50000000u)
#define QSPI_SPI_MODE                        (0u)          /* Mode 0: CPOL=0, CPHA=0 */
#define QSPI_FRAME_LENGTH_BITS               (8u)
#define QSPI_DMA_ENABLED                     (0u)          /* false */
#define QSPI_TLE9180_WORKBUF_SIZE            (8192u)

#define QSPI_ISR_PROVIDER                    (IfxSrc_Tos_cpu0)
#define QSPI_ISR_PRIORITY_TX                 (40u)
#define QSPI_ISR_PRIORITY_RX                 (41u)
#define QSPI_ISR_PRIORITY_ERR                (42u)

/* ======================================================================================
 * Pin and module selection (from CONFIGURATION VALUES FROM REQUIREMENTS)
 * ====================================================================================== */
/* XSPI module instance */
#define QSPI_XSPI_MODULE                      (&MODULE_XSPI0)

/* XSPI pinmap symbols (Integrator must verify against target board schematic) */
#define QSPI_PIN_SCLK                         (&IfxXspi0_SCLK_P20_11)
#define QSPI_PIN_MOSI                         (&IfxXspi0_MOSI_P20_12)
#define QSPI_PIN_MISO                         (&IfxXspi0_MISO_P20_13)
#define QSPI_PIN_CS0                          (&IfxXspi0_SSn0_P20_10)

/* TLE9180 control GPIOs (Integrator must verify) */
#define QSPI_TLE9180_ENABLE_PORT              (&MODULE_P33)
#define QSPI_TLE9180_ENABLE_PIN               (0u)
#define QSPI_TLE9180_NINT_PORT                (&MODULE_P33)
#define QSPI_TLE9180_NINT_PIN                 (1u)

/* ======================================================================================
 * Internal types and module state
 * ====================================================================================== */

typedef struct
{
    IfxXspi_Spi                    xspi;        /* XSPI driver handle */
    Ifx_XSPI                      *module;      /* XSPI base */
    IfxXspi_Spi_GpioPins          pins;         /* Configured pin set */
    IfxXspi_Spi_CpuJobConfig      jobCfg;       /* Template job config (8-bit frames by default) */
} Qspi_State;

/*
 * Public communication context used by upper layers (TLE9180 abstraction)
 */
struct QspiCommunication
{
    IfxXspi_Spi                  *xspiHandle;            /* XSPI handle for transactions */
    IfxXspi_Spi_CpuJobConfig      defaultJob;            /* Default job configuration (caller may copy and adjust) */
    uint32                        tle9180WorkBufferSize;  /* size = 8192 as required */
    /* Exchange callback wired to XSPI runtime function */
    boolean (*exchange)(IfxXspi_Spi *xspi, IfxXspi_Spi_CpuJobConfig *jobConfig);
};

/* Use IFX_STATIC for module-level state as per production code rules */
IFX_STATIC Qspi_State g_qspi;

/* ======================================================================================
 * ISR wrappers (installed with priorities 40/41/42) — bodies call XSPI driver ISRs
 * ====================================================================================== */
void interruptXspiTx(void)
{
    /* TX service: forward to XSPI driver */
    IfxXspi_Spi_isrTransmit(&g_qspi.xspi);
}

void interruptXspiRx(void)
{
    /* RX service: forward to XSPI driver (non-DMA path) */
    (void)IfxXspi_Spi_isrReceive(&g_qspi.xspi);
}

void interruptXspiErr(void)
{
    /* Error service: XSPI error ISR not available in provided API list.
       Minimal safe handler provided; receiver ISR may clear certain flags depending on iLLD. */
    (void)IfxXspi_Spi_isrReceive(&g_qspi.xspi);
}

/* ======================================================================================
 * Local helpers (static inline-equivalents avoided; using direct sequence in init)
 * ====================================================================================== */

/* ======================================================================================
 * API IMPLEMENTATION
 * ====================================================================================== */

/**
 * Initialize the SPI master using the XSPI driver and wire it to the TLE9180 abstraction.
 * Algorithm (per SW detailed design):
 * 1) Create and load a default XSPI module configuration for the selected module instance.
 * 2) Assign the SPI I/O pins using the XSPI PinMap and configure via the driver pins structure.
 * 3) Set operating parameters: 50 MHz max baud, SPI mode 0, MSB-first, 8-bit frames,
 *    CS polarity active-low, channel-based CS, DMA disabled.
 * 4) Configure interrupts to CPU0 with TX/RX/ERR priorities 40/41/42 and provide ISR wrappers.
 * 5) Initialize the XSPI module and store handles in the provided communication context.
 * 6) Initialize target device control GPIOs (enable as output inactive; nINT as input pull-up).
 * 7) Prepare TLE9180 abstraction: set work buffer size to 8192 and link exchange callback.
 */
Ifx_Status Qspi_initQspi(QspiCommunication* const qspiCommunication)
{
    if (qspiCommunication == NULL_PTR)
    {
        return Ifx_Status_notOk;
    }

    /* Bind base module */
    g_qspi.module = QSPI_XSPI_MODULE;

    /* 1) Load default module configuration */
    IfxXspi_Spi_Config spiCfg;
    IfxXspi_Spi_initModuleConfig(&spiCfg, g_qspi.module);

    /* 2) Configure SPI I/O pins via driver structure */
    g_qspi.pins.sclk = QSPI_PIN_SCLK;
    g_qspi.pins.mosi = QSPI_PIN_MOSI;
    g_qspi.pins.miso = QSPI_PIN_MISO;
    g_qspi.pins.ss0  = QSPI_PIN_CS0;   /* channel-based CS: use CS0 by default */
    IfxXspi_Spi_setXspiGpioPins(g_qspi.module, &g_qspi.pins);

    /* 3) Operating parameters — module-level baud rate and basic timing/mode fields
       Note: Exact field names are per iLLD; values align with requirements. */
    spiCfg.maximumBaudrate = QSPI_MAX_BAUDRATE_HZ;           /* limit: 50 MHz */
    spiCfg.spiMode         = QSPI_SPI_MODE;                  /* Mode 0 */
    spiCfg.bitOrderMsbFirst = TRUE;                          /* MSB first */

    /* Interrupt routing from module config: provider and priorities */
    spiCfg.isrProvider = QSPI_ISR_PROVIDER;
    spiCfg.txPriority  = (uint8)QSPI_ISR_PRIORITY_TX;
    spiCfg.rxPriority  = (uint8)QSPI_ISR_PRIORITY_RX;
    spiCfg.erPriority  = (uint8)QSPI_ISR_PRIORITY_ERR;

    /* 5) Initialize the XSPI module */
    {
        IfxXspi_Status xspiStatus = IfxXspi_Spi_initModule(&g_qspi.xspi, &spiCfg);
        if (xspiStatus != 0)
        {
            return Ifx_Status_notOk;
        }
    }

    /* Prepare a default transfer configuration (frame size, CS behavior, polarity, DMA off) */
    {
        IfxXspi_Spi_initTransferConfig xferCfg;
        /* Initialize structure fields explicitly as per requirements */
        xferCfg.frameLengthBits     = QSPI_FRAME_LENGTH_BITS;    /* 8-bit frames */
        xferCfg.chipSelectActive    = Ifx_ActiveState_low;       /* CS active-low */
        xferCfg.channelBasedCs      = TRUE;                      /* channel-based CS */
        xferCfg.dmaEnabled          = (boolean)QSPI_DMA_ENABLED; /* DMA disabled */
        xferCfg.bitOrderMsbFirst    = TRUE;                      /* MSB first */
        xferCfg.spiMode             = QSPI_SPI_MODE;             /* Mode 0 */
        xferCfg.csPin               = QSPI_PIN_CS0;              /* default CS pin mapping */

        /* Initialize hardware transfer parameters on the selected module */
        IfxXspi_Spi_transferInit(g_qspi.module, &xferCfg);

        /* Seed a template job configuration in the driver state for re-use by callers */
        g_qspi.jobCfg.frameLen = QSPI_FRAME_LENGTH_BITS;
        g_qspi.jobCfg.cs       = 0u;    /* CS channel index 0 by default */
        g_qspi.jobCfg.tx       = NULL_PTR; /* caller provides buffers */
        g_qspi.jobCfg.rx       = NULL_PTR;
        g_qspi.jobCfg.length   = 0u;       /* caller sets length */
    }

    /* 4) Install ISR wrappers with specified priorities (TX/RX/ERR: 40/41/42) */
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiTx,  (uint32)QSPI_ISR_PRIORITY_TX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiRx,  (uint32)QSPI_ISR_PRIORITY_RX);
    IfxCpu_Irq_installInterruptHandler((void*)interruptXspiErr, (uint32)QSPI_ISR_PRIORITY_ERR);

    /* 6) Target device control GPIOs: enable = output (inactive low), nINT = input pull-up */
    IfxPort_setPinModeOutput(QSPI_TLE9180_ENABLE_PORT, QSPI_TLE9180_ENABLE_PIN,
                             IfxPort_OutputMode_pushPull, IfxPort_OutputIdx_general);
    IfxPort_setPinState(QSPI_TLE9180_ENABLE_PORT, QSPI_TLE9180_ENABLE_PIN, IfxPort_State_low);

    IfxPort_setPinModeInput(QSPI_TLE9180_NINT_PORT, QSPI_TLE9180_NINT_PIN, IfxPort_InputMode_pullUp);

    /* 7) Bind XSPI exchange function and working buffer size to TLE9180 abstraction context */
    qspiCommunication->xspiHandle            = &g_qspi.xspi;
    qspiCommunication->defaultJob            = g_qspi.jobCfg;
    qspiCommunication->tle9180WorkBufferSize = (uint32)QSPI_TLE9180_WORKBUF_SIZE;
    qspiCommunication->exchange              = IfxXspi_Spi_exchange;

    return Ifx_Status_ok;
}
