#include "unity.h"
#include "qspi.h"

#include "IfxXspi_Spi.h"   // XSPI (TC4xx) mock controls
#include "IfxPort.h"        // GPIO mock controls
#include "IfxCpu_Irq.h"     // IRQ mock controls
#include "Ifx_Types.h"

// Structured configuration values (from requirements)
// SPI settings
#define TIMING_MAXIMUM_BAUDRATE_HZ   (50000000u)    // 50 MHz
#define SPI_ISR_PRIO_TX              (40u)
#define SPI_ISR_PRIO_RX              (41u)
#define SPI_ISR_PRIO_ERR             (42u)

// TLE9180 control pins derived from config:
//  - ENABLE: P33.0  => port index 33, pin index 0
//  - nINT:   P33.1  => pin index 1 on port 33
#define TLE9180_ENABLE_PORT_INDEX    (33u)
#define TLE9180_ENABLE_PIN_INDEX     (0u)
#define TLE9180_NINT_PIN_INDEX       (1u)

// Port state expectations derived from ACTIVE level = high => initial inactive = low(0)
#define PORT_STATE_LOW               (0u)

static void arrange_default_mock_returns(void)
{
    // Production init returns Ifx_Status from XSPI init path
    IfxXspi_Spi_Mock_SetReturn_initModule(Ifx_Status_ok);
    // In case production performs a sanity exchange during init/linking
    IfxXspi_Spi_Mock_SetReturn_exchange(Ifx_Status_ok);
}

void setUp(void)
{
    IfxXspi_Spi_Mock_Reset();
    IfxPort_Mock_Reset();
    IfxCpu_Irq_Mock_Reset();
}

void tearDown(void) {}

void test_Qspi_initQspi_CallsExpectedDriverAPIs(void)
{
    // Arrange
    arrange_default_mock_returns();
    QspiCommunication comm = {0};

    // Act
    Ifx_Status status = Qspi_initQspi(&comm);

    // Assert: init result OK
    TEST_ASSERT_EQUAL(Ifx_Status_ok, status);

    // Verify TC4xx XSPI driver calls (target family APIs)
    TEST_ASSERT_TRUE(IfxXspi_Spi_Mock_GetCallCount_initModuleConfig() > 0);
    TEST_ASSERT_TRUE(IfxXspi_Spi_Mock_GetCallCount_setXspiGpioPins() > 0);
    TEST_ASSERT_TRUE(IfxXspi_Spi_Mock_GetCallCount_transferInit() > 0);
    TEST_ASSERT_TRUE(IfxXspi_Spi_Mock_GetCallCount_initModule() > 0);

    // Verify GPIO configuration for control signals
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeOutput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinModeInput() > 0);
    TEST_ASSERT_TRUE(IfxPort_Mock_GetCallCount_setPinState() > 0);

    // Expected per design: link XSPI exchange (ensure symbol is used)
    TEST_ASSERT_TRUE(IfxXspi_Spi_Mock_GetCallCount_exchange() > 0);
}

void test_Qspi_initQspi_SetsExpectedConfigValues(void)
{
    // Arrange
    arrange_default_mock_returns();
    QspiCommunication comm = {0};

    // Act
    (void)Qspi_initQspi(&comm);

    // A) Control GPIOs
    // Enable pin: P33.0 -> port index 33, pin index 0, initial inactive level = low
    TEST_ASSERT_EQUAL_UINT32(TLE9180_ENABLE_PORT_INDEX, IfxPort_Mock_GetLastArg_setPinModeOutput_index());
    TEST_ASSERT_EQUAL_UINT32(TLE9180_ENABLE_PIN_INDEX,  IfxPort_Mock_GetLastArg_setPinModeOutput_pinIndex());

    TEST_ASSERT_EQUAL_UINT32(TLE9180_ENABLE_PIN_INDEX,  IfxPort_Mock_GetLastArg_setPinState_pinIndex());
    TEST_ASSERT_EQUAL_UINT32(PORT_STATE_LOW,            IfxPort_Mock_GetLastArg_setPinState_action());

    // nINT pin: P33.1 -> input on pin index 1
    TEST_ASSERT_EQUAL_UINT32(TLE9180_NINT_PIN_INDEX,    IfxPort_Mock_GetLastArg_setPinModeInput_pinIndex());

    // B) DMA disabled: no DMA receive ISR should be used during init/linking
    TEST_ASSERT_EQUAL_UINT32(0u, IfxXspi_Spi_Mock_GetCallCount_isrDmaReceive());
}

void test_Qspi_initQspi_ConfiguresInterrupts_TxRxErr_Priorities(void)
{
    // Arrange
    arrange_default_mock_returns();
    QspiCommunication comm = {0};

    // Act
    (void)Qspi_initQspi(&comm);

    // Assert: three ISR handlers installed (TX/RX/ERR)
    TEST_ASSERT_EQUAL_UINT32(3u, IfxCpu_Irq_Mock_GetCallCount_installInterruptHandler());

    // Verify the last installed priority matches ERR = 42 (TX=40, RX=41, ERR=42)
    TEST_ASSERT_EQUAL_UINT32(SPI_ISR_PRIO_ERR, IfxCpu_Irq_Mock_GetLastArg_installInterruptHandler_serviceReqPrioNumber());
}
